#include "usrmgt.h"

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
}

/* parse the message */
int parse_req_msg(char *b, struct request_msg *r)
{
    char *p1;
    char *p2;
    char *p3;

    /* pointer to start point */
    p1 = b;

    // search for "function" keyword
    p2 = strcasestr(p1, "function");
    if (p2) {
        p3 = NULL;
        do {
            p3 = strchr(p2, '\r');
            if (p3)
                break;
            p3 = strchr(p2, '\n');
            if (p3)
                break;
            p3 = strchr(p2, '\0');
        } while (0);

        if (!p3) {
            LM_ERR("CRLF not found after function line\n");
            goto error;
        }

        /* skip the space at end */
        while (isspace(*p3))
            p3--;
        p3++;

        p2 += strlen("function");

        if (!isspace(*p2)) {
            LM_ERR("function not followed by a space\n");
            goto error;
        }

        /* strip the space at start */
        while (isspace(*p2))
            p2++;

        r->func.len = (p3 - p2);    /* length */
        if (r->func.len <= 0) {
            LM_ERR("function is NULL\n");
            //return -1;
            goto error;
        }

        /* allocate the storage */
        r->func.s = (char *) pkg_malloc(r->func.len + 1);
        if (!r->func.s) {
            LM_ERR("out of memory\n");
            //return -1;
            goto error;
        }

        /* then strncpy may not set the termate for string */
        strncpy(r->func.s, p2, r->func.len);
        r->func.s[r->func.len] = '\0';

    } else {
        r->func.s = NULL;
        r->func.len = 0;
    }

    // search for devid
    p2 = strcasestr(p1, "devid");
    if (p2) {
        p3 = NULL;
        do {
            p3 = strchr(p2, '\r');
            if (p3)
                break;
            p3 = strchr(p2, '\n');
            if (p3)
                break;
            p3 = strchr(p2, '\0');
        } while (0);
        if (!p3) {
            LM_ERR("CRLF not found after devid line\n");
            //return -1;
            goto error;
        }
        while (isspace(*p3))
            p3--;               /* strip the space at end */
        p3++;

        p2 += strlen("devid");
        if (!isspace(*p2)) {
            LM_ERR("a space is not followed by the devid\n");
            goto error;
        }
        while (isspace(*p2))
            p2++;               /* strip the space at start */

        r->devid.len = (p3 - p2);
        if (r->devid.len <= 0) {
            LM_ERR("devid is NULL\n");
            //return -1;
            goto error;
        }
        r->devid.s = (char *) pkg_malloc(r->devid.len + 1);
        if (!r->devid.s) {
            LM_ERR("out of memory\n");
            //return -1;
            goto error;
        }

        strncpy(r->devid.s, p2, r->devid.len);
        r->devid.s[r->devid.len] = '\0';
    } else {
        r->devid.s = NULL;
        r->devid.len = 0;
    }

    // search for phoneid
    p2 = strcasestr(p1, "phoneid");
    if (p2) {
        p3 = NULL;
        do {
            p3 = strchr(p2, '\r');
            if (p3)
                break;
            p3 = strchr(p2, '\n');
            if (p3)
                break;
            p3 = strchr(p2, '\0');
        } while (0);
        if (!p3) {
            LM_ERR("CRLF not found after phoneid line\n");
            //return -1;
            goto error;
        }

        /* strip the space at end */
        while (isspace(*p3))
            p3--;
        p3++;

        p2 += strlen("phoneid");
        if (!isspace(*p2)) {
            LM_ERR("a space is not followed by the phoneid\n");
            goto error;
        }
        /* strip the space at start */
        while (isspace(*p2))
            p2++;

        r->phoneid.len = (p3 - p2);
        if (r->phoneid.len <= 0) {
            LM_ERR("phoneid is NULL\n");
            //return -1;
            goto error;
        }

        r->phoneid.s = (char *) pkg_malloc(r->phoneid.len + 1);
        if (!r->phoneid.s) {
            LM_ERR("out of memory\n");
            //return -1;
            goto error;
        }

        strncpy(r->phoneid.s, p2, r->phoneid.len);
        r->phoneid.s[r->phoneid.len] = '\0';
    } else {
        r->phoneid.s = NULL;
        r->phoneid.len = 0;
    }

    // search for snstype
    p2 = strcasestr(p1, "snstype");
    if (p2) {
        p3 = NULL;
        do {
            p3 = strchr(p2, '\r');
            if (p3)
                break;
            p3 = strchr(p2, '\n');
            if (p3)
                break;
            p3 = strchr(p2, '\0');
        } while (0);
        if (!p3) {
            LM_ERR("CRLF not found after snstype line\n");
            //return -1;
            goto error;
        }
        while (isspace(*p3))
            p3--;
        p3++;

        p2 += strlen("snstype");
        if (!isspace(*p2)) {
            LM_ERR("a space is not followed by the snstype\n");
            goto error;
        }
        while (isspace(*p2))
            p2++;

        r->snstype.len = (p3 - p2);
        if (r->snstype.len <= 0) {
            LM_ERR("snstype is NULL\n");
            //return -1;
            goto error;
        }

        r->snstype.s = (char *) pkg_malloc(r->snstype.len + 1);
        if (!r->snstype.s) {
            LM_ERR("out of memory\n");
            //return -1;
            goto error;
        }

        strncpy(r->snstype.s, p2, r->snstype.len);
        r->snstype.s[r->snstype.len] = '\0';
    } else {
        r->snstype.s = NULL;
        r->snstype.len = 0;
    }

    // search for snsname
    p2 = strcasestr(p1, "snsname");
    if (p2) {
        p3 = NULL;
        do {
            p3 = strchr(p2, '\r');
            if (p3)
                break;
            p3 = strchr(p2, '\n');
            if (p3)
                break;
            p3 = strchr(p2, '\0');
        } while (0);
        if (!p3) {
            LM_ERR("CRLF not found after snsname line\n");
            //return -1;
            goto error;
        }

        while (isspace(*p3))
            p3--;
        p3++;

        p2 += strlen("snsname");
        if (!isspace(*p2)) {
            LM_ERR("a space is not followed by the snsname\n");
            goto error;
        }
        while (isspace(*p2))
            p2++;

        r->snsname.len = (p3 - p2);
        if (r->snsname.len <= 0) {
            LM_ERR("snsname is NULL\n");
            //return -1;
            goto error;
        }

        r->snsname.s = (char *) pkg_malloc(r->snsname.len + 1);
        if (!r->snsname.s) {
            LM_ERR("out of memory\n");
            //return -1;
            goto error;
        }

        strncpy(r->snsname.s, p2, r->snsname.len);
        r->snsname.s[r->snsname.len] = '\0';
    } else {
        r->snsname.s = NULL;
        r->snsname.len = 0;
    }
    return 0;

  error:                       /* on error, free the memory */
    free_req_msg(r);
    return -1;
}
