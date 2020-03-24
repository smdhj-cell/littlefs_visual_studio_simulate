#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "lfs_config.h"

lfs_file_t file;

lfs_t little_fs = {0};

static uint8_t read_buffer[LITTLE_FS_READ_SIZE];
static uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE];
static uint8_t lookahead_buffer[LITTLE_FS_MAX_LOOKAHEAD / 8];	//16

static struct lfs_config little_fs_config = {
   .read = littlfs_read,
   .prog = littlfs_program,
   .erase = littlfs_erase,
   .sync = littlfs_sync,
   
   .read_size = LITTLE_FS_READ_SIZE,   //读
   .prog_size = LITTLE_FS_PROGRAM_SIZE,   //写
   .block_size = LITTLE_FS_BLOCK_SIZE, //块大小	2^12 = 4096
   .block_count = 64,	// 4096 * 2^6 = 2^18
   .lookahead = 256,
   
   .read_buffer = read_buffer,
   .prog_buffer = program_buffer,
   .lookahead_buffer = lookahead_buffer,
};

int mount_init(lfs_t* little_fs, struct lfs_config* little_fs_config)
{
   int mount_ret = lfs_mount(little_fs, little_fs_config);   //挂载littlefs
   if (mount_ret != LFS_ERR_OK)   //挂载失败
   {
      // sFLASH_NOR_ChipErase();
      const int format_ret = lfs_format(little_fs, little_fs_config); //使用littlefs格式化块设备
      if (format_ret != LFS_ERR_OK) //littlefs格式化块设备失败
      {
          printf("lfs_format error=%d", format_ret);
      }
      mount_ret = lfs_mount(little_fs, little_fs_config);
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
	char str1[256] = "test!"; 

	char *rp = NULL;

	rp = calloc(0, LITTLE_FS_BLOCK_SIZE+1);

	int ret = mount_init(&little_fs, &little_fs_config);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_mount error=%d\n", ret);
	  return 0;
	}

	/*int lfs_file_open(lfs_t *lfs, lfs_file_t *file,
		const char *path, int flags) {
	return lfs_file_opencfg(lfs, file, path, flags, NULL);}*/
	/*int lfs_file_opencfg(lfs_t *lfs, lfs_file_t *file,
		const char *path, int flags,
		const struct lfs_file_config *cfg)*/
	ret = lfs_file_open(&little_fs, &file, "rwtest.txt", LFS_O_RDWR | LFS_O_CREAT); //可读可写 不存在则创建   //+错误判断
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_open error=%d\n", ret);
	  return 0;
	}

	//int lfs_file_rewind(lfs_t *lfs, lfs_file_t *file);
	ret = lfs_file_rewind(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_rewind0 error=%d\n", ret);
	  return 0;
	}


	//写文件数据
	//lfs_ssize_t lfs_file_write(lfs_t *lfs, lfs_file_t *file,
	//   const void *buffer, lfs_size_t size);
	ret = lfs_file_write(&little_fs, &file, str, sizeof(str));
	if (ret < sizeof(str))
	{
	  printf("lfs_file_write0 error=%d\n", ret);
	  return 0;
	}

	ret = lfs_file_rewind(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_rewind1 error=%d\n", ret);
	  return 0;
	}

	/*lfs_ssize_t lfs_file_read(lfs_t *lfs, lfs_file_t *file,
		void *buffer, lfs_size_t size);*/
	//从"test"文件中读取sizeof(boot_count)数据到boot_count   //+错误判断
	ret = lfs_file_read(&little_fs, &file, rp, sizeof(str));
	if (ret < sizeof(str))
	{
	  printf("lfs_file_read0 error=%d\n", ret);
	  return 0;
	}

	printf("Frist read data = %s\n", rp);

	lfs_soff_t index = lfs_file_tell(&little_fs, &file);
	if (index < 0){
		printf("lfs_file_tell0 error=%d\n", ret);
	  	return 0;
	}

	printf("current0 = %d\n", index);

	ret = lfs_file_rewind(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
		printf("lfs_file_rewind2 error=%d\n", ret);
		return 0;
	}

	index =  lfs_file_tell(&little_fs, &file);
	if (index < 0){
		printf("lfs_file_tell1 error=%d\n", ret);
	  	return 0;
	}

	printf("current1 = %d\n", index);

	index = lfs_file_seek(&little_fs, &file, little_fs_config.block_size, 0);
	if (index < 0){
		printf("lfs_file_seek0 error=%d\n", ret);
	  	return 0;
	}

	index =  lfs_file_tell(&little_fs, &file);
	if (index < 0){
		printf("lfs_file_tell2 error=%d\n", ret);
	  	return 0;
	}

	printf("current2 = %d\n", index);

	//Second write read

	ret = lfs_file_write(&little_fs, &file, str1, sizeof(str1));
	if (ret < sizeof(str1))
	{
	  printf("lfs_file_write1 error=%d\n", ret);
	  return 0;
	}

	ret = lfs_file_rewind(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_rewind3 error=%d\n", ret);
	  return 0;
	}

	index = lfs_file_seek(&little_fs, &file, little_fs_config.block_size, 0);
	if (index < 0){
		printf("lfs_file_seek1 error=%d\n", ret);
	  	return 0;
	}

	ret = lfs_file_read(&little_fs, &file, rp, sizeof(str1));
	if (ret < sizeof(str1))
	{
	  printf("lfs_file_read1 error=%d\n", ret);
	  return 0;
	}

	index =  lfs_file_tell(&little_fs, &file);
	if (index < 0){
		printf("lfs_file_tell3 error=%d\n", ret);
	  	return 0;
	}

	printf("current3 = %d\n", index);

	printf("Second read data = %s\n", rp);

	//Third
	ret = lfs_file_rewind(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_rewind4 error=%d\n", ret);
	  return 0;
	}

	ret = lfs_file_read(&little_fs, &file, rp, sizeof(str));
	if (ret < sizeof(str1))
	{
	  printf("lfs_file_read2 error=%d\n", ret);
	  return 0;
	}

	printf("Third read data = %s\n", rp);

	ret = lfs_file_sync(&little_fs, &file);
	if (ret < 0)
	{
		printf("lfs_file_sync error=%d\n", ret);
		return 0;
	}
	
	printf("!!!!!\n");
	//int lfs_file_close(lfs_t *lfs, lfs_file_t *file);
	ret = lfs_file_close(&little_fs, &file);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_file_close error=%d\n", ret);
	  return 0;
	}

	//int lfs_unmount(lfs_t *lfs);  //卸载littlefs
	ret = lfs_unmount(&little_fs);
	if (ret != LFS_ERR_OK)
	{
	  printf("lfs_unmount error=%d\n", ret);
	  return 0;
	}

	printf("!!!!!\n");

	return 0;
}



