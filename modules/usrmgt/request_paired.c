#include "usrmgt.h"

extern int verify_devid;

/* handle the paried request */
int handle_paired(struct request_msg *r)
{
    char query_buf[1024];       /* the buffer for SQL */
    db_res_t *res;
    str query_str;
    str vendor;                 // the vendor of devid
    str serial;                 // the serial of devid
    //str vendor_m;  
    //str serial_m;
    char *p2;
    int ret = 0;

    LM_DBG("devid: |%s|, phoneid: |%s|\n", r->devid.s, r->phoneid.s);

    if ((!r->devid.s) || (!r->phoneid.s)) { /* devid or phoneid is NULL */
        LM_ERR("invalid argument\n");
        ret = -1;
        goto err1;
    }

    p2 = strchr(r->devid.s, '_');   /* check if the format is vendor_serial */
    if (!p2) {
        LM_ERR("invalid devid\n");
        ret = -1;
        goto err1;
    }

    /* verify devid */
    if (verify_devid) {
        /* get the vendor */
        vendor.len = p2 - (r->devid.s);
        vendor.s = pkg_malloc(vendor.len + 1);
        if (!vendor.s) {
            LM_ERR("out of memory\n");
            ret = -1;
            goto err1;
        }
        strncpy(vendor.s, r->devid.s, vendor.len);
        vendor.s[vendor.len] = '\0';

        /* get the serial */
        serial.len = r->devid.len - vendor.len - 1;
        serial.s = pkg_malloc(serial.len + 1);
        if (!serial.s) {
            LM_ERR("out of memory\n");
            ret = -1;
            goto err2;
        }
        p2++;
        strncpy(serial.s, p2, serial.len);
        serial.s[serial.len] = '\0';

        /* check if the devid is exists */
        sprintf(query_buf,
                "select serial from %s where serial='%s';", vendor.s,
                serial.s);
        LM_DBG("query: %s\n", query_buf);
        query_str.s = query_buf;
        query_str.len = strlen(query_buf);

        /* execute the SQL */
        if (dbf.raw_query(db_handle2, &query_str, &res) < 0) {
            LM_ERR("failed to query devid\n");
            ret = -1;
            goto err3;
        }

        /* result row */
        if (RES_ROW_N(res) == 0) {
            LM_ERR("devid not exists\n");
            dbf.free_result(db_handle, res);
            ret = -2;
            goto err3;
        }

        /* free result */
        dbf.free_result(db_handle, res);
    }

    /* check if already paired */
    sprintf(query_buf,
            "select phoneid from phonedevpaired where phoneid='%s' and serial = '%s';",
            r->phoneid.s, r->devid.s);
    LM_DBG("query: %s\n", query_buf);
    query_str.s = query_buf;
    query_str.len = strlen(query_buf);
    if (dbf.raw_query(db_handle, &query_str, &res) < 0) {
        LM_ERR("failed to query paired devid\n");
        ret = -1;
        goto err3;
    }
    if (RES_ROW_N(res) > 0) {
        LM_DBG("devid phoneid already exists\n");
        dbf.free_result(db_handle, res);
        ret = 0;
        goto err1;
    }
    dbf.free_result(db_handle, res);

    /* not paired, insert a record */
    sprintf(query_buf,
            "insert into phonedevpaired (phoneid,serial,op_time) values('%s','%s',now());",
            r->phoneid.s, r->devid.s);

    query_str.s = query_buf;
    query_str.len = strlen(query_buf);
    if (dbf.raw_query(db_handle, &query_str, &res) < 0) {
        LM_ERR("failed to insert to database\n");
        ret = -1;
        goto err3;
    }
    dbf.free_result(db_handle, res);

    ret = 0;
  err3:
    pkg_free(serial.s);
  err2:
    pkg_free(vendor.s);
  err1:
    return ret;
}
