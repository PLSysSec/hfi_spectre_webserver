#include <stdio.h>
#include <stdint.h>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include "sodium.h"

#include "../server_hostcalls.h"

char* to_hex(unsigned char* word, unsigned int len, unsigned int& out_len){
    out_len = len * 2;
    char* outword = new char[out_len  + 1];

    for(unsigned int i = 0; i < len; i++) {
        sprintf(outword+i*2, "%02X", word[i]);
    }

    return outword;
}

int main(int argc, char const *argv[])
{
    if (argc != 3) {
        std::cerr << "two arguments expected [(blank), msg, hash], got " << argc << " arg\n";
        abort();
    }

    char const * msg = argv[1];
    char const * msg_hash = argv[2];

    unsigned char hash[crypto_hash_sha256_BYTES];
    crypto_hash_sha256(hash, (char unsigned *) msg, strlen(msg));

    unsigned int out_size;
    char* hex = to_hex(hash, crypto_hash_sha256_BYTES, out_size);
    unsigned int actual_size = strlen(msg_hash);

    bool same = true;
    if (out_size == actual_size) {
        for(unsigned int i = 0; i < out_size; i++) {
            same = same && (hex[i] == msg_hash[i]);
        }
    } else {
        same = false;
    }

    if (!same) {
        char resultString[1000];
        snprintf(resultString, 1000, "Failed: Got %s, Expected %s\n", msg_hash, hex);
        server_module_string_result(resultString);
    } else {
        server_module_string_result("Succeeded\n");
    }
    delete[] hex;

    return 0;
}
