#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include <kernel/filesystem.h>
#include <kernel/pmm.h>
#include <kernel/disk_driver.h>

//#define DEBUG

/* utilities start */

// in this bitmap 0 refers to MSB
void set_bitmap(bitmap_t b, int i) {
	b[i / 32] |= 1 << (31 - i);
}

void reset_bitmap(bitmap_t b, int i) {
	b[i / 32] &= ~(1 << (31 - i));
}

int get_bitmap(bitmap_t b, int i) {
	return b[i / 32] & (1 << (31 - i)) ? 1 : 0;
}

int get_bitmap_sz(int n) {
	return (n + 31) / 32;
}

int Read(disk_driver_t *disk, int addr, void *buf, size_t count) {
	int rc = (*(disk->lseek))(addr);
	if (rc == -1) return -1;

	rc = (*(disk->read))(buf, count);
	if ((size_t)rc != count) return -1;
	return 0;
}

int Write(disk_driver_t *disk, int addr, void *buf, size_t count) {
	int rc = (*(disk->lseek))(addr);
	if (rc == -1) return -1;

	rc = (*(disk->write))(buf, count);
	if ((size_t)rc != count) return -1;
	return 0;
}

/* utilities end */

/*
typedef struct __ufs {
	int fd;
	super_t s;
	bitmap_t inode_bp; // inode bitmap
	int inode_bp_sz; // inode bitmap size (size of inode_bp array)
	bitmap_t data_bp; // data bitmap
        int data_bp_sz; //data bitmap size
	inode_t *inodes;

	//recording dirty parts to make writes convenient
	//u would think its for caching writes or smth, but no, this
	//system aint even close to efficient lol
	bitmap_t dirty_inode_bp;
	bitmap_t dirty_data_bp;
} ufs;

typedef struct __dir_block_t {
	dir_ent_t entries[128];
} dir_block_t;*/

void ufs_clean([[maybe_unused]]ufs *nfs) {
	/*
	free(nfs->inode_bp);
	free(nfs->data_bp);
	free(nfs->inodes);
	free(nfs->dirty_inode_bp);
	free(nfs->dirty_data_bp);
	close(nfs->fd);
	free(nfs);
	*/
}

void print_superblock(super_t s) {
	printf("=======================\n");
	printf("superblock print\n");
	printf("inode_bitmap_addr %d\n", s.inode_bitmap_addr);
	printf("inode_bitmap_len %d\n", s.inode_bitmap_len);
	printf("data_bitmap_addr %d\n", s.data_bitmap_addr);
	printf("data_bitmap_len %d\n", s.data_bitmap_len);
	printf("inode_region_addr %d\n", s.inode_region_addr);
	printf("inode_region_len %d\n", s.inode_region_len);
	printf("data_region_addr %d\n", s.data_region_addr);
	printf("data_region_len %d\n", s.data_region_len);
	printf("num_inodes %d\n", s.num_inodes);
	printf("num_data %d\n", s.num_data);
	printf("=======================\n");
}

void print_bitmaps(ufs *nfs) {
	printf("=======================\n");
	printf("inode bitmap: ");
	for (int i = 0; i < nfs->inode_bp_sz * (int)sizeof(unsigned int) * 8; ++i) {
		printf("%d", get_bitmap(nfs->inode_bp, i));
	}
	printf("\ndata bitmap: ");
	for (int i = 0; i < nfs->data_bp_sz * (int)sizeof(unsigned int) * 8; ++i) {
		printf("%d", get_bitmap(nfs->data_bp, i));
	}
	puts("");
	printf("=======================\n");
}

void print_inodes(ufs *nfs) {
	printf("=======================\n");
	printf("Printing all inodes...\n");
	for (int i =0; i < nfs->inode_bp_sz * (int)sizeof(unsigned int) * 8; ++i) {
		if (get_bitmap(nfs->inode_bp, i)) {
			printf("-----Found inode %d-----\n", i);
			printf("type %d\n", nfs->inodes[i].type);
			printf("size %d\n", nfs->inodes[i].size);
			for (int j = 0; j < DIRECT_PTRS; ++j) {
				printf("dptr %d: %u\n", j, nfs->inodes[i].direct[j]);
			}
		}
	}
	printf("=======================\n");
}

