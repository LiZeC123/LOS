#define PAINC(...) assert(__FILE__, __LINE__, __func__, __VA_ARGS__)

#define ASSERT(COND)                                                           \
  if (COND) {                                                                  \
  } else {                                                                     \
    PAINC(#COND);                                                              \
  }

void assert(const char *filename, int line, const char *func,
            const char *condition) {
  printf("===\n File: %s\n Line: %d\n Func: %s\n Cond: %s\n===\n", filename,
         line, func, condition);
}