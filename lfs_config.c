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
   .block_count = 64,
   .lookahead = 64,
   
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
   return LITTLE_FS_READ_SIZE*8;
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
   char str[] = "hello world";

   char *rp = NULL;

   rp = calloc(0, sizeof(str)+1);

   //little_fs_config.context = calloc(0, 4096 * 64);
   //cfg_t* p = little_fs_config.context;


   little_fs_config.block_count = get_block_count();     //3484
   little_fs_config.lookahead = get_lookahead(); //3488
   int ret = mount_init(little_fs, little_fs_config);
   if (ret != LFS_ERR_OK)
   {
      printf("main lfs_mount error=%d", ret);
      return 0;
   }

   /*int lfs_file_open(lfs_t *lfs, lfs_file_t *file,
      const char *path, int flags) {
   return lfs_file_opencfg(lfs, file, path, flags, NULL);}*/
   /*int lfs_file_opencfg(lfs_t *lfs, lfs_file_t *file,
      const char *path, int flags,
      const struct lfs_file_config *cfg)*/
   lfs_file_open(&little_fs, &file, "rwtest.txt", LFS_O_RDWR | LFS_O_CREAT); //可读可写 不存在则创建   //+错误判断

   //写文件数据
   /*lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file,
      const void *buffer, lfs_size_t size);*/
   lfs_file_write(&little_fs, &file, str, sizeof(str));

   /*
   rp = calloc(0, sizeof(str + 1));
   if (rp == NULL){
      printf("rp calloc error!\n");
   }
   */

   /*lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file,
      void *buffer, lfs_size_t size);*/
      //从"test"文件中读取sizeof(boot_count)数据到boot_count   //+错误判断
   lfs_file_read(&little_fs, &file, &rp, sizeof(str));

   printf("read data = %s\n", rp);

   /*将文件的位置更改为文件的开头

   相当于lfs_file_seek（lfs，file，0，LFS_SEEK_CUR）
   失败时返回负错误代码。*/
   //int lfs_file_rewind(lfs_t *lfs, lfs_file_t *file);
   lfs_file_rewind(&little_fs, &file);   //+错误判断

   //int lfs_file_close(lfs_t *lfs, lfs_file_t *file);
   lfs_file_close(&little_fs, &file);

   //int lfs_unmount(lfs_t *lfs);  //卸载littlefs
   lfs_unmount(&little_fs);

   return 0;
}






int littlfs_read(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
	//printf("littlfs_read(\"%p, %d, %d, %p, %d)\n",
    //        (void*)cfg, block, off, buffer, size);
	char *data = buffer;

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
	//printf("littlfs_program(%p, %d, %d, %p, %d)\n",
    //        (void*)cfg, block, off, buffer, size);
    //cfg_t* emu = cfg->context;
	const uint8_t* data = buffer;

	// Check if write is valid
    assert(off % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);

    FILE *f = fopen("rwtest.txt", "w+b");
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
    char* erase = malloc(cfg->block_size);  //void* calloc（unsigned int num，unsigned int size
    memset(erase, 0xffffffff, cfg->block_size);

    FILE *f = fopen("rwtest.txt", "w");
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

    size_t res = fwrite(erase, 1, block * cfg->block_size, f);
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

    free(erase);

    //printf("lfs_emubd_erase end -> %d", 0);
    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{
    //cfg_t* emu = cfg->context;
	FILE *f = fopen("rwtest.txt", "w");
    if (!f) {
        int err = -errno;
        printf("lfs_emubd_sync fopen -> %d\n", err);
        return err;
    }

    //lfs_emubd_tole32(emu);
    //lfs_emubd_fromle32(emu);
    int err = fclose(f);
    if (err) {
        err = -errno;
        printf("lfs_emubd_sync fclose -> %d\n", err);
        return err;
    }


    //printf("lfs_emubd_sync end -> %d", 0);
    return 0;
}