int littlfs_read(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, void *buffer, lfs_size_t size)
{
	//printf("littlfs_read(%p, %d, %d, %p, %d)\n",
    //        (void*)cfg, block, off, buffer, size);
	char *data = buffer;

	assert(off % cfg->read_size == 0);  //void assert( int expression );    它的条件返回错误，则终止程序执行
	assert(size % cfg->read_size == 0);
	assert(block < cfg->block_count);

	FILE *f = fopen("rwtest.txt", "r");
	if (!f) {
    	f = fopen("rwtest.txt", "w+");
    	if (!f) {
	        printf("lfs_read fopen create -> -1\n");
	        return -1;
    	}

    	int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
	    if (err) {
	        printf("lfs_read fseek0 -> %d\n", err);
	        fclose(f);
	        return err;
	    }

	    unsigned int count = 0;
	    for(count = 0; count < cfg->block_count; ++count)
	    	littlfs_erase(cfg, count);
    }

    int err = fseek(f, block * cfg->block_size + off, SEEK_SET);  //int fseek(FILE *stream, long offset, int fromwhere); 移动指针
    if (err) {
        printf("lfs_read fseek1 -> %d\n", err);
        fclose(f);
        return err;
    }


	size_t res = fread(data, 1, size, f);   //从f读取大小size的字节数到data
    if (res < size && !feof(f)) {	//int feof(FILE *stream); 如果文件结束，则返回非0值，否则返回0，文件结束符只能被clearerr()清除
        printf("lfs_read fread -> %ld\n", res);
        fclose(f);
        return res;
    }

    err = fclose(f);
    if (err) {
        printf("lfs_read fclose -> %d\n", err);
        return err;
    }

    printf("lfs_read end!\n");
	return 0;
}

