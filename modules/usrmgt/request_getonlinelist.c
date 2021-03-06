#include "usrmgt.h"
#include "../usrloc/ucontact.h"

int getonlinelist(struct request_msg *msg, str ** s)
{
    int rval;
    void *buf;
    int cblen = 4096;
    int n;
    ucontact_t uc;
    int ret = 0;

    //struct socket_info *send_sock;
    //unsigned int flags;
    buf = pkg_malloc(cblen);
    if (!buf) {
        LM_ERR("out of memory\n");
        ret = -2;
        goto err1;
    }
    rval = ul.get_all_ucontacts(buf, cblen, 0, 0, 1);
    if (rval < 0) {
        LM_ERR("failed to fetch contacts\n");
        ret = -1;
        goto err2;
    }
    if (rval > 0) {
        if (buf != NULL)
            pkg_free(buf);
        cblen += rval + 128;
        LM_DBG("remalloc for buf\n");
        buf = pkg_malloc(cblen);
        if (!buf) {
            LM_ERR("out of memory\n");
            ret = -2;
            goto err1;
        }
        rval = ul.get_all_ucontacts(buf, cblen, 0, 0, 1);
        if (rval != 0) {
            LM_ERR("get ucontacts failed\n");
            ret = -1;
            goto err2;
        }
    }
    if (buf == NULL) {
        LM_ERR("null buff\n");
        ret = -1;
        goto err1;
    }
    char *cp;
    cp = buf;
    *s = pkg_malloc(sizeof(str *));
    if (!(*s)) {
        LM_ERR("out of memory\n");
        ret = -2;
        goto err2;
    }
    (*s)->len = 0;
    (*s)->s = pkg_malloc(4096);
    if (!((*s)->s)) {
        LM_ERR("out of memory\n");
        ret = -2;
        goto err2;
    }
    strcpy((*s)->s, "userlist:");
    (*s)->len = strlen((*s)->s);
    int nt = (*s)->len;

    /* contact path sock flags nexthop */
    while (1) {
        char u[50];
        memcpy(&n, cp, sizeof(n));
        if (n == 0)
            break;
        cp += sizeof(n);
        LM_DBG("contact: %.*s\n", n, cp);
        sscanf(cp, "sip:%[^@]", u); /* get usrname, discard other */
        if (((*s)->len + strlen(u)) > 4095)
            break;              /* make sure we have enough memory */
        //if(((*s)->len+n) > 4095)break;
        if ((*s)->len != nt) {
            strcat((*s)->s, "|");
            ((*s)->len) += 1;
        }
        strncat((*s)->s, u, strlen(u));
        (*s)->len += strlen(u);
        //strncat((*s)->s,cp,n);
        //(*s)->len+=n;
        cp += n;

        memcpy(&n, cp, sizeof(n));
        cp += sizeof(n);
        LM_DBG("path: %.*s\n", n, cp);
        cp += n;
        cp += sizeof(uc.sock);
        cp += sizeof(uc.flags);
        cp += sizeof(uc.next_hop);
        //memcpy(&n,cp,sizeof(n));
        //cp+=(n+sizeof(n));
    }
    (*s)->s[(*s)->len] = '\0';
    LM_DBG("s->s %s\n", (*s)->s);
    ret = 0;
  err2:
    if (buf)
        pkg_free(buf);
  err1:
    return ret;
}
