#include "usrmgt.h"

int handle_unbind(struct request_msg *r, str ** resb)
{
    int ret = 0;
    char query_buf[1024];
    db_res_t *res;
    str qstr;
    if ((!r->devid.s) || (!r->phoneid.s)) {
        LM_ERR("invalid argument\n");
        ret = -1;
        goto err1;
    }
    sprintf(query_buf,
            "delete from phonedevpaired where serial='%.*s' and phoneid='%.*s';",
            r->devid.len, r->devid.s, r->phoneid.len, r->phoneid.s);
    qstr.s = query_buf;
    qstr.len = strlen(query_buf);
    if (dbf.raw_query(db_handle, &qstr, &res) < 0) {
        LM_ERR("db query failed\n");
        ret = -1;
        goto err1;
    }
    dbf.free_result(db_handle, res);
    ret = 0;
  err1:
    return ret;
}
