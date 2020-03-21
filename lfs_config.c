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
   .block_count = 2,
   .lookahead = 128,
   
   .read_buffer = read_buffer,
   .prog_buffer = program_buffer,
   .lookahead_buffer = lookahead_buffer,
};


static inline void lfs_emubd_tole32(cfg_t *emu) {
   emu->read_size     = lfs_tole32(emu->read_size);
   emu->prog_size     = lfs_tole32(emu->prog_size);
   emu->block_size    = lfs_tole32(emu->block_size);
   emu->block_count   = lfs_tole32(emu->block_count);
}

static inline void lfs_emubd_fromle32(cfg_t *emu) {
   emu->read_size     = lfs_fromle32(emu->read_size);
   emu->prog_size     = lfs_fromle32(emu->prog_size);
   emu->block_size    = lfs_fromle32(emu->block_size);
   emu->block_count   = lfs_fromle32(emu->block_count);
}

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

int mount_init(lfs_t little_fs, struct lfs_config little_fs_config)
{
   int mount_ret = lfs_mount(&little_fs, &little_fs_config);   //挂载littlefs
   if (mount_ret != LFS_ERR_OK)   //挂载失败
   {
      // sFLASH_NOR_ChipErase();
      const int format_ret = lfs_format(&little_fs, &little_fs_config); //使用littlefs格式化块设备
      if (format_ret != LFS_ERR_OK) //littlefs格式化块设备失败
      {
          printf("lfs_format error=%d", format_ret);
      }
      mount_ret = lfs_mount(&little_fs, &little_fs_config);
      if (mount_ret != LFS_ERR_OK)
      {
          printf("lfs_mount 2 error=%d", mount_ret);
      }
   }

   return mount_ret;
}

int main(int argc, char const *argv[])
{
   char str[256] = "hello world";   //最小操作读写快为#define LITTLE_FS_READ_SIZE (128) #define LITTLE_FS_PROGRAM_SIZE (128)

   char *rp = NULL;

   rp = calloc(0, LITTLE_FS_BLOCK_SIZE+1);

   //little_fs_config.context = calloc(0, 4096 * 64);
   //cfg_t* p = little_fs_config.context;

   //system("touch rwtest.txt");

   littlfs_program(&little_fs_config, 0, 0, str, sizeof(str));

   littlfs_read(&little_fs_config, 0, 0, rp, sizeof(str));

   //littlfs_erase(&little_fs_config, 0);

   printf("read data = %s\n", rp);

   little_fs_config.block_count = get_block_count();     //3484
   little_fs_config.lookahead = get_lookahead(); //3488
   int ret = mount_init(little_fs, little_fs_config);
   if (ret != LFS_ERR_OK)
   {
      printf("main lfs_mount error=%d", ret);
      return 0;
   }
   
   //lfs_file_open(&little_fs, &file, "rwtest.txt", LFS_O_RDWR | LFS_O_CREAT); //可读可写 不存在则创建   //+错误判断
   
   //lfs_file_write(&little_fs, &file, str, sizeof(str));


   //lfs_file_rewind(&little_fs, &file);
   
   lfs_file_read(&little_fs, &file, &rp, sizeof(str));

   printf("read data = %s\n", rp);
   
   //lfs_file_close(&little_fs, &file);

  // //int lfs_unmount(lfs_t *lfs);  //卸载littlefs
   
   //lfs_unmount(&little_fs);
   printf("!!!\n");
   return 0;
}






int littlfs_read(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
	printf("littlfs_read(%p, %d, %d, %p, %d)\n",
            (void*)cfg, block, off, buffer, size);
	char *data = buffer;
	memset(data, 0, size);

	assert(off % cfg->read_size == 0);  //void assert( int expression );    它的条件返回错误，则终止程序执行
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);

	//strncpy(buffer, f, size);
    
    FILE *f = fopen("rwtest.txt", "rb");
	if (f) {
		int err = fseek(f, block * cfg->block_size + off, SEEK_SET);  //int fseek(FILE *stream, long offset, int fromwhere); 移动指针
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
        printf("res = %ld\n", res);

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
    //cfg_t* emu = cfg->context;
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

    int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
    if (err) {
        err = -errno;
        printf("lfs_emubd_prog fseek0 -> %d\n", err);
        fclose(f);
        return err;
    }

    //lfs_emubd_tole32(emu);
    size_t res = fwrite(data, 1, size, f);
    if (res < size) {
        err = -errno;
        printf("lfs_emubd_prog fwrite -> %d\n", err);
        fclose(f);
        return err;
    }
    //lfs_emubd_fromle32(emu);


    err = fseek(f, block * cfg->block_size + off, SEEK_SET);
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
    //注意：num：对象个数，size：对象占据的内存字节数，相较于malloc函数，calloc函数会自动将内存初始化为0；
    char* perase = malloc(cfg->block_size + 1);  //void* calloc（unsigned int num，unsigned int size
    memset(perase, 0, cfg->block_size);

    FILE *f = fopen("rwtest.txt", "r+b");
    if (!f) {
        int err = (errno == EACCES) ? 0 : -errno;
        printf("littlfs_erase fopen -> %d\n", err);
        return err;
    }

    // Check that file was erased
    assert(f);

    int err = fseek(f, block * cfg->block_size, SEEK_SET);
    if (err) {
        err = -errno;
        printf("littlfs_erase fseek -> %d\n", err);
        fclose(f);
        return err;
    }

    size_t res = fwrite(perase, cfg->block_size, 1, f);
    if (res < 1) {
        err = -errno;
        printf("littlfs_erase fwrite -> %d\n", err);
        fclose(f);
        return err;
    }


    err = fclose(f);
    if (err) {
        err = -errno;
        printf("lfs_emubd_erase fclose -> %d\n", err);
        return err;
    }


    free(perase);

    //printf("lfs_emubd_erase end -> %d", 0);
    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{

    //printf("lfs_emubd_sync end -> %d", 0);
    return 0;
}
