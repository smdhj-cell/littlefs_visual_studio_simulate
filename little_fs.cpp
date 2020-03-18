#include <stddef.h>
#include "little_fs.h"
#include "little_fs_config.h"
#include <string.h>
#include  "stdio.h"
#include "lfs.h"
#define NRF_LOG_INFO(fmt, ...) printf(fmt "\r\n", ##__VA_ARGS__)

#define LITTLE_FS_MAX_LOOKAHEAD (32 * ((LITTLE_FS_MAX_BLOCK_COUNT + 31) / 32))

lfs_t little_fs = { 0 };
static uint8_t read_buffer[LITTLE_FS_READ_SIZE];
static uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE];
static uint8_t lookahead_buffer[LITTLE_FS_MAX_LOOKAHEAD / 8];


static int little_fs_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
static int little_fs_program(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
static int little_fs_erase(const struct lfs_config* c, lfs_block_t block);
static int little_fs_sync(const struct lfs_config* c);
static bool is_init = false;
static struct lfs_config little_fs_config = {
	.read = little_fs_read,
	.prog = little_fs_program,
	.erase = little_fs_erase,
	.sync = little_fs_sync,
	.read_size = LITTLE_FS_READ_SIZE,
	.prog_size = LITTLE_FS_PROGRAM_SIZE,
	.block_size = LITTLE_FS_BLOCK_SIZE,
	.block_count = 0,
	.lookahead = 0,
	.read_buffer = read_buffer,
	.prog_buffer = program_buffer,
	.lookahead_buffer = lookahead_buffer,
};
uint32_t get_block_count(void)
{
	return 3484;
}
uint32_t get_lookahead(void)
{
	const uint32_t block_count = get_block_count();
	return (32 * ((block_count + 31) / 32));
}
static int little_fs_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size)
{
	// NRF_LOG_INFO("read %u from 0x%0x to 0x%x", size, block*c->block_size + off,buffer);
	//sFLASH_NOR_Read(buffer, block * c->block_size + off, size);
	// NRF_LOG_INFO("end read");
	return LFS_ERR_OK;
}

static int little_fs_program(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{

	uint32_t temp32 = 0;
	while (size)
	{
		uint16_t write_cnt = size > 256 ? 256 : size;
		//sFLASH_NOR_Pageprogram((uint8_t *)buffer + temp32, block * c->block_size + off + temp32, write_cnt);
		temp32 += write_cnt;
		size -= write_cnt;
	}
	return LFS_ERR_OK;
}

static int little_fs_erase(const struct lfs_config* c, lfs_block_t block)
{
	// NRF_LOG_INFO("erase %u", block * c->block_size);
	//sFLASH_NOR_SectorErase(block * c->block_size);
	return LFS_ERR_OK;
}
static int little_fs_sync(const struct lfs_config* c)
{
	return LFS_ERR_OK;
}

/**
 * @brief
 *
 * @return true
 * @return false
 */
void little_fs_init(void)
{
	//! 从qf移植的memory_pool，比较简单，分配内存的时候没有循环        
	little_fs_config.block_count = get_block_count();
	little_fs_config.lookahead = get_lookahead();
	int mount_ret = lfs_mount(&little_fs, &little_fs_config);
	if (mount_ret != LFS_ERR_OK)
	{
		// sFLASH_NOR_ChipErase();
		const int format_ret = lfs_format(&little_fs, &little_fs_config);
		if (format_ret != LFS_ERR_OK)
		{
			NRF_LOG_INFO("lfs_format error=%d", format_ret);
			return ;
		}
		mount_ret = lfs_mount(&little_fs, &little_fs_config);
		if (mount_ret != LFS_ERR_OK)
		{
			NRF_LOG_INFO("lfs_mount 2 error=%d", mount_ret);
			return ;
		}
	}
}
