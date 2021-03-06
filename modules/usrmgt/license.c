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
            "select devid, license, uuid from license where activecode = '%s' "
            "and date_add(start_time, interval ttl second) > now()",
            r->active_code.s);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        LM_ERR("db query failed\n");
        return -2;
    }

    if (RES_ROW_N(res) == 0) {
        if (res)
            dbf.free_result(db_handle, res);
        return 0;
    }
    if (RES_ROW_N(res) != 1) {
        LM_ERR("there too many activecode\n");
        if (res)
            dbf.free_result(db_handle, res);
        return -2;
    }

    result[0] = '\0';

    /* active code */
    strcat(result, "activecode ");
    strcat(result, r->active_code.s);
    strcat(result, "\r\n");

    /* devid */
    strcat(result, "devid ");
    strcat(result, RES_ROWS(res)[0].values[0].val.string_val);
    strcat(result, "\r\n");

    /* tmp license */
    strcat(result, "tmplic ");
    strcat(result, RES_ROWS(res)[0].values[1].val.string_val);
    strcat(result, "\r\n");

    /* uuid */
    strcat(result, "uuid ");
    strcat(result, RES_ROWS(res)[0].values[2].val.string_val);
    strcat(result, "\r\n");
    dbf.free_result(db_handle, res);

    *ret = pkg_malloc(sizeof(str));

    if (!(*ret)) {
        LM_ERR("out of memory\n");
        return -2;
    }

    (*ret)->len = strlen(result);
    (*ret)->s = pkg_malloc((*ret)->len + 1);
    if (!((*ret)->s)) {
        LM_ERR("out of memory\n");
        (*ret)->len = 0;
        return -2;
    }

    strcpy((*ret)->s, result);

    return 0;
}

int license_put(struct request_msg *r, str ** ret)
{

    char buf[1024];
    str sql;
    db_res_t *res = NULL;
    int exists = 0;

    LM_DBG
        ("devid: %.*s, license: %.*s, active code: %.*s, uuid: %.*s, ttl: %d\n",
         r->devid.len, r->devid.s, r->license.len, r->license.s,
         r->active_code.len, r->active_code.s, r->uuid.len, r->uuid.s,
         r->ttl);

    if (r->devid.s == NULL || r->active_code.s == NULL
        || r->license.s == NULL || r->uuid.s == NULL) {
        LM_ERR("devid, license, active code, uuid must be placed\n");
        return -1;
    }

    if (r->devid.len > 64 || r->license.len > 64
        || r->active_code.len > 64 || r->uuid.len > 32) {
        LM_ERR("some value is too long\n");
        return -1;
    }

    /* delete the expired value and previous value */
    sprintf(buf,
            "delete from license where date_add"
            "(start_time, interval ttl second) < now()"
            );
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        LM_ERR("delete expired active code from db failed\n");
        return -2;
    }
    if (res) {
        dbf.free_result(db_handle, res);
        res = NULL;
    }

    /* query if exists */
    sprintf(buf, "select count(*) from license where activecode= '%s'",
            r->active_code.s);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        LM_ERR("query db failed\n");
        return -2;
    }
    
    if (RES_ROWS(res)[0].values[0].val.int_val > 0){
        LM_WARN("active code %s is exists\n", r->active_code.s);
        exists = 1;
    }
    if (res) {
        dbf.free_result(db_handle, res);
        res = NULL;
    }
    if (exists){
        return -5;
    }
    /* store it */
    sprintf(buf,
            "insert into license (devid, license, activecode, uuid, ttl, start_time) "
            "values('%s', '%s', '%s', '%s', %d, now())",
            r->devid.s, r->license.s, r->active_code.s, r->uuid.s, r->ttl);
    sql.s = buf;
    sql.len = strlen(buf);
    if (dbf.raw_query(db_handle, &sql, &res) < 0) {
        LM_ERR("insert to db failed\n");
        return -2;
    }
    if (res) {
        dbf.free_result(db_handle, res);
        res = NULL;
    }
    return 0;
}
