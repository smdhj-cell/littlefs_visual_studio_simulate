#include "lfs_config.h"
#include <unistd.h>

lfs_file_t file;

lfs_t little_fs = {0};

lfs_dir_t dir_data;
struct lfs_info dirlfsinfo;

static uint8_t read_buffer[LITTLE_FS_READ_SIZE];
static uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE];
static uint8_t lookahead_buffer[LITTLE_FS_MAX_LOOKAHEAD / 8]; // 16

static struct lfs_config little_fs_config = {
    .read = littlfs_read,
    .prog = littlfs_program,
    .erase = littlfs_erase,
    .sync = littlfs_sync,

    .read_size = LITTLE_FS_READ_SIZE,         //读
    .prog_size = LITTLE_FS_PROGRAM_SIZE,      //写
    .block_size = LITTLE_FS_BLOCK_SIZE,       //块大小	2^12 = 4096
    .block_count = LITTLE_FS_MAX_BLOCK_COUNT, // block块的数量
    //总的大小为: block_count * block_size = 4096 * 2^6 = 2^18

    //用于标记block的bit数 每一个block需要1bit作为标记	且要求为32的倍数
    //即需要lookahead >= block_count;
    .lookahead = LITTLE_FS_MAX_LOOKAHEAD,

    .read_buffer = read_buffer,
    .prog_buffer = program_buffer,
    //一个字节8bit位,根据lookahead需要多少的bit位得出需要的字节数大小	即
    // lookahead_buffer=lookahead/8 + 1
    .lookahead_buffer = lookahead_buffer,
};

int mount_init(lfs_t *little_fs, struct lfs_config *little_fs_config)
{
    int mount_ret = lfs_mount(little_fs, little_fs_config); //挂载littlefs
    if (mount_ret != LFS_ERR_OK)                            //挂载失败
    {
        // sFLASH_NOR_ChipErase();
        const int format_ret = lfs_format(little_fs, little_fs_config); //使用littlefs格式化块设备
        if (format_ret != LFS_ERR_OK)                                   // littlefs格式化块设备失败
        {
            printf("lfs_format error=%d\n", format_ret);
        }
        mount_ret = lfs_mount(little_fs, little_fs_config);
        if (mount_ret != LFS_ERR_OK)
        {
            printf("lfs_mount 2 error=%d\n", mount_ret);
        }
    }

    return mount_ret;
}

