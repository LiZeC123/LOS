#pragma once

void painc_spin(const char *filename, int line, const char *func,
                const char *condition);

#define PANIC(...) painc_spin(__FILE__, __LINE__, __func__, __VA_ARGS__)

#ifdef NODEBUG
#define ASSERT(COND) ((void)(COND))
#else
#define ASSERT(COND)                                                           \
  if (COND) {                                                                  \
  } else {                                                                     \
    PANIC(#COND);                                                              \
  }

#endif