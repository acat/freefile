// MakeURL.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <openssl/sha.h>
#include <openssl/aes.h>
#include <memory.h>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <queue>

#define BLOCK_SIZE (128*1024)

namespace fs = boost::filesystem;

char to_hex_char(unsigned char b)
{
    b &= 0x0F;

    if (b >= 0 && b <= 9)
        return '0' + b;

    return 'a' + b - 10;
}

void bin_hash_to_hex(const unsigned char* binhash, char* hexhash)
{
    for (int i=0; i<SHA_DIGEST_LENGTH; i++) {
        *(hexhash++) = to_hex_char(binhash[i] >> 4);
        *(hexhash++) = to_hex_char(binhash[i]);
    }
}

void check_and_create_path(const char* path)
{
    fs::path file_path(path);

    if (fs::exists(file_path))
        return;

    fs::create_directory(file_path);
}

void save_block(unsigned char* buffer, unsigned char* binhash, AES_KEY& aeskey)
{
    unsigned char iv[16];

    memset(iv, 0, sizeof(iv));
    AES_cbc_encrypt(buffer, buffer, BLOCK_SIZE, &aeskey, iv, AES_ENCRYPT);

    SHA_CTX ctx;
    SHA1_Init(&ctx);
    SHA1_Update(&ctx, buffer, BLOCK_SIZE);
    SHA1_Final(binhash, &ctx);

    char hex_hash[SHA_DIGEST_LENGTH * 2 + 1];
    bin_hash_to_hex(binhash, hex_hash);
    hex_hash[SHA_DIGEST_LENGTH * 2] = '\0';

    char file_name[64];
    sprintf(file_name, "%s.ffb", hex_hash);

    check_and_create_path("blocks");

    char file_dir[256];
    sprintf(file_dir, "blocks/%c%c", hex_hash[0], hex_hash[1]);
//    sprintf(file_dir, "blocks");
    check_and_create_path(file_dir);

    char file_path[256];
    sprintf(file_path, "%s/%s", file_dir, file_name);

    fs::ofstream os(file_path, std::ios_base::binary);
    os.write((const char *)buffer, BLOCK_SIZE);
}

int main(int argc, const char* argv[])
{
    unsigned char file_hash[SHA_DIGEST_LENGTH], block_hash[SHA_DIGEST_LENGTH];
    unsigned char index_buffer[BLOCK_SIZE];
    AES_KEY aeskey;

    if (argc != 2) {
        printf("MakeURL filename\n");
        return 1;
    }

    {
        unsigned char buffer[BLOCK_SIZE];
        SHA_CTX ctx;
        fs::ifstream is(argv[1], std::ios_base::binary);

        if (!is.good())
            return -1;

        SHA1_Init(&ctx);
        while (!is.eof()) {
            memset(buffer, 0, BLOCK_SIZE);
            is.read((char *)buffer, BLOCK_SIZE);
            std::streamsize bytes = is.gcount();
            if (bytes > 0)
                SHA1_Update(&ctx, buffer, BLOCK_SIZE);
        }
        SHA1_Final(file_hash, &ctx);
        AES_set_encrypt_key(file_hash, 16*8, &aeskey);
    }

    fs::ifstream is(argv[1], std::ios_base::binary);
    std::queue<std::string> hashs;
    memset(index_buffer, 0, BLOCK_SIZE);
    size_t length = 0;
    while (!is.eof()) {
        unsigned char buffer[BLOCK_SIZE];

        memset(buffer, 0, BLOCK_SIZE);

        is.read((char*)buffer, BLOCK_SIZE);
        std::streamsize bytes = is.gcount();

        if (bytes) {
            save_block(buffer, block_hash, aeskey);
            length += bytes;
            hashs.push(std::string((char*)block_hash, SHA_DIGEST_LENGTH));
        }
    }

    int levels = 0;
    while (hashs.size() > 10) {
        size_t size = hashs.size();
        while (size) {
            unsigned char buffer[BLOCK_SIZE];
            memset(buffer, 0, BLOCK_SIZE);
            size_t pos = 0;
            while (pos + SHA_DIGEST_LENGTH <= BLOCK_SIZE && size) {
                memmove(buffer + pos, hashs.front().c_str(), SHA_DIGEST_LENGTH);
                hashs.pop();
                pos += SHA_DIGEST_LENGTH;
                size--;
            }
            if (pos) {
                save_block(buffer, block_hash, aeskey);
                hashs.push(std::string((char*)block_hash, SHA_DIGEST_LENGTH));
            }
        }
        levels++;
    }

    fs::ofstream url("URL.txt", std::ios_base::app);
    url << "http://localhost:4490/FreeFile/V1/";
    {
        char hex_hash[SHA_DIGEST_LENGTH * 2 + 1];
        bin_hash_to_hex(file_hash, hex_hash);
        hex_hash[SHA_DIGEST_LENGTH * 2] = '\0';
        url << hex_hash;
    }
    url << "/" << length;
    url << "/" << levels;
    while (hashs.size() > 0) {
        char hex_hash[SHA_DIGEST_LENGTH * 2 + 1];
        bin_hash_to_hex((const unsigned char*)hashs.front().c_str(), hex_hash);
        hashs.pop();
        hex_hash[SHA_DIGEST_LENGTH * 2] = '\0';
        url << "/" << hex_hash;
    }
    url << "/" << argv[1] << std::endl;

	return 0;
}

