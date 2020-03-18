//#include "little_fs.h"
//#include "little_fs_config.h"
//#include "lfs.h"
//#include "lfs_port.h"
//#include <stdio.h>
//
//#define LITTLE_FS_MAX_LOOKAHEAD (32 * ((LITTLE_FS_MAX_BLOCK_COUNT + 31) / 32))
//
//lfs_t little_fs = {0};
//static uint8_t read_buffer[LITTLE_FS_READ_SIZE];
//static uint8_t program_buffer[LITTLE_FS_PROGRAM_SIZE];
//static uint8_t lookahead_buffer[LITTLE_FS_MAX_LOOKAHEAD / 8];
//
//
//
//static bool is_init = false;
//static struct lfs_config little_fs_config = {
//    .read = little_fs_read,
//    .prog = little_fs_program,
//    .erase = little_fs_erase,
//    .sync = little_fs_sync,
//    .read_size = LITTLE_FS_READ_SIZE,
//    .prog_size = LITTLE_FS_PROGRAM_SIZE,
//    .block_size = LITTLE_FS_BLOCK_SIZE,
//    .block_count = 0,
//    .lookahead = 0,
//    .read_buffer = read_buffer,
//    .prog_buffer = program_buffer,
//    .lookahead_buffer = lookahead_buffer,
//};
//uint32_t get_block_count(void)
//{
//        return 3484;
//}
//uint32_t get_lookahead(void)
//{
//    uint32_t block_count = get_block_count();
//    return (32 * ((block_count + 31) / 32));
//}
//
///**
// * @brief
// *
// * @return true
// * @return false
// */
//void little_fs_init(void)
//{    
//    // flash_spi_init();
//    //! 从qf移植的memory_pool，比较简单，分配内存的时候没有循环        
//    little_fs_config.block_count = get_block_count();
//    little_fs_config.lookahead = get_lookahead();
//    int mount_ret = lfs_mount(&little_fs, &little_fs_config);
//    if (mount_ret != LFS_ERR_OK)
//    {
//        // sFLASH_NOR_ChipErase();
//        const int format_ret = lfs_format(&little_fs, &little_fs_config);
//        if (format_ret != LFS_ERR_OK)
//        {
//            printf("lfs_format error=%d", format_ret);            
//        }
//        mount_ret = lfs_mount(&little_fs, &little_fs_config);
//        if (mount_ret != LFS_ERR_OK)
//        {
//			printf("lfs_mount 2 error=%d", mount_ret);            
//        }
//    }
//
//
//}
