#include "test.h"
#include "../kernel/string.h"

void test_memset() {
  char arr[] = {
      'X', 'F', 'A', 'B', 'C', 'F', 'X',
  };

  memset(&arr[2], 0, 3);

  ASSERT(arr[0] == 'X');
  ASSERT(arr[1] == 'F');
  ASSERT(arr[5] == 'F');
  ASSERT(arr[6] == 'X');
  for (unsigned int i = 2; i < 5; i++) {
    ASSERT(arr[i] == 0);
  }
}

void test_memcpy() {
  unsigned int arr[6];
  arr[0] = 0xffffdede;
  arr[5] = 0x0a0a0a0a;
  unsigned int brr[] = {0x12345678, 0xcafebabe, 0x0000ffff, 0xacf41231};

  memcpy(&arr[1], brr, sizeof(brr));

  ASSERT(arr[0] == 0xffffdede);
  ASSERT(arr[5] == 0x0a0a0a0a);
  for (unsigned int i = 1; i < 5; i++) {
    ASSERT(arr[i] == brr[i-1]);
  }
}

// // 比较a和b处的size个字节, 大于返回1, 小于返回-1, 相等返回0
// int8_t memcmp(const void* a, const void* b, uint32_t size);

void test_memcmp() {
  const char *a = "Hello";
  const char *b = "Helee";
  ASSERT(memcmp(a, b, 4) == 1);

  a = "Java";
  b = "Java";
  ASSERT(memcmp(a, b, 4) == 0);

  a = "Raiden charm";
  b = "Raiden xharm";
  ASSERT(memcmp(a, b, 13) == -1);
}

// // 将字符串src中的内容复制到dst开始的位置
// char* strcpy(char* dst, const char* src);

void test_strcpy() {
  char a[] = "Hello World.";
  const char *b = "Haha,";

  strcpy(a, b);

  ASSERT(strcmp(a, b) == 0);
}

// // 返回字符串的长度
// uint32_t strlen(const char* str);
void test_strlen() {

  ASSERT(strlen("") == 0);
  ASSERT(strlen("a") == 1);
  ASSERT(strlen("Hello World.") == 12);
}

// // 比较两个字符串, 大于返回1, 小于返回-1, 相等返回0
// int8_t strcmp(const char* a, const char* b);
void test_strcmp() {

  const char *a = "Hello";
  const char *b = "Helee";
  ASSERT(memcmp(a, b, 4) == 1);

  a = "Java";
  b = "Java";
  ASSERT(memcmp(a, b, 4) == 0);

  a = "Raiden charm";
  b = "Raiden xharm";
  ASSERT(memcmp(a, b, 13) == -1);

  // TODO: 不等长字符串比较
}

// // 查找ch第一次出现在str的位置, 不存在则返回NULL
// char* strchr(const char* str, uint8_t ch);
void test_strchr() {
  const char *str = "0abcdefghijklmnopqrstuvwxyz";

  ASSERT(strchr(str, '0') == &str[0])
  ASSERT(strchr(str, 'a') == &str[1])
  ASSERT(strchr(str, 'f') == &str[6])
  ASSERT(strchr(str, 'x') == &str[24])
  ASSERT(strchr(str, 'X') == NULL)
}

// // 从后向前查找字符ch在str中首次出现的位置, 不存在则返回NULL
// char* strrchr(const char* str, const uint8_t ch);
void test_strrchr() {
  const char *str = "0abcdefghijklmnopqrstuvwxyz";

  ASSERT(strchr(str, '0') == &str[0])
  ASSERT(strchr(str, 'a') == &str[1])
  ASSERT(strchr(str, 'f') == &str[6])
  ASSERT(strchr(str, 'x') == &str[24])
  ASSERT(strchr(str, 'X') == NULL)
}

// // 将字符串src拼接在字符串dst之后
// char* strcat(char* dst, const char* src);
void test_strcat() {
  char dst[] = "Hello World.";
  dst[6] = '\0';
  strcat(dst, "LiZeC");
  ASSERT(strcmp(dst, "Hello LiZeC") == 0);
}

// // 统计字符ch在字符串str中出现的次数
// uint32_t strchrs(const char* str, uint8_t ch);
void test_strchrs() {
  const char *str = "It is raining cats and dogs";

  ASSERT(strchrs(str, 's') == 3);
  ASSERT(strchrs(str, 'a') == 3);
  ASSERT(strchrs(str, 't') == 2);

}

int main() {
  test_memset();
  test_memcpy();
  test_memcmp();
  test_strcpy();
  test_strlen();
  test_strcmp();
  test_strchr();
  test_strrchr();
  test_strcat();
  test_strchrs();
}