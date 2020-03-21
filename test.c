#include <string.h>
#include <assert.h>
#include "lfs_config.h"

lfs_file_t file;

lfs_t little_fs = {0};

static uint8_t read_buffer[LITTLE_FS_READ_SIZE];
static uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE];
static uint8_t lookahead_buffer[LITTLE_FS_MAX_LOOKAHEAD / 8];

static struct lfs_config little_fs_config = {
   .read = littlfs_read,
   .prog = littlfs_program,
   .erase = littlfs_erase,
   .sync = littlfs_sync,
   
   .read_size = LITTLE_FS_READ_SIZE,   //读
   .prog_size = LITTLE_FS_PROGRAM_SIZE,   //写
   .block_size = LITTLE_FS_BLOCK_SIZE, //块大小
   .block_count = 3,
   .lookahead = 128,
   
   .read_buffer = read_buffer,
   .prog_buffer = program_buffer,
   .lookahead_buffer = lookahead_buffer,
};

uint32_t get_block_count(void)
{
	//return LITTLE_FS_READ_SIZE*8;
	return 3488;
}

uint32_t get_lookahead(void)
{
   uint32_t block_count = get_block_count();
   return block_count;
}

int main(int argc, char const *argv[])
{
  char str[256] = "hello world";   //最小操作读写快为#define LITTLE_FS_READ_SIZE (128) #define LITTLE_FS_PROGRAM_SIZE (128)

  char *rp = NULL;

  rp = calloc(0, LITTLE_FS_BLOCK_SIZE+1);

	char* p = malloc(little_fs_config.block_count * little_fs_config.block_size + 1);
	little_fs_config.context = malloc(little_fs_config.block_count * little_fs_config.block_size + 1);
  char* pd = little_fs_config.context;

  memset(p, 255, little_fs_config.block_count * little_fs_config.block_size);
  
  littlfs_program(&little_fs_config, 0, 0, str, sizeof(str));

  //littlfs_read(&little_fs_config, 0, 0, p, sizeof(str));

  //printf("rp = %s\n", p);
  
  //littlfs_erase(&little_fs_config, 0);
  //printf("little_fs_config.context = %s\n", pd);

  //printf("read data = %s\n", rp);

  //lfs_file_write(&little_fs, &file, str, sizeof(str));

  //int mount_ret = lfs_mount(&little_fs, &little_fs_config);   //挂载littlefs

  printf("little_fs_config.context = %s\n", pd);

  lfs_file_rewind(&little_fs, &file);

  lfs_file_read(&little_fs, &file, p, sizeof(str));

  printf("read data = %s\n", p);

	

	//lfs_file_open(&little_fs, &file, "tt.txt", LFS_O_RDWR | LFS_O_CREAT); //可读可写 不存在则创建   //+错误判断

  

  

  //lfs_file_close(&little_fs, &file);

  //lfs_unmount(&little_fs);

  free(p);
	free(little_fs_config.context);

	return 0;
}


int littlfs_read(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
  printf("littlfs_read(%p, %d, %d, %p, %d)\n",
            (void*)cfg, block, off, buffer, size);
  memset(buffer, 0, size);

  assert(off % cfg->read_size == 0);  //void assert( int expression );    它的条件返回错误，则终止程序执行
  assert(size % cfg->read_size == 0);
  assert(block < cfg->block_count);

  strncpy( ((char *)buffer) + (block * cfg->block_size + off), ((char *)cfg->context) + (block * cfg->block_size + off), size);
    
    //printf("lfs_emubd_read end -> %d", 0);
  return 0;
}

int littlfs_program(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
	printf("littlfs_program(%p, %d, %d, %p, %d)\n",
            (void*)cfg, block, off, buffer, size);

	// Check if write is valid
    assert(off % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);

    strncpy(((char *)cfg->context) + (block * cfg->block_size + off), ((char *)buffer) + (block * cfg->block_size + off), size);

    //printf("lfs_emubd_prog end -> %d", 0);
    return 0;
}

int littlfs_erase(const struct lfs_config *cfg, lfs_block_t block)
{
	// Check if erase is valid
    // assert(block < cfg->block_count);
    //注意：num：对象个数，size：对象占据的内存字节数，相较于malloc函数，calloc函数会自动将内存初始化为0；
    //char* perase = malloc(cfg->block_size + 1);  //void* calloc（unsigned int num，unsigned int size
    memset(cfg->context + block * cfg->block_size, 255, cfg->block_size);

    printf("lfs_emubd_erase end -> %d\n", 0);
    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{
    printf("littlfs_sync test -> %d\n", 0);
    return 0;
}
