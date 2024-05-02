#pragma once

#include "ide.h"
#include "list.h"
#include <stdint.h>

typedef struct inode {
  uint32_t i_no; // inode 编号
  uint32_t i_size;
  uint32_t i_open_cnts; // 记录此文件被打开的次数
  bool write_deny; // 写文件不能并行, 进程写文件前检查此标识

  // i_sectors[0-11]是直接块, i_sectors[13]用来存储一级间接块指针
  uint32_t i_sectors[13];
  ListElem inode_tag; // 用于加入已打开的 inode 队列
} INode;

void inode_init(uint32_t inode_no, INode *new_inode);
void inode_sync(Partition *part, INode *inode, void *io_buf);
INode *inode_open(Partition *part, uint32_t inode_no);
void inode_close(INode *inode);
void inode_release(struct partition *part, uint32_t inode_no);