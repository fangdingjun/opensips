//#include <stdio.h>
#include <string.h>
//#include <stdlib.h>
#include "rc4.h"

#define buf_size 1024

//struct rc4_key;
/*
typedef struct rc4_key {
    unsigned char state[256];
    unsigned char x;
    unsigned char y;
} rc4_key;
*/
#define swap_byte(x,y) t = *(x); *(x) = *(y); *(y) = t

void prepare_key(unsigned char *key_data_ptr, int key_data_len,
                 rc4_key * key)
{
    unsigned char t;
    unsigned char index1;
    unsigned char index2;
    unsigned char *state;
    short counter;

    state = &key->state[0];
    for (counter = 0; counter < 256; counter++) {
        state[counter] = counter;
    }
    key->x = 0;
    key->y = 0;
    index1 = 0;
    index2 = 0;
    for (counter = 0; counter < 256; counter++) {
        index2 = (key_data_ptr[index1] + state[counter] + index2) % 256;
        swap_byte(&state[counter], &state[index2]);
        index1 = (index1 + 1) % key_data_len;
    }
}

void rc4(unsigned char *buffer_ptr, int buffer_len, rc4_key * key)
{
    unsigned char t;
    unsigned char x;
    unsigned char y;
    unsigned char *state;
    unsigned char xorIndex;
    short counter;

    x = key->x;
    y = key->y;
    state = &key->state[0];
    for (counter = 0; counter < buffer_len; counter++) {
        x = (x + 1) % 256;
        y = (state[x] + y) % 256;
        swap_byte(&state[x], &state[y]);
        xorIndex = (state[x] + state[y]) % 256;
        buffer_ptr[counter] ^= state[xorIndex];
    }
    key->x = x;
    key->y = y;
}
/*
int main(int argc, char *argv[])
{
    char data[512];
    char buf[buf_size];
    int rd;
    int n;
    rc4_key key;

    if (argc < 2) {
        fprintf(stderr, "%s key <in >out\n", argv[0]);
        exit(1);
    }
    strcpy(data, argv[1]);
    n = strlen(data);
    prepare_key((unsigned char *) data, n, &key);
    rd = fread(buf, 1, buf_size, stdin);
    while (rd > 0) {
        rc4((unsigned char *) buf, rd, &key);
        fwrite(buf, 1, rd, stdout);
        rd = fread(buf, 1, buf_size, stdin);
    }
}
*/