ufs* ufs_init(disk_driver_t *disk) {
	assert(sizeof(dir_ent_t) * 128 == UFS_BLOCK_SIZE);

	ufs* nfs = (ufs*)malloc(sizeof(ufs));
	nfs->disk = disk;

	int rc = (*(nfs->disk->read))( &nfs->s, sizeof(super_t));
	if (rc != sizeof(super_t)) {
		printf( "ufs_init superblock read fail\n");
		abort();
	}

#ifdef DEBUG
	print_superblock(nfs->s);
#endif

	nfs->inode_bp_sz = get_bitmap_sz(nfs->s.num_inodes);
	nfs->data_bp_sz = get_bitmap_sz(nfs->s.num_data);
	nfs->inode_bp = (bitmap_t)malloc(nfs->inode_bp_sz * sizeof(unsigned int));
	nfs->data_bp = (bitmap_t)malloc(nfs->data_bp_sz * sizeof(unsigned int));
	nfs->dirty_inode_bp = (bitmap_t)malloc(nfs->inode_bp_sz * sizeof(unsigned int));
	nfs->dirty_data_bp = (bitmap_t)malloc(nfs->data_bp_sz * sizeof(unsigned int));

	rc = (*(nfs->disk->lseek))(nfs->s.inode_bitmap_addr * UFS_BLOCK_SIZE);
	if (rc == -1) {
		printf("ufs_init inode bitmap lseek fail");
		abort();
	}

	rc = (*(nfs->disk->read))(nfs->inode_bp, nfs->inode_bp_sz * sizeof(unsigned int));
	if (rc != nfs->inode_bp_sz * (int)sizeof(unsigned int)) {
		printf( "ufs_init inode bitmap read fail\n");
		abort();
	}

	rc = (*(nfs->disk->lseek))(nfs->s.data_bitmap_addr * UFS_BLOCK_SIZE);
	if (rc == -1) {
		printf("ufs_init data bitmap lseek fail");
		abort();
	}

	rc = (*(nfs->disk->read))(nfs->data_bp, nfs->data_bp_sz * sizeof(unsigned int));
	if (rc != nfs->data_bp_sz * (int)sizeof(unsigned int)) {
		printf( "ufs_init data bitmap read fail\n");
		abort();
	}

#ifdef DEBUG
	print_bitmaps(nfs);
#endif

	rc = (*(nfs->disk->lseek))(nfs->s.inode_region_addr * UFS_BLOCK_SIZE);
	if (rc == -1) {
		printf("ufs_init itable lseek fail");
		abort();
	}

	nfs->inodes = (inode_t*)malloc(sizeof(inode_t) * nfs->s.num_inodes);
	rc = (*(nfs->disk->read))(nfs->inodes, sizeof(inode_t) * nfs->s.num_inodes);
	if (rc != (int)sizeof(inode_t) * nfs->s.num_inodes) {
		printf( "ufs_init inode table read fail\n");
		abort();
	}

#ifdef DEBUG
	print_inodes(nfs);
#endif
	return nfs;
}

// I just realized this is inefficient af, can do better by reading in whole dir_block at once...
int ufs_lookup(ufs *nfs, int pinum, char *name) {
	if (pinum < 0 || pinum >= nfs->s.num_inodes) return -2;
	if (!get_bitmap(nfs->inode_bp, pinum)) return -3;
	if (nfs->inodes[pinum].type != UFS_DIRECTORY) return -4;

	inode_t inode = nfs->inodes[pinum];

	int dir_ent_cnt = inode.size / sizeof(dir_ent_t);

	if (inode.size % sizeof(dir_ent_t)) {
		printf( "ufs_lookup inode size not divisible by dir_ent_t size, probably corrupted\n");
		abort();
	}

        for (int i = 0; i < DIRECT_PTRS && dir_ent_cnt; ++i) {
		int rc = (*(nfs->disk->lseek))(inode.direct[i] * UFS_BLOCK_SIZE);
 		if (rc == -1) {
 			printf("ufs_lookup lseek fail, probably corrupted inode table");
 			abort();
 		}

		int dcnt_in_block = UFS_BLOCK_SIZE / sizeof(dir_ent_t);
		for (int j = 0; j < dcnt_in_block; ++j) {
 			dir_ent_t dir_ent;
 			rc = (*(nfs->disk->read))(&dir_ent, sizeof(dir_ent_t));
			if (rc != sizeof(dir_ent_t)) {
				printf("ufs_lookup dir_ent read fail");
				abort();
			}

			assert(dir_ent.inum != -1);
			if (!strcmp(name, dir_ent.name)) return dir_ent.inum;

			dir_ent_cnt--;
			if (!dir_ent_cnt) break;
		}
	}
	return -1;
}

