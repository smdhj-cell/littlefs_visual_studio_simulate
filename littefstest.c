#include "lfs.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// variables used by the filesystem

lfs_t lfs;

lfs_file_t file;

int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);

int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size);

int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block);

int user_provided_block_device_sync(const struct lfs_config *c);


// configuration of the filesystem is provided by this struct

const struct lfs_config cfg = {

    // block device operations

    .read = user_provided_block_device_read,

    .prog = user_provided_block_device_prog,

    .erase = user_provided_block_device_erase,

    .sync = user_provided_block_device_sync,



    // block device configuration

    .read_size = 256,

    .prog_size = 256,

    .block_size = 4096,

    .block_count = 512,

    .lookahead = 256,

};


int user_provided_block_device_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
	//printf("user_provided_block_device_read test!\n");
	//void *p = c->context;
	//strncpy(buffer, ((char *)p + block * c->block_size), size);
	return 0;
}

int user_provided_block_device_prog(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
	//printf("user_provided_block_device_prog test!\n");
	//void *p = c->context;
	//strncpy(((char *)p + block * c->block_size), buffer, size);
	return 0;
}

int user_provided_block_device_erase(const struct lfs_config *c, lfs_block_t block)
{
	//printf("user_provided_block_device_erase test!\n");
	//void *p = c->context;
	//memset(((char *)p + block * c->block_size), 0, 4096);
	return 0;
}

int user_provided_block_device_sync(const struct lfs_config *c)
{
	printf("user_provided_block_device_sync test\n");
	return 0;
}



// entry point

int main(void) {

    // mount the filesystem
    lfs_format(&lfs, &cfg);
    int err = lfs_mount(&lfs, &cfg);


    // reformat if we can't mount the filesystem

    // this should only happen on the first boot

    if (err) {

        lfs_format(&lfs, &cfg);

        lfs_mount(&lfs, &cfg);

    }



    // read current count

    uint32_t boot_count = 0;

    lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);

    lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));



    // update boot count

    boot_count += 1;

    lfs_file_rewind(&lfs, &file);

    lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));



    // remember the storage is not updated until the file is closed successfully

    lfs_file_close(&lfs, &file);



    // release any resources we were using

    lfs_unmount(&lfs);



    // print the boot count

    printf("boot_count: %d\n", boot_count);

}
