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

void open_root_dir(Partition *part);
Dir *dir_open(Partition *part, uint32_t inode_no);
bool search_dir_entry(Partition *part, Dir *pdir, const char *name,
                      DirEntry *dir_e);
void dir_close(Dir *dir);
void create_dir_entry(char *filename, uint32_t inode_no, uint8_t file_type,
                      DirEntry *p_de);
bool sync_dir_entry(Dir *parent_dir, DirEntry *p_de, void *io_buf);
bool delete_dir_entry(struct partition *part, struct dir *pdir,
                      uint32_t inode_no, void *io_buf);
struct dir_entry *dir_read(struct dir *dir);
bool dir_is_empty(struct dir *dir);
int32_t dir_remove(struct dir *parent_dir, struct dir *child_dir);
