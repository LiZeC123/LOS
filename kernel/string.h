#include <stdint.h>

// 从dst开始的size字节设置为value
void memset(void* dst, uint8_t value, uint32_t size);

// 将src处的size个字节复制到dst处
void memcpy(void* dst, const void* src, uint32_t size);

// 比较a和b处的size个字节, 大于返回1, 小于返回-1, 相等返回0
int8_t memcmp(const void* a, const void* b, uint32_t size);

// 将字符串src中的内容复制到dst开始的位置
char* strcpy(char* dst, const char* src);

// 返回字符串的长度
uint32_t strlen(const char* str);

// 比较两个字符串, 大于返回1, 小于返回-1, 相等返回0
int8_t strcmp(const char* a, const char* b);

// 查找ch第一次出现在str的位置, 不存在则返回NULL
char* strchr(const char* str, uint8_t ch);

// 从后向前查找字符ch在str中首次出现的位置, 不存在则返回NULL
char* strrchr(const char* str, const uint8_t ch);

// 将字符串src拼接在字符串dst之后
char* strcat(char* dst, const char* src);

// 统计字符ch在字符串str中出现的次数
uint32_t strchrs(const char* str, uint8_t ch);