// Don't forget to fsync afterwards!!
int commit_dirty_to_disk(ufs *nfs) {
	for (int i = 0; i < nfs->s.num_inodes; ++i) {
		if (!get_bitmap(nfs->dirty_inode_bp, i)) continue;
		reset_bitmap(nfs->dirty_inode_bp, i);

		// there can be a few repeated writes here which can be avoided..
		int idx = i / 32;
		int addr = nfs->s.inode_bitmap_addr * UFS_BLOCK_SIZE + idx * sizeof(unsigned int);
		if (Write(nfs->disk, addr, &nfs->inode_bp[idx], sizeof(unsigned int)) == -1) return -1;

		addr = nfs->s.inode_region_addr * UFS_BLOCK_SIZE + i * sizeof(inode_t);
		if (Write(nfs->disk, addr, &nfs->inodes[i], sizeof(inode_t)) == -1) return -1;
	}

	for (int i = 0; i < nfs->s.num_data; ++i) {
		if (!get_bitmap(nfs->dirty_data_bp, i)) continue;
		reset_bitmap(nfs->dirty_data_bp, i);

		int idx = i / 32;
		int addr = nfs->s.data_bitmap_addr * UFS_BLOCK_SIZE + idx * sizeof(unsigned int);
		if (Write(nfs->disk, addr, &nfs->data_bp[idx], sizeof(unsigned int)) == -1) return -1;
	}
	return 0;
}

//assumes that name is null-terminated, not sure how to verify it properly lmao...
int ufs_creat(ufs *nfs, int pinum, int type, char *name) {
	if (pinum < 0 || pinum >= nfs->s.num_inodes) return -1;
	if (!get_bitmap(nfs->inode_bp, pinum)) return -1;
	inode_t dir_inode = nfs->inodes[pinum];
	if (dir_inode.type != UFS_DIRECTORY) return -1;

	int len = strlen(name);
	if (len > 27) return -1;

	// already exists
	if (ufs_lookup(nfs, pinum, name) != -1) return -1;

	// First find an empty inode and empty data block
	int empty_pos_inode = -1;
	for (int i = 0; i < nfs->s.num_inodes; ++i) {
		if (!get_bitmap(nfs->inode_bp, i)) {
			empty_pos_inode = i;
			break;
		}
	}

	int empty_pos_data = -1, empty_pos_data2 = -1;
	for (int i = 0; i < nfs->s.num_data; ++i) {
		if (!get_bitmap(nfs->data_bp, i)) {
			if(empty_pos_data == -1) empty_pos_data = i;
			else if (empty_pos_data2 == -1) empty_pos_data2 = i;
		}
	}

	if (empty_pos_inode == -1) return -1;

	/*
	 * there are some bugs with error handling - for eg,
	 * Im setting the bitmap here, but there might be an error later on
	 * and the function might return the code for failure.. but everything should work alright
	 * provided there are no errors which should be the case provided we don't hit the max
	 * filesystem capacity.
	 */
	set_bitmap(nfs->inode_bp, empty_pos_inode);
	set_bitmap(nfs->dirty_inode_bp, empty_pos_inode);
	inode_t* inode = &nfs->inodes[empty_pos_inode];
	inode->type = type;
	for (int i = 0; i < DIRECT_PTRS; ++i) inode->direct[i] = -1;

	if (nfs->inodes[pinum].size == DIRECT_PTRS * UFS_BLOCK_SIZE) return -1;
	int is_pinode_full = 1;
	for (int i = 0; i < DIRECT_PTRS; ++i) {
		if (nfs->inodes[pinum].direct[i] == (unsigned int)(-1)) continue;

		dir_block_t dir_block;
		if (Read(nfs->disk, nfs->inodes[pinum].direct[i] * UFS_BLOCK_SIZE,
					&dir_block, UFS_BLOCK_SIZE) == -1) {
			printf( "ufs_creat read fail\n");
			abort();
		}

		for (int j = 0; j < 128; j++) {
			if (dir_block.entries[j].inum == -1) {
				is_pinode_full = 0;
				break;
			}
		}
		if (!is_pinode_full) break;
	}

	if (type == UFS_DIRECTORY) {
		if (empty_pos_data == -1) return -1;
		// we might require >= 2 data blocks
		// gotta do this check here itself once as we r writing to disk
		if (is_pinode_full && empty_pos_data2 == -1)
			return -1;
		inode->size = 2 * sizeof(dir_ent_t);
		inode->direct[0] = empty_pos_data + nfs->s.data_region_addr;

		// gotta allocate data block as well and put in . and ..

		set_bitmap(nfs->data_bp, empty_pos_data);
		set_bitmap(nfs->dirty_data_bp, empty_pos_data);

		dir_block_t data;
		strcpy(data.entries[0].name, ".");
		data.entries[0].inum = empty_pos_inode;
		strcpy(data.entries[1].name, "..");
		data.entries[1].inum = pinum;

		for (int i = 2; i < 128; ++i) data.entries[i].inum = -1;

		if (Write(nfs->disk, inode->direct[0] * UFS_BLOCK_SIZE, &data, sizeof(data)) == -1) {
			printf( "ufs_creat write fail\n");
			abort();
		}
		empty_pos_data = empty_pos_data2;
	} else {
		inode->size = 0;
	}

	//time to update parent
	set_bitmap(nfs->dirty_inode_bp, pinum);
	if (is_pinode_full) {
		if (empty_pos_data == -1) return -1;
		if (nfs->inodes[pinum].size == DIRECT_PTRS * UFS_BLOCK_SIZE) return -1;

		dir_block_t data;
		data.entries[0].inum = empty_pos_inode;
		strcpy(data.entries[0].name, name);
		for (int i = 1; i < 128; ++i) {
			data.entries[i].inum = -1;
		}

		int addr = empty_pos_data + nfs->s.data_region_addr; // in blocks

		if (Write(nfs->disk, addr * UFS_BLOCK_SIZE, &data, sizeof(data)) == -1) {
			printf( "ufs_creat write fail\n");
			abort();
		}

		set_bitmap(nfs->data_bp, empty_pos_data);
		set_bitmap(nfs->dirty_data_bp, empty_pos_data);

		for (int i = 0; i < DIRECT_PTRS; ++i) {
			if (nfs->inodes[pinum].direct[i] == (unsigned int)(-1)) {
				nfs->inodes[pinum].direct[i] = addr;
				break;
			}
		}
	} else {
		dir_block_t *data = (dir_block_t*)malloc(sizeof(dir_block_t));
		for (int i = 0; i < DIRECT_PTRS; ++i) {
			if (Read(nfs->disk, nfs->inodes[pinum].direct[i] * UFS_BLOCK_SIZE, data, UFS_BLOCK_SIZE) == -1) {
				printf( "ufs_creat read fail\n");
				abort();
			}

			int empty_idx = -1;
			for (int j = 0; j < 128; ++j) {
				if (data->entries[j].inum == -1) {
					empty_idx = j;
					break;
				}
			}
			if (empty_idx == -1) continue;

			dir_ent_t dent;
			strcpy(dent.name, name);
			dent.inum = empty_pos_inode;

			if (Write(nfs->disk, (nfs->inodes[pinum].direct[i] * UFS_BLOCK_SIZE) +
						(empty_idx * sizeof(dir_ent_t)),
						&dent, sizeof(dent)) == -1) {
				printf( "ufs_creat write fail\n");
				abort();
			}
			break;
		}
		//free(data);
	}
	nfs->inodes[pinum].size += sizeof(dir_ent_t);

	if (commit_dirty_to_disk(nfs) == -1) {
		printf( "ufs_creat couldn't commit dirty to disk\n");
		abort();
	}

	(*(nfs->disk->fsync))(); // Important
	return 0;
}

