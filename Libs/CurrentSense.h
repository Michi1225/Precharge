#pragma once    

#include "main.h"

#define PC_SENSITIVITY 0.1618f // V/A
#define BP_SENSITIVITY 0.0325f   // V/A

HAL_StatusTypeDef cs_init();
void cs_run();
float cs_get_pc_current();
float cs_get_bp_current();
uint32_t cs_get_pc_raw();
float get_vrefint();


