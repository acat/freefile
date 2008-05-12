#ifndef _H_BLOCK
#define _H_BLOCK

#define BLOCK_SIZE (128*1024)

void block_read(const unsigned char* block_id, unsigned char* buffer);

#endif
