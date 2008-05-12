#ifndef _H_FILE
#define _H_FILE

class File {
public:
    File(std::string string);

    size_t read(size_t pos, size_t length);

    void read_all(std::string& buffer);

private:
    std::string file_hash;
    qword length;
    unsigned int level;
    std::string block_hashes, file_name;

    void read_block(dword i, unsigned char* buffer);
};

#endif
