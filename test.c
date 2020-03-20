#include "lfs_config.h"

#define LITTLE_FS_MAX_LOOKAHEAD (32 * ((LITTLE_FS_MAX_BLOCK_COUNT + 31) / 32))

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
   .block_count = 0,
   .lookahead = 0,
   
   .read_buffer = read_buffer,
   .prog_buffer = program_buffer,
   .lookahead_buffer = lookahead_buffer,
};

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
   // Create directory if it doesn't exist
   int err = mkdir("rwtest", 0777);
   if (err && errno != EEXIST) {
      err = -errno;
      printf("lfs_emubd_create mkdir -> %d", err);
      return err;
   }

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

   uint64_t data = 0x0123456789abcef;
   uint64_t rdata = 0;
   char *rp = NULL;

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
   lfs_file_write(&little_fs, &file, &data, sizeof(data));

   /*
   rp = calloc(0, sizeof(str + 1));
   if (rp == NULL){
      printf("rp calloc error!\n");
   }
   */

   /*lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file,
      void *buffer, lfs_size_t size);*/
      //从"test"文件中读取sizeof(boot_count)数据到boot_count   //+错误判断
   lfs_file_read(&little_fs, &file, &rdata, sizeof(rdata));

   printf("read data = %ld\n", rdata);

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