int ufs_write(ufs *nfs, int inum, char *buf, int offset, int nbytes) {
	if (inum < 0 || inum >= nfs->s.num_inodes) return -1;
	if (!get_bitmap(nfs->inode_bp, inum)) return -1;
	if (nbytes > 4096 || offset > nfs->inodes[inum].size
			|| nfs->inodes[inum].type != UFS_REGULAR_FILE ||
			offset < 0 || nbytes <= 0) return -1;

	int strt = offset / UFS_BLOCK_SIZE;
	offset %= UFS_BLOCK_SIZE;
	int cur = 0;
	set_bitmap(nfs->dirty_inode_bp, inum);
	for (int i = strt; i < DIRECT_PTRS && cur < nbytes; ++i) {
		if (nfs->inodes[inum].direct[i] == (unsigned int)(-1)) {
			int empty_block = -1;
			for (int j = 0; j < nfs->s.num_data; ++j) {
				if (!get_bitmap(nfs->data_bp, j)) {
					empty_block = j;
					break;
				}
			}
			if (empty_block == -1) return -1;

			set_bitmap(nfs->data_bp, empty_block);
			set_bitmap(nfs->dirty_data_bp, empty_block);
			nfs->inodes[inum].direct[i] = empty_block + nfs->s.data_region_addr;
		}

		int sz = nbytes - cur;
		if (sz > UFS_BLOCK_SIZE - offset) sz = UFS_BLOCK_SIZE - offset;

		if (Write(nfs->disk, nfs->inodes[inum].direct[i] * UFS_BLOCK_SIZE + offset,
					buf + cur, sz) == -1) {
			printf( "ufs_write fail\n");
		       abort();
		}

		cur += sz;
		nfs->inodes[inum].size += sz;
		offset = 0;
	}

	if (commit_dirty_to_disk(nfs) == -1) {
		printf( "ufs_write commit dirty to disk fail\n");
		abort();
	}
	(*(nfs->disk->fsync))();
	return 0;
}

