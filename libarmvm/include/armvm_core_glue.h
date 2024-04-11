#pragma once

/* **** */

#define pARMVM_TRACE pARMVM_CORE->armvm_trace
#define pARMVM_CORE core

#define CONFIG (&pARMVM_CORE->config)

#include "armvm_glue.h"
#include "core_trace_glue.h"

/* **** */

#define irGPR(_x) GPRx(ARM_IR_R(_x))

#define irR_NAME(_x) arm_reg_name_lcase_string[0][ARM_IR_R(_x)]

#define IP rSPR32(IP)
#define IR rSPR32(IR)

/* **** */