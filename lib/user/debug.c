#include "debug.h"

#include "print.h"
#include "interrupt.h"

void painc_spin(const char* filename, int line, const char* func, const char* condition) {
  intr_disable();

  put_str("\n\nError:\n\n");
  put_str("File: ");put_str(filename);next_line();
  put_str("Line: ");print_number(line);next_line();
  put_str("Func: ");put_str(func);next_line();
  put_str("Cond: ");put_str(condition);next_line();
  while(1);

}