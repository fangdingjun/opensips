#include "usrmgt.h"
#include "../usrloc/ucontact.h"

/* get bind devlist by phone id
 * one phone may bind to more than one device
 * */
int getpaireddev(struct request_msg *r, str ** r1)
{
    char buf[128];
    str query_str;
    db_res_t *res=NULL;
    str body={0,0};

    char *all_contacts=NULL;
    int cblen = 4096;
    char *pu=NULL;

    ucontact_t uc;

    int rval;

    int i;
    struct id_r {
        char *id;
        int online;
        struct id_r *next;
    };
    struct id_r *idl;
    struct id_r *p1;
    int errcode = 0;

    /* allocate memory for id list */
    idl = (struct id_r *) pkg_malloc(sizeof(struct id_r));
    if (!idl) {
        LM_ERR("out of memory\n");
        return -2;
    }
    idl->id = NULL;
    idl->online = 0;
    idl->next = NULL;

    LM_DBG("here\n");
    if (!r->phoneid.s) {
        LM_ERR("phoneid is empty\n");
        errcode = -2;
        goto err2;
    }

    sprintf(buf,
            "select serial from phonedevpaired where phoneid = '%s';",
            r->phoneid.s);
    LM_DBG("query: %s\n", buf);
    query_str.s = buf;
    query_str.len = strlen(buf);
    if (dbf.raw_query(db_handle, &query_str, &res) < 0) {
        LM_ERR("query failed\n");
        errcode = -1;
        goto err2;
    }

    if (RES_ROW_N(res) == 0) {
        LM_DBG("empty result\n");
        dbf.free_result(db_handle, res);
        errcode = 0;
        goto err2;
    }

    p1 = idl;
    for (i = 0; i < RES_ROW_N(res); i++) {
        char *tmp;
        int n;
        tmp = RES_ROWS(res)[i].values[0].val.string_val;
        n = strlen(tmp);
        p1->id = (char *) pkg_malloc(n + 1);
        if (!(p1->id)) {
            LM_ERR("out of memory\n");
            errcode = -2;
            goto err2;
        }
        strcpy(p1->id, tmp);
        p1->online = 0;
        struct id_r *t;
        t = (struct id_r *) pkg_malloc(sizeof(struct id_r));
        if (!t) {
            LM_ERR("out of memory\n");
            errcode = -2;
            goto err2;
        }
        p1->next = t;
        p1 = p1->next;
        p1->id = NULL;
        p1->next = NULL;
    }
    dbf.free_result(db_handle, res);

    LM_DBG("allocate memory for body\n");

    body.len = 0;
    body.s = (char *) pkg_malloc(4096);
    if (!(body.s)) {
        LM_ERR("out of memory\n");
        errcode = -2;
        goto err2;
    }
    //strcpy(body.s,"devlist\r\n");
    sprintf(body.s, "devlist %s\r\n", r->phoneid.s);
    body.len = strlen(body.s);


    LM_DBG("allocate memory for contacts\n");
    all_contacts = pkg_malloc(cblen);
    if (!all_contacts) {
        LM_ERR("out of memory\n");
        errcode = -2;
        goto err3;
    }

    rval = ul.get_all_ucontacts(all_contacts, cblen, 0, 0, 1);
    if (rval < 0) {
        LM_ERR("failed to fetch contacts\n");
        errcode = -1;
        goto err4;
    }

    if (rval > 0) {
        if (all_contacts != NULL)
            pkg_free(all_contacts);
        cblen += (rval + 128);
        LM_DBG("re-malloc memory for contacts\n");
        all_contacts = pkg_malloc(cblen);
        if (!all_contacts) {
            LM_ERR("out of memory\n");
            errcode = -1;
            goto err3;
        }
        rval = ul.get_all_ucontacts(all_contacts, cblen, 0, 0, 1);
        if (rval != 0) {
            LM_ERR("get contacts failed2\n");
            errcode = -1;
            goto err4;
        }

    }
    if (all_contacts == NULL) {
        LM_ERR("null contacts\n");
        errcode = -1;
        goto err3;
    }

    LM_DBG("begin to parse contacts\n");

    pu = all_contacts;
    int n;
    char u[256];
    while (1) {
        memcpy(&n, pu, sizeof(n));
        if (n == 0)
            break;
        pu += sizeof(n);
        sscanf(pu, "sip:%[^@]", u); // get username from contact
        for (p1 = idl; p1 && p1->id; p1 = p1->next) {
            if (!(p1 && p1->id)) {
                LM_DBG("null p1 or p1->id\n");
                break;
            }
            if (strcmp(p1->id, u) == 0) {
                p1->online = 1;
            }
        }
        pu += n;
        memcpy(&n, pu, sizeof(n));
        pu += sizeof(n);        //path len
        pu += n;                // path
        pu += sizeof(uc.sock);  //skip socket_info
        pu += sizeof(uc.flags); // skip flags
        pu += sizeof(uc.next_hop);  // next hop
    }
    char tmp[10];
    for (p1 = idl; p1 && p1->id; p1 = p1->next) {
        if ((body.len + strlen(p1->id)) > 4092) {
            LM_ERR("more device, truncate.\n");
            break;
        }
        strcat(body.s, p1->id);
        body.len += strlen(p1->id);
        sprintf(tmp, " %d\r\n", p1->online);
        strcat(body.s, tmp);
        body.len += strlen(tmp);
    }
    LM_DBG("done\n");
    /* the return result */
    *r1 = (str *) pkg_malloc(sizeof(str));
    if (!(*r1)) {
        LM_ERR("out of memory\n");
        errcode = -2;
        goto err4;
    }
    (*r1)->s = (char *) pkg_malloc(body.len + 1);
    if (!((*r1)->s)) {
        LM_ERR("out of memory\n");
        errcode = -2;
        goto err4;
    }
    strcpy((*r1)->s, body.s);
    (*r1)->len = body.len;

  err4:
    LM_DBG("free contacts\n");
    if (all_contacts)
        pkg_free(all_contacts);
  err3:
    LM_DBG("free tmp body\n");
    if (body.s)
        pkg_free(body.s);
    //LM_DBG("body %s\n",(*r1)->s);

  err2:
    LM_DBG("free id_r\n");
    struct id_r *p2;
    for (p1 = idl; p1;) {
        p2 = p1->next;
        if (p1->id)
            pkg_free(p1->id);
        pkg_free(p1);
        p1 = p2;
    }
  err1:
    return errcode;
}