int littlefs_cp_dir(char *open_path, char *save_path, lfs_t little_fs, lfs_dir_t dir_data, struct lfs_info dirlfsinfo, lfs_file_t file)
{

#ifdef DEBUG
    printf("path = %s\n", open_path);
#endif

    int ret = lfs_dir_open(&little_fs, &dir_data, open_path);
    if (ret < LFS_ERR_OK)
    {
        printf("copy lfs_dir_open error=%d\n", ret);
        return ret;
    }

    while (1)
    {
        ret = lfs_dir_read(&little_fs, &dir_data, &dirlfsinfo);
        if (ret < LFS_ERR_OK)
        {
            printf("copy lfs_stat error=%d\n", ret);
            lfs_dir_close(&little_fs, &dir_data);
            return ret;
        }

#ifdef DEBUG
        printf("dirlfsinfo.type = %x, dirlfsinfo.size = %d, dirlfsinfo.name = %s\n", dirlfsinfo.type, dirlfsinfo.size, dirlfsinfo.name);
#endif

        //剔除.和..这两个文件
        if (strcmp(dirlfsinfo.name, ".") == 0 || strcmp(dirlfsinfo.name, "..") == 0)
        {
            continue;
        }

        if (dirlfsinfo.name[0] == 32 || dirlfsinfo.name[0] == 0)
        {
            lfs_dir_close(&little_fs, &dir_data);
            break;
        }

        if (dirlfsinfo.type == LFS_TYPE_DIR)
        {
            printf("find a dir = %s\n", dirlfsinfo.name);
            char *next_openpath = calloc(LITTLE_FS_MAX_FILENAME + 1 + strlen(open_path), 1);
            char *next_savepath = calloc(LITTLE_FS_MAX_FILENAME + 1 + strlen(save_path), 1);

            if (strcmp(open_path, "/") == 0)
            {
                sprintf(next_openpath, "/%s", dirlfsinfo.name);
            }
            else
            {
                sprintf(next_openpath, "%s/%s", open_path, dirlfsinfo.name);
            }

            sprintf(next_savepath, "%s/%s", save_path, dirlfsinfo.name);

#ifdef DEBUG
            printf("next_openpath = %s\n", next_openpath);
            printf("next_savepath = %s\n", next_savepath);
#endif
            mkdir(next_savepath, 0777);

            ret = littlefs_cp_dir(next_openpath, next_savepath, little_fs, dir_data, dirlfsinfo, file);
            if (ret < LFS_ERR_OK)
            {
                printf("copy littlefs_cp_dir error=%d\n", ret);
                free(next_savepath);
                free(next_openpath);
                lfs_dir_close(&little_fs, &dir_data);
                return ret;
            }

            free(next_savepath);
            free(next_openpath);
        }
        else if (dirlfsinfo.type == LFS_TYPE_REG)
        {

            char *openfile = calloc(LITTLE_FS_MAX_FILENAME + 1 + strlen(open_path), 1);
            char *savefile = calloc(LITTLE_FS_MAX_FILENAME + 1 + strlen(save_path), 1);

            if (strcmp(open_path, "/") == 0)
            {
                sprintf(openfile, "/%s", dirlfsinfo.name);
            }
            else
            {
                sprintf(openfile, "%s/%s", open_path, dirlfsinfo.name);
            }

            sprintf(savefile, "%s/%s", save_path, dirlfsinfo.name);

#ifdef DEBUG
            printf("openfile = %s\n", openfile);
            printf("savefile = %s\n", savefile);
#endif

            ret = lfs_file_open(&little_fs, &file, openfile, LFS_O_RDONLY);
            if (ret < LFS_ERR_OK)
            {
                printf("copy lfs_file_open error=%d\n", ret);
                free(openfile);
                free(savefile);
                lfs_dir_close(&little_fs, &dir_data);
                return ret;
            }

            char *save_data = calloc(1, dirlfsinfo.size + 2);

            //确保是LITTLE_FS_READ_SIZE的倍数并比数据多一个字节预留为结束符
            lfs_ssize_t res = lfs_file_read(&little_fs, &file, save_data, ((dirlfsinfo.size + LITTLE_FS_READ_SIZE) / LITTLE_FS_READ_SIZE) * LITTLE_FS_READ_SIZE);
            if (res < 0)
            {
                printf("copy lfs_file_read error=%d\n", res);
                free(openfile);
                free(savefile);
                return res;
            }

            FILE *f = fopen(savefile, "w+");
            if (!f)
            {
                printf("copy fopen create -> -1\n");
                free(save_data);
                free(openfile);
                free(savefile);
                lfs_dir_close(&little_fs, &dir_data);
                return -1;
            }

            res = fwrite(save_data, 1, dirlfsinfo.size, f);
            if (res < dirlfsinfo.size)
            {
                printf("fwrite -> %d\n", res);
                free(save_data);
                fclose(f);
                free(openfile);
                free(savefile);
                lfs_dir_close(&little_fs, &dir_data);
                return res;
            }

            free(save_data);

            ret = fclose(f);
            if (ret)
            {
                printf("fclose -> %d\n", ret);
                free(save_data);
                lfs_dir_close(&little_fs, &dir_data);
                return ret;
            }

            free(openfile);
            free(savefile);

            printf("copy one file is end = %s\n", dirlfsinfo.name);
        }
    }
}

