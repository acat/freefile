#include "stdafx.h"

#include <openssl/sha.h>

#include "block.h"

namespace fs = boost::filesystem;

static char to_hex_char(unsigned char b)
{
    b &= 0x0F;

    if (b >= 0 && b <= 9)
        return '0' + b;

    return 'a' + b - 10;
}

static void bin_hash_to_hex(const unsigned char* binhash, char* hexhash)
{
    for (int i=0; i<SHA_DIGEST_LENGTH; i++) {
        *(hexhash++) = to_hex_char(binhash[i] >> 4);
        *(hexhash++) = to_hex_char(binhash[i]);
    }
}

void block_read(const unsigned char* block_id, unsigned char* buffer)
{
    char hex_hash[SHA_DIGEST_LENGTH * 2 + 1];
    bin_hash_to_hex(block_id, hex_hash);
    hex_hash[SHA_DIGEST_LENGTH * 2] = '\0';

    char file_path[256];
    sprintf(file_path, "blocks/%c%c/%s.ffb", hex_hash[0], hex_hash[1], hex_hash);

    fs::ifstream os(file_path, std::ios_base::binary);
    os.read((char *)buffer, BLOCK_SIZE);
}
