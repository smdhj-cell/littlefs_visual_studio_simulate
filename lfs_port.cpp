#include "lfs_port.h"
#include "little_fs_config.h"
#include <memory>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <cassert>
using namespace std;
extern "C"
{
#include "little_fs.h"
}
using namespace gsl;
using namespace std;
using namespace std::filesystem;

constexpr auto font_size = 2 * 1024 * 1024;
constexpr auto ota_size = 400 * 1024;

int little_fs_read(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, void* buffer, lfs_size_t size)
{
	byte* memory_bin = static_cast<byte*>(c->context);
	memcpy(buffer, reinterpret_cast<void*>(memory_bin + block * c->block_size + off), size);
	return LFS_ERR_OK;
}

int little_fs_program(const struct lfs_config* c, lfs_block_t block, lfs_off_t off, const void* buffer, lfs_size_t size)
{
	byte* memory_bin = static_cast<byte*>(c->context);
	memcpy(reinterpret_cast<void*>(memory_bin + block * c->block_size + off), buffer, size);
	return LFS_ERR_OK;
}
int little_fs_erase(const struct lfs_config* c, lfs_block_t block)
{
	byte* memory_bin = static_cast<byte*>(c->context);
	memset(reinterpret_cast<void*>(memory_bin + block * c->block_size), 0xff, LITTLE_FS_BLOCK_SIZE);
	return LFS_ERR_OK;
}
int little_fs_sync(const struct lfs_config* c)
{
	return LFS_ERR_OK;
}

void little_fs_bin_create(Flash_base& flash)
{
	const auto flash_size = flash.block_count() * LITTLE_FS_BLOCK_SIZE + font_size + ota_size;
	auto memory_bin = new byte[flash_size];
	auto const lookahead_buff_size = (32 * ((flash.block_count() + 31) / 32)) / 8;
	auto const lookahead_buffer = new byte[lookahead_buff_size];
	auto _ = finally([&]()
	{
		delete[] memory_bin;
		delete[] lookahead_buffer;
	});
	memset(memory_bin, 0xff, flash_size);
	memset(lookahead_buffer, 0, lookahead_buff_size);
	lfs_t little_fs = { 0 };
	uint8_t read_buffer[LITTLE_FS_READ_SIZE] = {0};
	uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE] ={0};
	
	struct lfs_config little_fs_config = {
		.context = memory_bin,
		.read = little_fs_read,
		.prog = little_fs_program,
		.erase = little_fs_erase,
		.sync = little_fs_sync,
		.read_size = LITTLE_FS_READ_SIZE,
		.prog_size = LITTLE_FS_PROGRAM_SIZE,
		.block_size = LITTLE_FS_BLOCK_SIZE,
		.block_count = flash.block_count(),
		.lookahead = 32 * ((flash.block_count() + 31) / 32),
		.read_buffer = read_buffer,
		.prog_buffer = program_buffer,
		.lookahead_buffer = lookahead_buffer,		
	};
	auto mount_ret = lfs_mount(&little_fs, &little_fs_config);
	if (mount_ret != LFS_ERR_OK)
	{		
		const auto format_ret = lfs_format(&little_fs, &little_fs_config);
		if (format_ret != LFS_ERR_OK)
		{
			printf("lfs_format error=%d", format_ret);
			cout << "lfs_format error %d" << format_ret << endl;
			return;
		}
		mount_ret = lfs_mount(&little_fs, &little_fs_config);
		if (mount_ret != LFS_ERR_OK)
		{
			cout << "lfs_format mount error %d" << mount_ret << endl;
			return;
		}
	}
	cout << "little_fs_start"<<endl;
	const string image_path = "image";
	lfs_file_t file{ };
	char buff[1024] = { 0 };
	int buff_len = 0;
	function<void(string pc_path, string little_fs_path)> directory_traverse;
	directory_traverse = [&](string pc_path,string little_fs_path)
	{
		for (const auto& entry : directory_iterator(pc_path))
		{
			if (entry.is_directory())
			{
				auto file_name = entry.path().filename();
				auto path_name = little_fs_path;
				path_name.append(file_name.string());
				int const make_dir_result = lfs_mkdir(&little_fs, path_name.c_str());
				if (make_dir_result != LFS_ERR_OK && make_dir_result != LFS_ERR_EXIST)
				{
					//LITTLE_FS_LOG("create dir %s fail %d", path_buff, make_dir_result);
					cout << "create dir " << path_name << "fail " << make_dir_result << endl;
					return;
				}
				path_name.append("/");
				directory_traverse(entry.path().string(), path_name);
			}
			else
			{
				auto file_name = little_fs_path;				
				auto base_name = entry.path().filename();
				file_name.append(base_name.string());
				if (ifstream image{ entry.path(), ios::binary })
				{
					const auto open_result = lfs_file_open(&little_fs, &file, file_name.c_str(), LFS_O_CREAT | LFS_O_WRONLY | LFS_O_TRUNC);
					assert(open_result == LFS_ERR_OK);
					auto file_size = 0;
					do
					{						
						image.read(buff, sizeof(buff));
						buff_len = image.gcount();
						if (buff_len == 0)
						{
							break;
						}
						file_size += buff_len;
						const auto write_result = lfs_file_write(&little_fs, &file, buff, buff_len);
						assert(write_result == buff_len);
					} while (buff_len != 0);
					const auto close_result = lfs_file_close(&little_fs, &file);
					assert(close_result == LFS_ERR_OK);
					cout << file_name << "\t" << file_size << std::endl;
				}
			}
		}
	};
	directory_traverse(image_path,"");
	ifstream font_bin("font24X24.bin", ios::binary);
	font_bin.read(reinterpret_cast<char*>(memory_bin) + flash_size - font_size, font_size);
	ofstream file_bin(flash.bin_name(), ios::binary | ios::trunc);
	file_bin.write(reinterpret_cast<char*>(memory_bin), flash_size);

}