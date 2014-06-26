#include "usrmgt.h"
#include <stdlib.h>

int get_string_value(char *keyname, char *buffer, str * dest);
int get_int_value(char *keyname, char *buffer, int *dest);

/* free the request_msg struct */
void free_req_msg(struct request_msg *r)
{
    if (r->func.s)
        pkg_free(r->func.s);
    r->func.s = NULL;
    r->func.len = 0;

    if (r->devid.s)
        pkg_free(r->devid.s);
    r->devid.s = NULL;
    r->devid.len = 0;

    if (r->phoneid.s)
        pkg_free(r->phoneid.s);
    r->phoneid.s = NULL;
    r->phoneid.len = 0;

    if (r->snstype.s)
        pkg_free(r->snstype.s);
    r->snstype.s = NULL;
    r->snstype.len = 0;

    if (r->snsname.s)
        pkg_free(r->snsname.s);
    r->snsname.s = NULL;
    r->snsname.len = 0;

    if (r->license.s)
        pkg_free(r->license.s);
    r->license.s = NULL;
    r->license.len = 0;

    if (r->active_code.s)
        pkg_free(r->active_code.s);
    r->active_code.s = NULL;
    r->active_code.len = 0;

    if (r->uuid.s)
        pkg_free(r->uuid.s);
    r->uuid.s = NULL;
    r->uuid.len = 0;

}

/* parse the message */
int parse_req_msg(char *b, struct request_msg *r)
{
    get_string_value("function", b, &r->func);
    get_string_value("devid", b, &r->devid);
    get_string_value("phoneid", b, &r->phoneid);
    get_string_value("snsname", b, &r->snsname);
    get_string_value("snstype", b, &r->snstype);
    get_string_value("tmplic", b, &r->license);
    get_string_value("activecode", b, &r->active_code);
    get_string_value("uuid", b, &r->uuid);
    get_int_value("ttl", b, &r->ttl);

    return 0;
}

int get_int_value(char *keyname, char *buffer, int *dest)
{
    str v = { 0, 0 };
    int v2 = 0;
    get_string_value(keyname, buffer, &v);
    if (v.s) {
        v2 = atoi(v.s);
        pkg_free(v.s);
    }

    *dest = v2;
    return 0;
}

int get_string_value(char *keyname, char *buffer, str * dest)
{
    char *p1, *p2;

    if (keyname == NULL || buffer == NULL) {
        return -3;
    }

    /* search for key */
    p1 = strcasestr(buffer, keyname);
    if (!p1) {
        return -1;
    }

    /* pointer to the end of key name */
    p1 += strlen(keyname);

    /* strip the space at the begin */
    while (isspace(*p1))
        p1++;

    /* search for delimeter */
    do {
        p2 = strchr(p1, '\r');
        if (p2)
            break;
        p2 = strchr(p1, '\n');
        if (p2)
            break;
        p2 = strchr(p1, '\0');
    } while (0);

    if (!p2) {
        return -2;
    }

    /* strip the space at the end */
    while (isspace(*p2))
        p2--;
    p2++;

    /* get length */
    dest->len = (p2 - p1);

    if (dest->len == 0) {
        return -4;
    }

    /* allocate memory */
    dest->s = (char *) pkg_malloc(dest->len + 1);

    if (!dest->s) {
        LM_ERR("out of memory\n");
        dest->len = 0;
        return -5;
    }

    /* copy to destination */
    strncpy(dest->s, p1, dest->len);
    dest->s[dest->len] = '\0';

    return 0;
}