int main(int argc, char const *argv[])
{
    printf("start!!!!littlefs_file_path = %s\n", LITTLE_CP_PATH);

    /*
        //First test
        char str[256] = "hello good morning world!!!  is so cool!!! excuse me?!";   //最小操作读写快为#define LITTLE_FS_READ_SIZE (128) #define LITTLE_FS_PROGRAM_SIZE (128)

        char *rp = NULL;

        rp = calloc(0, LITTLE_FS_BLOCK_SIZE + 1);
        
        littlfs_program(&little_fs_config, 0, 0, str, LITTLE_FS_PROGRAM_SIZE);

        littlfs_read(&little_fs_config, 0, 0, rp, LITTLE_FS_READ_SIZE);

        printf("read data = %s\n", rp);

        //littlfs_erase(&little_fs_config, 0);

        free(rp);
    */

    int ret = mount_init(&little_fs, &little_fs_config);
    if (ret < LFS_ERR_OK)
    {
        printf("lfs_mount error=%d\n", ret);
        return ret;
    }

    ret = mkdir(CP_SAVE_PATH, 0777);
    if (ret < LFS_ERR_OK && errno != EEXIST)
    {
        printf("mkdir error=%d\n", ret);
        lfs_unmount(&little_fs);
        return ret;
    }

    ret = lfs_dir_open(&little_fs, &dir_data, "/");
    if (ret < LFS_ERR_OK)
    {
        printf("lfs_dir_open error=%d\n", ret);
        lfs_unmount(&little_fs);
        return ret;
    }

    ret = littlefs_cp_dir("/", CP_SAVE_PATH, little_fs, dir_data, dirlfsinfo, file);
    if (ret < LFS_ERR_OK)
    {
        printf("littlefs_cp_dir error %d\n", ret);
        lfs_unmount(&little_fs);
        return ret;
    }

    /**************************TEST*********************************/
    // while(i){
    // 	ret = lfs_dir_read(&little_fs, &dir_data, &dirlfsinfo);
    // 	if (ret < LFS_ERR_OK){
    // 		printf("copy lfs_stat error=%d\n", ret);
    // 		lfs_dir_close(&little_fs, &dir_data);
    // 		lfs_unmount(&little_fs);
    // 		return ret;
    // 	}

    //	if(dirlfsinfo.name[0] == 32 || dirlfsinfo.name[0] == 0){
    // 		lfs_dir_close(&little_fs, &dir_data);
    // 		break;
    // 	}

    // 	printf("dirlfsinfo.type = %x, dirlfsinfo.size = %d, dirlfsinfo.name = %s\n", dirlfsinfo.type, dirlfsinfo.size, dirlfsinfo.name);
    // }
    /**************************TEST*********************************/

    ret = lfs_dir_close(&little_fs, &dir_data);
    if (ret < LFS_ERR_OK)
    {
        printf("lfs_dir_close error=%d\n", ret);
        lfs_unmount(&little_fs);
        return ret;
    }

    ret = lfs_unmount(&little_fs);
    if (ret < 0)
    {
        printf("lfs_unmount error=%d\n", ret);
        return 0;
    }

    printf("end!!!!!\n");

    return 0;
}

int littlfs_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{

#ifdef DEBUG
    printf("littlfs_read(%p, %d, %d, %p, %d)\n", (void *)cfg, block, off, buffer, size);
#endif

    char *p_buffer = (char *)buffer;
    assert(off % cfg->read_size == 0); // void assert( int expression );    它的条件返回错误，则终止程序执行
    assert(size % cfg->read_size == 0);
    assert(block < cfg->block_count);

    // printf("path = %s\n", p_lfs_config->path);
    FILE *f = fopen(LITTLE_CP_PATH, "r");
    if (!f)
    {
        f = fopen(LITTLE_CP_PATH, "w+");
        if (!f)
        {
            printf("lfs_read fopen create -> -1\n");
            return -1;
        }

        int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
        if (err)
        {
            printf("lfs_read fseek0 -> %d\n", err);
            fclose(f);
            return err;
        }

        unsigned int count = 0;
        for (count = 0; count < cfg->block_count; ++count)
            littlfs_erase(cfg, count);
        return 0;
    }

    int err = fseek(f, block * cfg->block_size + off, SEEK_SET); // int fseek(FILE *stream, long offset, int fromwhere); 移动指针
    if (err)
    {
        printf("lfs_read fseek1 -> %d\n", err);
        fclose(f);
        return err;
    }

    // printf("lfs_read test!!! \n");
    // Check that file was erased
    assert(f);

    size_t res = fread(p_buffer, 1, size, f); //从f读取大小size的字节数到data
    if (res < size && !feof(f))
    { // int feof(FILE *stream); 如果文件结束，则返回非0值，否则返回0，文件结束符只能被clearerr()清除
        printf("lfs_read fread -> %ld\n", res);
    }

    // printf("fread data size = %ld\n", res);

    //////////////////////////////////没有关闭文件
    err = fclose(f);
    if (err)
    {
        printf("lfs_read fclose -> %d\n", err);
        return err;
    }

#ifdef DEBUG
    printf("lfs_read end!\n");
#endif

    return 0;
}

