#pragma once

#include "inode.h"
#include "dir.h"
#include <stdint.h>

// 文件结构
typedef struct file {
  // 记录当前文件操作的偏移地址, 以 0 为起始, 最大为文件大小 - 1
  uint32_t fd_pos;
  uint32_t fd_flag;
  INode *fd_inode;
} File;

// 标准输入输出描述符
typedef enum std_fd {
  stdin_no,  // 0 标准输入
  stdout_no, // 1 标准输出
  stderr_no  // 2 标准错误
} StdFdType;

// 位图类型
typedef enum bitmap_type {
  INODE_BITMAP, // inode 位图
  BLOCK_BITMAP  // 块位图
} FdBitMapType;

#define MAX_FILE_OPEN 32 // 系统可打开的最大文件数


int32_t block_bitmap_alloc(Partition *part);
int32_t inode_bitmap_alloc(Partition *part);
void bitmap_sync(Partition *part, uint32_t bit_idx, uint8_t btmp);
int32_t file_create(Dir* parent_dir, char* filename, uint8_t flag);
