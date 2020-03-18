#pragma once
#include "lfs.h"
#include <gsl/gsl>
#include <string>
#ifdef __cplusplus
extern "C"
{
#endif
	int little_fs_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size);
	int little_fs_program(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size);
	int little_fs_erase(const struct lfs_config* c, lfs_block_t block);
	int little_fs_sync(const struct lfs_config* c);
#ifdef __cplusplus
}
enum class FlashType
{
	byte16_m,
	byte8_m,
};
class Flash_base
{
public:
	virtual ~Flash_base() = default;
	virtual  uint32_t block_count() = 0;
	virtual gsl::czstring<> bin_name() = 0;
};

class Flash_8m final :public Flash_base
{
public:	
	uint32_t block_count() final
	{
		return 1436;
	}
	gsl::czstring<> bin_name() final
	{
		return "8m.bin";
	}
};
class Flash_16m final :public Flash_base
{
public:
	uint32_t block_count() final
	{
		return 3484;
	}
	gsl::czstring<> bin_name() final
	{
		return "16m.bin";
	}
};

void little_fs_bin_create(Flash_base& );
#endif