int littlfs_program(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
#ifdef DEBUG
    printf("littlfs_program(%p, %d, %d, %p, %d)\n", (void *)cfg, block, off, buffer, size);
#endif

    uint8_t *data = (char *)buffer;

    // Check if write is valid
    assert(off % cfg->prog_size == 0);
    assert(block < cfg->block_count);
    assert(size % cfg->prog_size == 0);

    FILE *f = fopen(LITTLE_CP_PATH, "r+b");
    if (!f)
    {
        f = fopen(LITTLE_CP_PATH, "w+");
        if (!f)
        {
            printf("lfs_program fopen create -> -1\n");
            return -1;
        }

        int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
        if (err)
        {
            printf("littlfs_program fseek0 -> %d\n", err);
            fclose(f);
            return err;
        }

        unsigned int count = 0;
        for (count = 0; count < cfg->block_count; ++count)
            littlfs_erase(cfg, count);
    }

    int err = fseek(f, block * cfg->block_size + off, SEEK_SET);
    if (err)
    {
        printf("lfs_program fseek0 -> %d\n", err);
        fclose(f);
        return err;
    }

    // Check that file was erased
    assert(f);

    size_t res = fwrite(data, 1, size, f);
    if (res < size)
    {
        printf("lfs_program fwrite -> %ld\n", res);
    }

    // printf("fwrite data size = %ld\n", res);

    //////////////////////////////////没有关闭文件
    err = fclose(f);
    if (err)
    {
        printf("lfs_program fclose -> %d\n", err);
        return err;
    }

#ifdef DEBUG
    printf("lfs_prog end!\n");
#endif
    return 0;
}

int littlfs_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    // Check if erase is valid
    // assert(block < cfg->block_count);
    //注意：num：对象个数，size：对象占据的内存字节数，相较于malloc函数，calloc函数会自动将内存初始化为0；
    char *perase = malloc(cfg->block_size + 1); // void* calloc（unsigned int num，unsigned int size
    memset(perase, 255, cfg->block_size);

    FILE *f = fopen(LITTLE_CP_PATH, "r+b");
    if (!f)
    {
        printf("littlfs_erase fopen -> -1\n");
        free(perase);
        return -1;
    }

    // Check that file was erased
    // assert(f);

    int err = fseek(f, block * cfg->block_size, SEEK_SET);
    if (err)
    {
        printf("littlfs_erase fseek -> %d\n", err);
        free(perase);
        fclose(f);
        return err;
    }

    size_t res = fwrite(perase, cfg->block_size, 1, f);
    if (res < 1)
    {
        printf("littlfs_erase fwrite -> %ld\n", res);
    }

    err = fclose(f);
    if (err)
    {
        printf("lfs_erase fclose -> %d\n", err);
        free(perase);
        return err;
    }

    free(perase);

#ifdef DEBUG
    printf("lfs_erase end!\n");
#endif

    return 0;
}

int littlfs_sync(const struct lfs_config *cfg)
{

#ifdef DEBUG
    printf("lfs_sync end!\n");
#endif

    return 0;
}