int littlfs_program(const struct lfs_config *cfg, lfs_block_t block,
            lfs_off_t off, const void *buffer, lfs_size_t size)
{
	//printf("littlfs_program(%p, %d, %d, %p, %d)\n",
    //        (void*)cfg, block, off, buffer, size);
	uint8_t* data = (char *)buffer;

	// Check if write is valid
    assert(off % cfg->prog_size == 0);
    assert(size % cfg->prog_size == 0);
    assert(block < cfg->block_count);

    FILE *f = fopen("rwtest.txt", "r+");
    if (!f) {
        f = fopen("rwtest.txt", "w+");
    	if (!f) {
	        printf("lfs_prog fopen create -> -1\n");
	        return -1;
    	}

    	int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
	    if (err) {
	        printf("lfs_emubd_prog fseek0 -> %d\n", err);
	        fclose(f);
	        return err;
	    }

	    unsigned int count = 0;
	    for(count = 0; count < cfg->block_count; ++count)
	    	littlfs_erase(cfg, count);
    }

    // Check that file was erased
    assert(f);

    int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
    if (err) {
        printf("lfs_prog fseek0 -> %d\n", err);
        fclose(f);
        return err;
    }


    size_t res = fwrite(data, 1, size, f);
    if (res < size) {
        printf("lfs_prog fwrite -> %ld\n", res);
        fclose(f);
        return res;
    }


    /*err = fseek(f, block * cfg->block_size + off, SEEK_SET);
    if (err) {
        printf("lfs_prog fseek1 -> %d\n", err);
        fclose(f);
        return err;
    }*/

    /*uint8_t dat;
    res = fread(&dat, 1, 1, f);
    if (res < 1) {
        printf("lfs_prog fread -> %d\n", err);
        fclose(f);
        return err;
    }*/

    err = fclose(f);
    if (err) {
        printf("lfs_prog fclose -> %d\n", err);
        return err;
    }

    printf("lfs_prog end!\n");
    return 0;
}

int littlfs_erase(const struct lfs_config *cfg, lfs_block_t block)
{
	// Check if erase is valid
    // assert(block < cfg->block_count);
    //注意：num：对象个数，size：对象占据的内存字节数，相较于malloc函数，calloc函数会自动将内存初始化为0；
    char* perase = malloc(cfg->block_size + 1);  //void* calloc（unsigned int num，unsigned int size
    memset(perase, 0, cfg->block_size);

    FILE *f = fopen("rwtest.txt", "r+");
    if (!f) {
        printf("littlfs_erase fopen -> -1\n");
        free(perase);
        return -1;
    }

    // Check that file was erased
    assert(f);

    int err = fseek(f, block * cfg->block_size, SEEK_SET);
    if (err) {
        printf("littlfs_erase fseek -> %d\n", err);
        fclose(f);
        free(perase);
        return err;
    }

    size_t res = fwrite(perase, cfg->block_size, 1, f);
    if (res < 1) {
        printf("littlfs_erase fwrite -> %d\n", err);
        fclose(f);
        free(perase);
        return err;
    }


    err = fclose(f);
    if (err) {
        printf("lfs_erase fclose -> %d\n", err);
        free(perase);
        return err;
    }

    free(perase);

    //printf("lfs_erase end!\n");
    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{
    /*
    //cfg_t* emu = cfg->context;
	FILE *f = fopen("rwtest.txt", "a+");
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
    */

    //printf("lfs_sync end!\n");
    return 0;
}
