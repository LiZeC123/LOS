#include "print.h"

int main() {
  const char *str = "Hello From Kernel.\n 12\b3";
  for (int i = 0; i < 24; i++) {
    put_char(str[i]);
  }

  while (1) {
  };

  return 0;
}