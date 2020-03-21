#ifndef __LFS_CONFIG_H__
#define __LFS_CONFIG_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include "lfs.h"
#include "lfs_util.h"

typedef struct cfg{
    uint32_t read_size;
    uint32_t prog_size;
    uint32_t block_size;
    uint32_t block_count;
} cfg_t;

#define LITTLE_FS_MAX_LOOKAHEAD (128)
//#define LITTLE_FS_MAX_LOOKAHEAD (32 * ((LITTLE_FS_MAX_BLOCK_COUNT + 31) / 32))

//#define LITTLE_FS_MAX_BLOCK_COUNT (3488)

/**
 * @brief 与flash通讯每次至少读取多少字节
 *        此值过大，需要大量内存
 *        此值过小，每次读取都会与flash进行通讯
 *
 */
#define LITTLE_FS_READ_SIZE (128)

/**
 * @brief 与flash通讯每次至少写入多少字节
 *        必须是LITTLE_FS_READ_SIZE的倍数
 *
 */
#define LITTLE_FS_PROGRAM_SIZE (128)

/**
 * @brief flash每次至少擦除多少字节
 *        必须是LITTLE_FS_PROGRAM_SIZE的倍数
 *
 */
#define LITTLE_FS_BLOCK_SIZE (4096) // 2^12

int littlfs_read(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size);

int littlfs_program(const struct lfs_config *c, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size);

int littlfs_erase(const struct lfs_config *c, lfs_block_t block);

int littlfs_sync(const struct lfs_config *c);


#endif
