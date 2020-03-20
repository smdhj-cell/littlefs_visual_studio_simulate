#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include "lfs.h"

int littlfs_read(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
	printf("littlfs_read(\"%p, %d, %d, %p, %d)\n",
            (void*)cfg, block, off, buffer, size);
	char *data = buffer;

	assert(off % cfg->read_size == 0);  //void assert( int expression );    它的条件返回错误，则终止程序执行
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);

	//strncpy(buffer, f, size);

	if (f) {
		int err = fseek(f, off, SEEK_SET);  //int fseek(FILE *stream, long offset, int fromwhere); 移动指针
            if (err) {
                err = -errno;
                printf("lfs_emubd_read fseek -> %d\n", err);
                fclose(f);
                return err;
            }

		size_t res = fread(data, 1, size, f);   //从f读取大小size的字节数到data
        if (res < size && !feof(f)) {	//int feof(FILE *stream); 如果文件结束，则返回非0值，否则返回0，文件结束符只能被clearerr()清除
            err = -errno;
            printf("lfs_emubd_read fread -> %d\n", err);
            fclose(f);
            return err;
        }

        err = fclose(f);
        if (err) {
            err = -errno;
            printf("lfs_emubd_read fclose -> %d\n", err);
            return err;
        }
    }

    //printf("lfs_emubd_read end -> %d", 0);
	return 0;
}

int littlfs_program(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
	printf("littlfs_program(%p, %d, %d, %p, %d)\n",
            (void*)cfg, block, off, buffer, size);
	const uint8_t* data = buffer;

	// Check if write is valid
    assert(off % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);

    FILE *f = fopen("rwtest.txt", "r+b");
    if (!f) {
        int err = (errno == EACCES) ? 0 : -errno;
        printf("lfs_emubd_prog fopen -> %d\n", err);
        return err;
    }

    // Check that file was erased
    assert(f);

    int err = fseek(f, off, SEEK_SET);
    if (err) {
        err = -errno;
        printf("lfs_emubd_prog fseek0 -> %d\n", err);
        fclose(f);
        return err;
    }

    size_t res = fwrite(data, 1, size, f);
    if (res < size) {
        err = -errno;
        printf("lfs_emubd_prog fwrite -> %d\n", err);
        fclose(f);
        return err;
    }


    err = fseek(f, off, SEEK_SET);
    if (err) {
        err = -errno;
        printf("lfs_emubd_prog fseek1 -> %d\n", err);
        fclose(f);
        return err;
    }

    uint8_t dat;
    res = fread(&dat, 1, 1, f);
    if (res < 1) {
        err = -errno;
        printf("lfs_emubd_prog fread -> %d\n", err);
        fclose(f);
        return err;
    }
    printf("dat = %hd\n", dat);

    err = fclose(f);
    if (err) {
        err = -errno;
        printf("lfs_emubd_prog fclose -> %d\n", err);
        return err;
    }

    //printf("lfs_emubd_prog end -> %d", 0);
    return 0;
}

int littlfs_erase(const struct lfs_config *cfg, lfs_block_t block)
{
	// Check if erase is valid
    // assert(block < cfg->block_count);

    FILE *f = fopen("rwtest.txt", "w");
    if (!f) {
        int err = (errno == EACCES) ? 0 : -errno;
        printf("littlfs_erase fopen littlfs_erase -> %d\n", err);
        return err;
    }

    // Check that file was erased
    assert(f);

    memset(f, 0, block);

    int err = fclose(f);
    if (err) {
        err = -errno;
        printf("lfs_emubd_erase fclose -> %d\n", err);
        return err;
    }

    //printf("lfs_emubd_erase end -> %d", 0);
    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{
	FILE *f = fopen("rwtest.txt", "w");
    if (!f) {
        int err = -errno;
        printf("lfs_emubd_sync fopen -> %d\n", err);
        return err;
    }

    int err = fclose(f);
    if (err) {
        err = -errno;
        printf("lfs_emubd_sync fclose -> %d\n", err);
        return err;
    }

    //printf("lfs_emubd_sync end -> %d", 0);
    return 0;
}