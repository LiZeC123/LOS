#pragma once

#include <stdint.h>

// 输出一个字符到屏幕, 使用汇编实现
void put_char(uint8_t char_ascii);

void put_str(const char *str);

void print_number(uint64_t num);

void next_line();

// 清空屏幕 使用汇编实现
void cls_screen();

// 内核内使用的print函数, 直接操作显存
void printk(const char *format, ...);