int ufs_read(ufs *nfs, int inum, char *buffer, int offset, int nbytes) {
       if (inum < 0 || inum >= nfs->s.num_inodes) return -1;
       if (!get_bitmap(nfs->inode_bp, inum)) return -1;

       if (offset < 0 || nbytes <= 0 || offset + nbytes > nfs->inodes[inum].size) return -1;

       int strt = offset / UFS_BLOCK_SIZE;
       int cur = 0;
       offset %= UFS_BLOCK_SIZE;

       if (nfs->inodes[inum].type == UFS_DIRECTORY && offset % sizeof(dir_ent_t)) return -1;

       for (int i = strt; i < DIRECT_PTRS && cur < nbytes; ++i) {
	     int sz = nbytes - cur;
	     if (sz > UFS_BLOCK_SIZE - offset) sz = UFS_BLOCK_SIZE - offset;

	     if (Read(nfs->disk, nfs->inodes[inum].direct[i] * UFS_BLOCK_SIZE + offset,
				    buffer + cur, sz) == -1) {
		    printf( "ufs_read fail\n");
		    abort();
	     }

	     cur += sz;
	     offset = 0;
       }
       return 0;
}

int ufs_unlink(ufs *nfs, int pinum, char *name) {
       if (pinum < 0 || pinum >= nfs->s.num_inodes) return -1;
       if (!get_bitmap(nfs->inode_bp, pinum)) return -1;
       if (!strcmp(name, ".") || !strcmp(name, "..")) return -1;

       int inum = ufs_lookup(nfs, pinum, name);
       if (inum == -1) return -1;

       if (nfs->inodes[inum].type == UFS_DIRECTORY && nfs->inodes[inum].size != 2 * sizeof(dir_ent_t)) return -1;

       reset_bitmap(nfs->inode_bp, inum);
       set_bitmap(nfs->dirty_inode_bp, inum);
       set_bitmap(nfs->dirty_inode_bp, pinum);

       for (int i = 0; i < DIRECT_PTRS; ++i) {
	       if (nfs->inodes[inum].direct[i] == (unsigned int)(-1)) continue;
	       reset_bitmap(nfs->data_bp, nfs->inodes[inum].direct[i]);
	       set_bitmap(nfs->dirty_data_bp, nfs->inodes[inum].direct[i]);
       }

       //parent updation time
       nfs->inodes[pinum].size -= sizeof(dir_ent_t);
       for (int i = 0; i < DIRECT_PTRS; ++i) {
	       if (nfs->inodes[pinum].direct[i] == (unsigned int)(-1)) continue;
	       dir_block_t dir_block;
	       if (Read(nfs->disk, nfs->inodes[pinum].direct[i] * UFS_BLOCK_SIZE, &dir_block, UFS_BLOCK_SIZE) == -1) {
		       printf( "ufs_unlink pinode data block read fail\n");
		       abort();
	       }

	       int entry_idx = -1, cnt = 0;
	       for (int j = 0; j < 128; ++j) {
		       if (dir_block.entries[j].inum == -1) continue;
		       ++cnt;
		       if (!strcmp(dir_block.entries[j].name, name)) {
			       entry_idx = j;
		       }
	       }

	       if (entry_idx == -1) continue;

	       if (cnt == 1) {
		       reset_bitmap(nfs->data_bp, nfs->inodes[pinum].direct[i]);
		       set_bitmap(nfs->dirty_data_bp, nfs->inodes[pinum].direct[i]);
		       nfs->inodes[pinum].direct[i] = -1;
		       break;
	       }

	       dir_ent_t dentry;
	       dentry.inum = -1;
	       int addr = nfs->inodes[pinum].direct[i] * UFS_BLOCK_SIZE + entry_idx * sizeof(dir_ent_t);
	       if (Write(nfs->disk, addr, &dentry, sizeof(dir_ent_t)) == -1) {
		      printf( "ufs_unlink write fail\n");
		      abort();
	       }
	       break;
       }

       if (commit_dirty_to_disk(nfs) == -1) {
	       printf( "ufs_unlink commit dirty to disk fail\n");
	       abort();
       }

       (*(nfs->disk->fsync))();
       return 0;
}

