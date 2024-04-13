#pragma once

#include "thread.h"
#include <stdint.h>

void update_tss_esp(TaskStruct *pthread);
void tss_init();