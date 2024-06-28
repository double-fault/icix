#include <stdio.h>


typedef struct _tmp {
    char name[28];  // up to 28 bytes of name in directory (including \0)
    int  inum;      // inode number of entry (-1 means entry not used)
} dir_ent_t;
typedef struct __dir_block_t {
	dir_ent_t entries[128];
} dir_block_t;


int main(void) {
	printf("%lu %lu\n", sizeof(dir_block_t), sizeof(dir_ent_t));
	return 0;
}


