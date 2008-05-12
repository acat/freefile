#include "stdafx.h"

#include "file.h"

#include "block.h"
#include <openssl/aes.h>

using namespace boost::spirit;

File::File(std::string string)
{
    uint_parser<byte, 16, 2, 2> hex_char_p;
    uint_parser<qword> qword_p;
    rule<> r = str_p("/FreeFile/V1/") >> repeat_p(20)[hex_char_p[push_back_a(file_hash)]] >> ch_p('/') >>
        qword_p[assign_a(length)] >> ch_p('/') >> uint_p[assign_a(level)] >> ch_p('/') >>
        +(repeat_p(20)[hex_char_p[push_back_a(block_hashes)]] >> ch_p('/'))
        >> +anychar_p[push_back_a(file_name)];

    parse_info<> hit = parse(string.c_str(), r);
    if (hit.hit && hit.full) {
    } else {
    }
}

size_t File::read(size_t pos, size_t length)
{
    return 0;
}

void File::read_all(std::string& buffer)
{
    buffer.clear();

    for (qword pos = 0; pos < length; pos += BLOCK_SIZE) {
        unsigned char buf[BLOCK_SIZE];

        read_block(pos / BLOCK_SIZE, buf);
        dword l = length - pos;
        if (l > BLOCK_SIZE)
            l = BLOCK_SIZE;

        buffer.append((char*)buf, l);
    }
}

void File::read_block(dword i, unsigned char* buffer)
{
    dword l = level + 1;
    std::string hashes = block_hashes;

    AES_KEY aeskey;
    AES_set_decrypt_key((const unsigned char*)file_hash.c_str(), 16*8, &aeskey);

    while (l) {
        dword step = 1;
        for (dword c=1; c<l; c++)
            step *= (BLOCK_SIZE / 20);

        dword index = i / step;
        std::string block_id = hashes.substr(index * 20, 20);
        block_read((const unsigned char*)block_id.c_str(), buffer);
        unsigned char iv[16];

        memset(iv, 0, sizeof(iv));
        AES_cbc_encrypt(buffer, buffer, BLOCK_SIZE, &aeskey, iv, AES_DECRYPT);
        hashes.clear();
        hashes.append((const char*)buffer, BLOCK_SIZE);
        i -= index * step;
        l--;
    }
}
