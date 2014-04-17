#include "usrmgt.h"


int license_get(struct request_msg *r, str ** ret)
{
    char buf[1024];
    char result[4096];
    str sql;
    db_res_t *res = NULL;
    if (r->active_code.s == NULL) {
        LM_ERR("active code is NULL\n");
        return -1;
    }
    if (r->active_code.len > 64) {
        LM_ERR("active code is too long\n");
        return -1;
    }
    sprintf(buf,
            "select devid, license from license where activecode = '%s' "
            "and date_add(start_time, interval ttl second) > now()",
            r->active_code.s);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        return -2;
    }

    if (RES_ROW_N(res) == 0) {
        if (res)
            dbf.free_result(db_handle, res);
        return 0;
    }
    if (RES_ROW_N(res) != 1) {
        LM_ERR("there many activecode\n");
        if (res)
            dbf.free_result(db_handle, res);
        return -2;
    }
    strcpy(result, "activecode ");
    strcat(result, r->active_code.s);
    strcat(result, "\r\n");
    strcat(result, "devid ");
    strcat(result, RES_ROWS(res)[0].values[0].val.string_val);
    strcat(result, "\r\n");
    strcat(result, "tmplic ");
    strcat(result, RES_ROWS(res)[0].values[1].val.string_val);
    strcat(result, "\r\n");
    dbf.free_result(db_handle, res);

    *ret = pkg_malloc(sizeof(str));

    if (!(*ret)) {
        LM_ERR("out of memory\n");
        return -2;
    }
    (*ret)->s = strdup(result);
    (*ret)->len = strlen((*ret)->s);

    return 0;
}

int license_put(struct request_msg *r, str ** ret)
{

    char buf[1024];
    str sql;
    db_res_t *res = NULL;
    LM_DBG("devid: %.*s, license: %.*s, active code: %.*s, ttl: %d\n",
           r->devid.len, r->devid.s,
           r->license.len, r->license.s,
           r->active_code.len, r->active_code.s, r->ttl);
    if (r->devid.s == NULL || r->active_code.s == NULL
        || r->license.s == NULL) {
        LM_ERR("devid, license, active code must be placed\n");
        return -1;
    }

    if (r->devid.len > 64 || r->license.len > 64
        || r->active_code.len > 64) {
        LM_ERR("some value is too long\n");
        return -1;
    }

    /* delete the expired value and previous value */
    sprintf(buf,
            "delete from license where date_add"
            "(start_time, interval ttl second) < now() or activecode = '%s'",
            r->active_code.s);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        return -2;
    }
    if (res) {
        dbf.free_result(db_handle, res);
        res = NULL;
    }

    /* store it */
    sprintf(buf,
            "insert into license (devid, license, activecode, ttl, start_time) "
            "values('%s', '%s', '%s', %d, now())",
            r->devid.s, r->license.s, r->active_code.s, r->ttl);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        return -2;
    }
    if (res) {
        dbf.free_result(db_handle, res);
        res = NULL;
    }
    return 0;
}
