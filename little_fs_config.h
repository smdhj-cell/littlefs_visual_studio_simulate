#pragma once
#ifndef __LITTLE_FS_CONFIG_H__
#define __LITTLE_FS_CONFIG_H__
/**
 * @brief 与flash通讯每次至少读取多少字节
 *        此值过大，需要大量内存
 *        此值过小，每次读取都会与flash进行通讯
 *
 */
#define LITTLE_FS_READ_SIZE (256)

/**
 * @brief 与flash通讯每次至少写入多少字节
 *        必须是LITTLE_FS_READ_SIZE的倍数
 *
 */
#define LITTLE_FS_PROGRAM_SIZE (256)

/**
 * @brief flash每次至少擦除多少字节
 *        必须是LITTLE_FS_PROGRAM_SIZE的倍数
 *
 */
#define LITTLE_FS_BLOCK_SIZE (4096)

/**
 * @brief LITTLE_FS_BLOCK_SIZE * LITTLE_FS_BLOCK_COUNT = flash大小
 * 
 * total 8*1024*1024/4096 = 2048  font_use 2*1024*1024/4096 = 512
 * ota use 100*4096
 */
// #define LITTLE_FS_BLOCK_COUNT (1436) 
//#define LITTLE_FS_MAX_BLOCK_COUNT (3484) 

/**
 * @brief 最大同时打开的文件数，每一个文件需要LITTLE_FS_PROGRAM_SIZE 大小的缓存
 *        静态分配了 LITTLE_FS_MAX_FILE_COUNT * LITTLE_FS_PROGRAM_SIZE的buffer
 *        内存不够用时，降低这个值，同时减少同时打开的文件数
 *
 */
//#define LITTLE_FS_MAX_FILE_COUNT (10)
#endif
