#pragma once

#include <stdint.h>

#define MAX_FILES_PER_PART 4096 // 每个分区所支持最大创建的文件数
#define BITS_PER_SECTOR 4096    // 每扇区的位数
#define SECTOR_SIZE 512         // 扇区字节大小
#define BLOCK_SIZE SECTOR_SIZE  // 块字节大小
#define MAX_PATH_LEN 64         // 最大路径长度

// 文件类型
typedef enum file_types {
  FT_UNKNOWN,  // 不支持的文件类型
  FT_REGULAR,  // 普通文件
  FT_DIRECTORY // 目录
} FileType;

// 文件读写位置偏移量
typedef enum whence { SEEK_SET = 1, SEEK_CUR, SEEK_END } Whence;

// 用来记录查找文件过程中已找到的上级路径
typedef struct path_search_record {
  char searched_path[MAX_PATH_LEN]; // 查找过程中的父路径
  struct dir *parent_dir;           // 文件或目录所在的直接父目录
  enum file_types file_type;        // 文件类型
} PathSearchRecord;

// 文件操作类型
#define O_RDONLY 0
#define O_WRONLY 1
#define O_RDWR   3
#define O_CREAT  4

void filesys_init();
int32_t sys_open(const char *pathname, uint8_t flags);
int32_t sys_close(int32_t fd);
int32_t sys_write(int32_t fd, const void *buf, uint32_t count);