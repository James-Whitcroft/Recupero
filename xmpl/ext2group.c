/*
 * ext2group.c
 *
 * Reads the first (and only) group-descriptor from a Ext2 floppy disk.
 *
 * Questions?
 * Emanuele Altieri
 * ealtieri@hampshire.edu
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ext2_fs.h"

#define BASE_OFFSET 1024                  /* locates beginning of the super block (first group) */
#define FD_DEVICE "/dev/sdb"               /* the floppy disk device */
#define TO_BYTE(block) (BASE_OFFSET + (block - 1)*block_size)
#define TO_BLOCK(byte) (((byte - BASE_OFFSET) / block_size) + 1)


static unsigned int block_size = 0;        /* block size (to be calculated) */

int main(void)
{
	struct ext2_super_block super;
	struct ext2_group_desc group;
	int fd;

	/* open floppy device */

	if ((fd = open(FD_DEVICE, O_RDONLY)) < 0) {
		perror(FD_DEVICE);
		exit(1);  /* error while opening the floppy device */
	}

	/* read super-block */

	lseek(fd, BASE_OFFSET, SEEK_SET); 
	read(fd, &super, sizeof(super));

	if (super.s_magic != EXT2_SUPER_MAGIC) {
		fprintf(stderr, "Not a Ext2 filesystem\n");
		exit(1);
	}
		
	block_size = 1024 << super.s_log_block_size;
    printf("block size: %u\n",block_size);
    printf("SEEK_SET: %u\n", SEEK_SET);	
    /* read group descriptor */

	//lseek(fd, BASE_OFFSET + block_size, SEEK_SET);
    lseek(fd, 134218752, SEEK_SET);
	read(fd, &group, sizeof(group));
	close(fd);
    printf("group size: %lu\n", sizeof(group));
	printf("Reading first group-descriptor from device " FD_DEVICE ":\n"
	       "Blocks bitmap block: %u\n"
	       "Inodes bitmap block: %u\n"
	       "Inodes table block : %u\n"
	       "Free blocks count  : %u\n"
	       "Free inodes count  : %u\n"
	       "Directories count  : %u\n"
	       ,
	       TO_BLOCK(group.bg_block_bitmap),
	       group.bg_inode_bitmap,
	       group.bg_inode_table,
	       group.bg_free_blocks_count,
	       group.bg_free_inodes_count,
	       group.bg_used_dirs_count);    /* directories count */
	
	exit(0);
} /* main() */
