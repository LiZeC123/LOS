#pragma once
#include "../lib/kernel/print.h"

#define PRINTLINE(STR, NUMBER)                                                 \
  put_str(STR);                                                                \
  print_number(NUMBER);                                                        \
  next_line();
