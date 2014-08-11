#ifndef RC4_H
#define RC4_H

typedef struct rc4_key {
    unsigned char state[256];
    unsigned char x;
    unsigned char y;
} rc4_key;
char *key;
//rc4_key key_l;
//rc4_key key_r;
void prepare_key(unsigned char *key_data_ptr, int key_data_len,
                 rc4_key * key);
void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key * key);

#define ENCRYPT(buf, l) do{\
    rc4_key k1;\
    prepare_key((unsigned char *)key, strlen(key), &k1);\
    rc4((unsigned char *)buf, l, &k1);\
}while(0)

#define DECRYPT(buf, l) do{\
    rc4_key k1;\
    prepare_key((unsigned char *)key, strlen(key), &k1);\
    rc4((unsigned char *)buf, l, &k1);\
}while(0)

#endif
