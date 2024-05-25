#pragma once
#include "bitmap.h"
#include "list.h"
#include "stdint.h"
#include "superblock.h"
#include "sync.h"

struct disk;

// 分区结构
typedef struct partition {
  uint32_t start_lba;   // 起始扇区
  uint32_t sec_cnt;     // 扇区数
  struct disk *my_disk; // 分区所属的硬盘
  ListElem part_tag;    // 用于队列中的标记
  char name[8];         // 分区名称
  SuperBlock *sb;       // 本分区的超级块
  BitMap block_bitmap;  // 块位图
  BitMap inode_bitmap;  // inode 位图
  List open_inodes;     // 本分区打开的 i 结点队列
} Partition;

// 硬盘结构
typedef struct disk {
  char name[8];                   // 本硬盘的名称
  struct ide_channel *my_channel; // 此块硬盘归属于哪个 ide 通道
  uint8_t dev_no;                 // 本硬盘是主 0, 还是从 1
  struct partition prim_parts[4]; // 主分区顶多是 4 个
  struct partition logic_parts[8]; // 逻辑分区数量无限, 本内核支持 8 个
} Disk;

// ata 通道结构
typedef struct ide_channel {
  char name[8];        // 本 ata 通道名称
  uint16_t port_base;  // 本通道的起始端口号
  uint8_t irq_no;      // 本通道所用的中断号
  Lock lock;           // 通道锁
  bool expecting_intr; // 表示等待硬盘的中断
  Semaphore disk_done; // 用于阻塞、唤醒驱动程序
  Disk devices[2];     // 一个通道上连接两个硬盘, 一主一从
} IdeChannel;

void ide_init();
void ide_read(Disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt);
void ide_write(Disk *hd, uint32_t lba, void *buf, uint32_t sec_cnt);

void sys_print_partition_info();