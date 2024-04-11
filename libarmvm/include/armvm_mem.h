#pragma once

/* **** */

typedef struct armvm_mem_t** armvm_mem_h;
typedef struct armvm_mem_t* armvm_mem_p;

typedef struct armvm_mem_callback_t** armvm_mem_callback_h;
typedef struct armvm_mem_callback_t* armvm_mem_callback_p;

#include <stddef.h>
#include <stdint.h>

typedef uint32_t (*armvm_mem_fn)(uint32_t ppa, size_t size, uint32_t* write, void* param);

#include "armvm.h"

#include <stdlib.h>

/* **** */

#include "libbse/include/log.h"

/* **** */

typedef struct armvm_mem_callback_t {
	armvm_mem_fn fn;
	void* param;
}armvm_mem_callback_t;

/* **** */

void armvm_mem(unsigned const action, armvm_mem_p const mem);
armvm_mem_p armvm_mem_alloc(armvm_mem_h const h2mem, armvm_p const avm);

uint32_t armvm_mem_access_read(const uint32_t ppa, const size_t size,
	armvm_mem_callback_h const h2cb, armvm_mem_p const mem);
armvm_mem_callback_p armvm_mem_access_write(const uint32_t ppa,
	const size_t size, const uint32_t write, armvm_mem_p const mem);

uint32_t armvm_mem_generic_page_ro(uint32_t const ppa, size_t const size,
	uint32_t *const write, void *const param);

uint32_t armvm_mem_generic_page_rw(uint32_t const ppa, size_t const size,
	uint32_t *const write, void *const param);

void armvm_mem_mmap(uint32_t const start, uint32_t const end,
	armvm_mem_fn const fn, void *const param, armvm_mem_p const mem);

/* **** */

static inline uint32_t armvm_mem_callback_io(uint32_t ppa, size_t size,
	uint32_t* write, armvm_mem_callback_p cb)
{
	if(!cb) return(0);
	if(!cb->fn) return(0);

	return(cb->fn(ppa, size, write, cb->param));
}