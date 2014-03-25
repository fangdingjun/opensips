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
 *  2014-3-24: add device vendor and serail check
 * 2014-02-22: done the basic functon
 */

#include "usrmgt.h"

/* function define */
static int mod_init(void);
static void destroy(void);
static int child_init(int);
static int options_func(struct sip_msg* _msg, char* _foo, char* _bar);

static str db_url={NULL,0}; /* user infomation db connect string */
static str db_url2={NULL,0}; /* product infomation db connect string */

/* response code*/
static str opt_200_rpl = str_init("OK");
static str opt_500_rpl = str_init("Server internal error");
static str opt_400_rpl = str_init("Bad request");
char *hdr="Content-Type: text/plain\r\n"; /* the header for reply body type*/

/*
 * Exported functions
 */
static cmd_export_t cmds[] = {
	{"options_func", (cmd_function)options_func, 0, 0, 0, REQUEST_ROUTE},
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
    {"db_url",STR_PARAM,&db_url.s},
    {"db_url2",STR_PARAM,&db_url2.s},
	{0, 0, 0}
};

/*
 * Module description
 */
struct module_exports exports = {
	"usrmgt",       /* Module name */
	MODULE_VERSION,
	DEFAULT_DLFLAGS, /* dlopen flags */
	cmds,            /* Exported functions */
	params,          /* Exported parameters */
	0,               /* exported statistics */
	0,               /* exported MI functions */
	0,               /* exported pseudo-variables */
	0,               /* extra processes */
	mod_init,        /* Initialization function */
	0,               /* Response function */
	destroy,               /* Destroy function */
	child_init                /* Child init function */
};

/* module destroy callback*/
static void destroy(void)
{
	if (db_handle) {
        /* close dabase connection*/
		dbf.close(db_handle);
		db_handle = 0;
	}
    if(db_handle2){
        dbf.close(db_handle2);
        db_handle2=0;
    }
}

/* child init callback*/
static int child_init(int rank)
{
    /*connect to database*/
	db_handle = dbf.init(&db_url);
	if (db_handle == 0){
		LM_ERR("unable to connect to the database\n");
		return -1;
	}
    
    db_handle2= dbf.init(&db_url2);
    if(db_handle2 == 0){
        LM_ERR("unalbe to connect to the db2\n");
        return -1;
    }
	return 0;
}

/*
 * initialize module
 */
static int mod_init(void) {
    bind_usrloc_t bind_usrloc;

	LM_INFO("usrmgt module initializing...\n");
    /* set to default url if db_url not set */
    init_db_url(db_url,0);
    init_db_url(db_url2,0);
    
    LM_DBG("db_url: %s\n", db_url.s);
    LM_DBG("db_url2: %s\n", db_url2.s);
    
	/* load SIGNALING API */
	if(load_sig_api(&sigb)< 0) {
		LM_ERR("can't load signaling functions\n");
		return -1;
	}

	/* Find a database module */
	if (db_bind_mod(&db_url, &dbf) < 0){
		LM_ERR("unable to bind to a database driver\n");
		return -1;
	}

    bind_usrloc=(bind_usrloc_t)find_export("ul_bind_usrloc",1,0);
    if(!bind_usrloc){
        LM_ERR("find bind_usrloc failed\n");
        return -1;
    }
    if(bind_usrloc(&ul)<0){
        LM_ERR("bind usrloc failed\n");
        return -1;
    }


	return 0;
}

/* handle the OPTIONS method */
static int options_func(struct sip_msg* _msg, char* _table, char* _bar) {
    str body; /* sip message body */
    struct request_msg parsed_msg; /* save parsed request message */

    LM_DBG("begin to parse msg\n");

    /* get the message body */
    if(get_body(_msg,&body) < 0){
        LM_ERR("get request body failed\n");
        sigb.reply(_msg,500,&opt_500_rpl,NULL);
        return -1;
    }

    /* no body specialed */
    if(!body.s){
        LM_DBG("request body is empty\n");
        sigb.reply(_msg,400,&opt_400_rpl,NULL);
        return -1;
    }

    memset(&parsed_msg,0,sizeof(parsed_msg));

    /* parse the request message */
    if(parse_req_msg(body.s,&parsed_msg) < 0 ){
        LM_DBG("request message in wrong format\n");
        sigb.reply(_msg,400,&opt_400_rpl,NULL);
        return -1;
    }

    if(!parsed_msg.func.s){
        LM_ERR("can not found function keyword\n");
        sigb.reply(_msg,400,&opt_400_rpl,NULL);
        goto error1;
    }
    LM_DBG("msg function |%s|\n",parsed_msg.func.s);

    /* hande paired request */
    if(strncasecmp(parsed_msg.func.s,"bind",
                parsed_msg.func.len)==0){ /* bind method */
        int ret=handle_paired(&parsed_msg);
        if(ret == -1){
            LM_DBG("database operate failed\n");
            sigb.reply(_msg,500,&opt_500_rpl,NULL);
            goto error1;
        }else if (ret == -2){
            LM_DBG("paired failed\n");
            sigb.reply(_msg,400,&opt_400_rpl,NULL);
            goto error1;
        }
    }else if(strncasecmp(parsed_msg.func.s,"getbinddev",
                parsed_msg.func.len) == 0){ /* getbinddev method */
        str *rr=NULL;
        LM_DBG("call getbinddev\n");
        int ret=getpaireddev(&parsed_msg,&rr);
        LM_DBG("after call getbinddev\n");
        if(ret != 0){
            if(ret == -2 || ret == -3){
                sigb.reply(_msg,400,&opt_400_rpl,NULL);
            }else{
                sigb.reply(_msg,500,&opt_500_rpl,NULL);
            }
            goto error1;
        }
        if(!rr){
            LM_ERR("rr is NULL\n");
            sigb.reply(_msg,500,&opt_500_rpl,NULL);
            goto error1;
        }
        LM_DBG("rr.s %s rr.len %d\n",rr->s,rr->len);
        if(rr->len){
            str h_type;
            h_type.s=(char *)pkg_malloc(strlen(hdr)+1);
            strcpy(h_type.s,hdr);
            h_type.len=strlen(hdr);
            add_lump_rpl(_msg,h_type.s,h_type.len,
                    LUMP_RPL_HDR|LUMP_RPL_NODUP);
            add_lump_rpl(_msg,rr->s,rr->len,LUMP_RPL_BODY);
        }
        if(rr) pkg_free(rr);
    }else if(strncasecmp(parsed_msg.func.s,"getonlinelist",
                parsed_msg.func.len)==0){  /* get online usr list */
        str *s;
        getonlinelist(&s);
        if(s->len){
            str h_type;
            h_type.s=(char *)pkg_malloc(strlen(hdr)+1);
            strcpy(h_type.s,hdr);
            h_type.len=strlen(hdr);
            add_lump_rpl(_msg,h_type.s,h_type.len,
                    LUMP_RPL_HDR|LUMP_RPL_NODUP);
            add_lump_rpl(_msg,s->s,s->len,LUMP_RPL_BODY);
            //pkg_free(s);
        }
        if(s) pkg_free(s);
    }else{
        sigb.reply(_msg,400,&opt_400_rpl,NULL);
        goto error1;
    }
    sigb.reply(_msg,200,&opt_200_rpl,NULL);
    free_req_msg(&parsed_msg);
    return 0;

error1:
    free_req_msg(&parsed_msg);
    return -1;
}



