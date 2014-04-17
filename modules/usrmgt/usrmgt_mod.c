/*
 * $Id: usrmgt_mod.c 7041 2014-02-22 13:44:51Z dingjun$
 *
 * user management modules
 *
 * Copyright (C) 2014-2015 fangdingjun@i4season.com
 *
 * This file is part of opensips, a free SIP server.
 *
 * opensips is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version
 *
 * opensips is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * History:
 * -------
 *  2014-04-17: change the code structure
 *  2014-3-24: add device vendor and serail check
 *  2014-02-22: done the basic functon
 */

#include "usrmgt.h"

/* function define */
static int mod_init(void);
static void destroy(void);
static int child_init(int);
static int options_func(struct sip_msg *_msg, char *_foo, char *_bar);

static str db_url = { NULL, 0 };    /* user infomation db connect string */
static str db_url2 = { NULL, 0 };   /* product infomation db connect string */

int verify_devid = 0;           /* if verify devid when bind */

/* response code*/
static str opt_200_rpl = str_init("OK");
static str opt_500_rpl = str_init("Server internal error");
static str opt_400_rpl = str_init("Bad request");
char *hdr = "Content-Type: text/plain\r\n"; /* the header for reply body type */

/*
 * Exported functions
 */
static cmd_export_t cmds[] = {
    {"options_func", (cmd_function) options_func, 0, 0, 0, REQUEST_ROUTE},
    {0, 0, 0, 0, 0, 0}
};

/*
 * Exported parameters
 */

static param_export_t params[] = {
    //{"accept",     STR_PARAM, &acpt_c},
    //{"accept_encoding", STR_PARAM, &acpt_enc_c},
    //{"accept_language", STR_PARAM, &acpt_lan_c},
    //{"support",     STR_PARAM, &supt_c},
    {"db_url", STR_PARAM, &db_url.s},
    {"db_url2", STR_PARAM, &db_url2.s},
    {"verify_devid", INT_PARAM, &verify_devid},
    {0, 0, 0}
};

/*
 * Module description
 */
struct module_exports exports = {
    "usrmgt",                   /* Module name */
    MODULE_VERSION,
    DEFAULT_DLFLAGS,            /* dlopen flags */
    cmds,                       /* Exported functions */
    params,                     /* Exported parameters */
    0,                          /* exported statistics */
    0,                          /* exported MI functions */
    0,                          /* exported pseudo-variables */
    0,                          /* extra processes */
    mod_init,                   /* Initialization function */
    0,                          /* Response function */
    destroy,                    /* Destroy function */
    child_init                  /* Child init function */
};

struct method {
    char *name;                 /* method name */
    int (*func) (struct request_msg * req, str ** ret); /* function to call */
};

/* the method and the function */
struct method m_list[] = {
    {"bind", handle_paired},
    {"unbind", handle_unbind},
    {"getonlinelist", getonlinelist},
    {"getbinddev", getpaireddev},
    {"licese_put", license_put},
    {"license_get", license_get},
    {0, 0}
};

/* module destroy callback*/
static void destroy(void)
{
    if (db_handle) {
        /* close dabase connection */
        dbf.close(db_handle);
        db_handle = 0;
    }
    if (db_handle2) {
        dbf.close(db_handle2);
        db_handle2 = 0;
    }
}

/* child init callback*/
static int child_init(int rank)
{
    /*connect to database */
    db_handle = dbf.init(&db_url);
    if (db_handle == 0) {
        LM_ERR("unable to connect to the database\n");
        return -1;
    }

    db_handle2 = dbf.init(&db_url2);
    if (db_handle2 == 0) {
        LM_ERR("unalbe to connect to the db2\n");
        return -1;
    }
    return 0;
}

/*
 * initialize module
 */
static int mod_init(void)
{
    bind_usrloc_t bind_usrloc;

    LM_INFO("usrmgt module initializing...\n");
    /* set to default url if db_url not set */
    init_db_url(db_url, 0);
    init_db_url(db_url2, 0);

    LM_DBG("db_url: %s\n", db_url.s);
    LM_DBG("db_url2: %s\n", db_url2.s);

    /* load SIGNALING API */
    if (load_sig_api(&sigb) < 0) {
        LM_ERR("can't load signaling functions\n");
        return -1;
    }

    /* Find a database module */
    if (db_bind_mod(&db_url, &dbf) < 0) {
        LM_ERR("unable to bind to a database driver\n");
        return -1;
    }

    bind_usrloc = (bind_usrloc_t) find_export("ul_bind_usrloc", 1, 0);
    if (!bind_usrloc) {
        LM_ERR("find bind_usrloc failed\n");
        return -1;
    }
    if (bind_usrloc(&ul) < 0) {
        LM_ERR("bind usrloc failed\n");
        return -1;
    }

    return 0;
}

/* handle the OPTIONS method */
static int options_func(struct sip_msg *_msg, char *_table, char *_bar)
{
    str body;                   /* sip message body */
    struct request_msg parsed_msg;  /* save parsed request message */
    int ret = 0;                /* result */
    str *res = NULL;            /* result return from function */
    int done = 0;               /* the function is callled or not */
    int i;

    LM_DBG("begin to parse msg\n");

    /* get the message body */
    if (get_body(_msg, &body) < 0) {
        LM_ERR("get request body failed\n");
        sigb.reply(_msg, 500, &opt_500_rpl, NULL);
        ret = -1;
        goto error2;
    }

    /* no body specialed */
    if (!body.s) {
        LM_DBG("request body is empty\n");
        sigb.reply(_msg, 400, &opt_400_rpl, NULL);
        ret = -1;
        goto error2;
    }

    memset(&parsed_msg, 0, sizeof(parsed_msg));

    /* parse the request message */
    parse_req_msg(body.s, &parsed_msg);

    if (!parsed_msg.func.s) {
        LM_ERR("can not found function keyword\n");
        sigb.reply(_msg, 400, &opt_400_rpl, NULL);
        ret = -1;
        goto error1;
    }

    ret = -1;

    for (i = 0; m_list[i].name != NULL && m_list[i].func != NULL; i++) {
        if (strncmp(m_list[i].name, parsed_msg.func.s, parsed_msg.func.len)
            == 0) {

            /* call function */
            ret = m_list[i].func(&parsed_msg, &res);
            done = 1;
            break;
        }
    }

    if (!done) {
        sigb.reply(_msg, 400, &opt_400_rpl, NULL);
        goto error3;
    }

    if (ret == 0 && res != NULL && res->len > 0) {
        /* prepare for response */
        char *h_hdr = (char *) pkg_malloc(strlen(hdr) + 1);
        char *bdy = (char *) pkg_malloc(res->len + 1);
        strcpy(h_hdr, hdr);
        strncpy(bdy, res->s, res->len);
        bdy[res->len] = '\0';

        /* add header type */
        add_lump_rpl(_msg, h_hdr, strlen(h_hdr),
                     LUMP_RPL_HDR | LUMP_RPL_NODUP);

        /* add body */
        add_lump_rpl(_msg, bdy, strlen(bdy), LUMP_RPL_BODY);
    }
    if (ret == 0) {
        sigb.reply(_msg, 200, &opt_200_rpl, NULL);
    } else if (ret == -1) {
        sigb.reply(_msg, 400, &opt_400_rpl, NULL);
    } else {
        sigb.reply(_msg, 500, &opt_500_rpl, NULL);
    }

  error3:
    if (res) {
        if (res->s)
            pkg_free(res->s);
        pkg_free(res);
    }
  error1:
    free_req_msg(&parsed_msg);
  error2:
    return ret;
}
