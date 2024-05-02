#pragma once

#include "dir.h"
#include "stdint.h"
#include "thread.h"

enum SYSCALL_NR { // 用来存放子功能号
  SYS_RESTART,
  SYS_FORK,
  SYS_READ,
  SYS_WRITE,
  SYS_OPEN,
  SYS_CLOSE,
  SYS_WAITPID,
  SYS_CREAT,
  SYS_LINK,
  SYS_UNLINK,
  SYS_EXECVE,

  SYS_LSEEK = 19,
  SYS_GETPID,

  SYS_MOUNT,
  SYS_UMOUNT,
  
  SYS_MALLOC,
  SYS_FREE,
  SYS_PUTCHAR,
  SYS_CLEAR,

  SYS_GETCWD = 79,
  SYS_CHDIR,

  SYS_MKDIR = 83,
  SYS_RMDIR,

  SYS_OPENDIR,
  SYS_CLOSEDIR,
  SYS_READDIR,
  SYS_REWINDDIR,
  SYS_PS,

  SYS_STAT = 106,

};

// SYS_RESTART

pid_t fork();
int32_t read(int32_t fd, void *buf, uint32_t count);
uint32_t write(int32_t fd, const void *buf, uint32_t count);
int32_t open(char *pathname, uint8_t flag);
int32_t close(int32_t fd);

// SYS_WAITPID
// SYS_CREAT
// SYS_LINK

int32_t unlink(const char *pathname);

// SYS_EXECVE

int32_t lseek(int32_t fd, int32_t offset, uint8_t whence);
uint32_t getpid(void);

// SYS_MOUNT
// SYS_UMOUNT

void *malloc(uint32_t size);
void free(void *ptr);
void putchar(char char_ascii);
void clear();

char *getcwd(char *buf, uint32_t size);
int32_t chdir(const char *path);
int32_t mkdir(const char *pathname);
int32_t rmdir(const char *pathname);
Dir *opendir(const char *name);
int32_t closedir(Dir *dir);
DirEntry *readdir(Dir *dir);
void rewinddir(Dir *dir);
void ps(void);
int32_t stat(const char *path, struct stat *buf);
