#pragma once

/* **** */

typedef struct armvm_t** armvm_h;
typedef struct armvm_t* armvm_p;

/* **** */


/* **** */

#include "armvm_config.h"
#include "armvm_coprocessor.h"
#include "armvm_core.h"
#include "armvm_mem.h"
#include "armvm_tlb.h"
#include "armvm_trace.h"

/* **** */

#include "stdint.h"
#include "stdlib.h"

/* **** */

typedef struct armvm_t {
	armvm_coprocessor_p coprocessor;
	armvm_core_p core;
	armvm_mem_p mem;
	armvm_tlb_p tlb;
//
	armvm_config_t config;
}armvm_t;

/* **** */

void armvm(unsigned action, armvm_p avm);
//armvm_p armvm_alloc_init(armvm_p h2avm);
armvm_p armvm_alloc(armvm_h h2avm);
//armvm_p armvm_init(armvm_p avm);
//void armvm_reset(armvm_p avm);
uint64_t armvm_run(uint64_t cycles, armvm_p avm);
void armvm_step(armvm_p avm);
//void armvm_step_trace(armvm_p avm);
