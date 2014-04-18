#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include "../../str.h"
#include "../../sr_module.h"
//#include "mod_options.h"
#include "../../str.h"
#include "../../ut.h"
#include "../../mem/mem.h"
#include "../../data_lump_rpl.h"
#include "../../parser/msg_parser.h"
#include "../../parser/parse_uri.h"
#include "../signaling/signaling.h"
#include "../../db/db.h"
#include "../usrloc/usrloc.h"
#include "../../ip_addr.h"
#include <string.h>

#ifndef _USRMGT_H
#define _USRMGT_H

db_con_t *db_handle;            /* database connection handle */
db_con_t *db_handle2;           /* product information handle */
db_func_t dbf;                  /* database function handle */

/** SIGNALING binds */
struct sig_binds sigb;

struct request_msg {
    str func;                   /* operation */
    str devid;                  /* device account */
    str phoneid;                /* phone account */
    str snstype;                /* sns type */
    str snsname;                /* sns name */
    str license;                /* temporay license */
    str active_code;            /* active code */
    str uuid;                   /* serial */
    int ttl;                    /* time to live */
};

int parse_req_msg(char *b, struct request_msg *r);
void free_req_msg(struct request_msg *r);
int handle_paired(struct request_msg *r1, str ** ret);
int getpaireddev(struct request_msg *r1, str ** r2);
int getonlinelist(struct request_msg *r1, str ** s);
int handle_unbind(struct request_msg *r, str ** ret);
int license_put(struct request_msg *r, str ** ret);
int license_get(struct request_msg *r, str ** ret);
usrloc_api_t ul;
#endif
