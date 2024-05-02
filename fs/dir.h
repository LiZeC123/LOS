#pragma once

#include "fs.h"
#include "inode.h"
#include <stdint.h>

#define MAX_FILE_NAME_LEN 16 // 最大文件名长度


// 目录结构
typedef struct dir {
  INode *inode;
  uint32_t dir_pos;     // 记录在目录内的偏移
  uint8_t dir_buf[512]; // 目录的数据缓冲
} Dir;

// 目录项结构
typedef struct dir_entry {
  char filename[MAX_FILE_NAME_LEN]; // 普通文件或目录名称
  uint32_t i_no;   // 普通文件或目录对应的 inode 编号
  FileType f_type; // 文件类型
} DirEntry;
