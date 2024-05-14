#pragma once

#include <stdint.h>
#include "thread.h"

pid_t sys_wait(int32_t *status);
void sys_exit(int32_t status);