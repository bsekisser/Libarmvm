#include "armvm_mem.h"

/* **** */

#include "armvm_action.h"
#include "armvm_mem_config.h"
#include "armvm.h"

/* **** */

#include "libbse/include/err_test.h"
#include "libbse/include/handle.h"
#include "libbse/include/mem_access_le.h"
#include "libbse/include/page.h"
#include "libbse/include/queue.h"
#include "libbse/include/unused.h"

/* **** */

#include <stddef.h>
#include <stdint.h>

/* **** */

typedef struct armvm_mem_t {
	void* l1[PTD(~0)];
	armvm_mem_callback_p l2heap[PAGE_SIZE][PAGE_SIZE];
//
	queue_t l2free;
	struct {
		void* ptr[PAGE_SIZE];
		unsigned count;
		unsigned limit;
	}l2malloc;
//
	armvm_p avm;
	armvm_mem_config_t config;
	armvm_mem_h h2mem;
}armvm_mem_t;

/* **** */

static void* _armvm_mem_access_l1(const uint32_t ppa, void** *const h2l1e, armvm_mem_p const mem)
{
	void* *const p2l1e = &mem->l1[PTD(ppa)];
	void* const p2l2 = *p2l1e;

	if(h2l1e) *h2l1e = p2l1e;

	return(p2l2);
}

static armvm_mem_callback_p _armvm_mem_access_l2(const uint32_t ppa, void *const p2l2, armvm_mem_p const mem)
{
	const armvm_mem_callback_p l2 = p2l2;

	return(&l2[PTE(ppa)]);
	UNUSED(mem);
}

static armvm_mem_callback_p _armvm_mem_access(const uint32_t ppa, void** *const h2l1e, armvm_mem_p const mem)
{
	void *const p2l2 = _armvm_mem_access_l1(ppa, h2l1e, mem);

	if(!p2l2) return(0);

	return(_armvm_mem_access_l2(ppa, p2l2, mem));
}

static armvm_mem_callback_p _armvm_mem_mmap_alloc_free(armvm_mem_p const mem)
{
	qelem_p const qel = mem->l2free.head;
	armvm_mem_callback_p const p2l2 = (armvm_mem_callback_p)qel;

	if(qel) {
		qelem_p const next = qel->next;

		if(mem->config.trace.mmap.alloc.free) {
			LOG_START(">> qel: 0x%016" PRIxPTR, (uintptr_t)qel);
			LOG_END(", next: 0x%016" PRIXPTR, (uintptr_t)next);
		}

		mem->l2free.head = next;
	}

	return(p2l2);
}

static armvm_mem_callback_p _armvm_mem_mmap_alloc_malloc(const size_t l2size, armvm_mem_p const mem)
{
	const armvm_mem_callback_p p2l2 = malloc(l2size);

	const unsigned count = mem->l2malloc.count;

	assert((1 + count) < mem->l2malloc.limit);

	mem->l2malloc.ptr[count] = p2l2;
	mem->l2malloc.count++;

	if(mem->config.trace.mmap.alloc.malloc) {
		LOG_START(">> p2l2: 0x%016" PRIxPTR, (uintptr_t)p2l2);
		_LOG_(", size: 0x%08zx", l2size);
		LOG_END(", count: 0x%08x", count);
	}

	return(p2l2);
}

static armvm_mem_callback_p _armvm_mem_mmap_alloc(const unsigned ppa, armvm_mem_p const mem)
{
	const unsigned l1ptd = PTD(ppa);
	const unsigned l2pte = PTE(ppa);
	const unsigned offset = PAGE_OFFSET(ppa);

	if(0)
		LOG("ppa: 0x%08x -- %03x:%03x:%03x", ppa, l1ptd, l2pte, offset);

	const size_t l2size = sizeof(armvm_mem_callback_t) * PTE_SIZE;

	void** p2l1e = 0;
	armvm_mem_callback_p p2l2 = _armvm_mem_access_l1(ppa, &p2l1e, mem);

	if(!p2l2) {
		p2l2 = _armvm_mem_mmap_alloc_free(mem);

		if(!p2l2)
			p2l2 = _armvm_mem_mmap_alloc_malloc(l2size, mem);

		if(mem->config.trace.mmap_alloc) {
			LOG_START("l2size: 0x%08zx", l2size);
			_LOG_(", p2l1e: 0x%016" PRIxPTR, (uintptr_t)p2l1e);
			LOG_END(", l2: 0x%016" PRIxPTR, (uintptr_t)p2l2);
		}

		if(!p2l2) return(0);

		*p2l1e = p2l2;
		memset(p2l2, 0, l2size);

		p2l2 = &p2l2[l2pte];
	}

	return(p2l2);
}

static void armvm__mem_alloc_init(armvm_mem_p const mem)
{
	if(mem->avm->config.trace.alloc_init) LOG();
}

static void armvm__mem_exit(armvm_mem_p mem)
{
	if(mem->avm->config.trace.exit) LOG();

	handle_free((void*)mem->h2mem);
}

void armvm_mem(unsigned action, armvm_mem_p mem)
{
	switch(action) {
		case ARMVM_ACTION_ALLOC_INIT: return(armvm__mem_alloc_init(mem));
		case ARMVM_ACTION_EXIT: return(armvm__mem_exit(mem));
	}
}

uint32_t armvm_mem_access_read(const uint32_t ppa, const size_t size,
	armvm_mem_callback_h h2cb, armvm_mem_p mem)
{
	armvm_mem_callback_p cb = _armvm_mem_access(ppa, 0, mem);

	if(h2cb) *h2cb = cb;

	return(armvm_mem_callback_io(ppa, size, 0, cb));
}

armvm_mem_callback_p armvm_mem_access_write(const uint32_t ppa, const size_t size,
	uint32_t write, armvm_mem_p mem)
{
	armvm_mem_callback_p cb = _armvm_mem_access(ppa, 0, mem);

	armvm_mem_callback_io(ppa, size, &write, cb);

	return(cb);
}

armvm_mem_p armvm_mem_alloc(armvm_mem_h h2mem, armvm_p avm)
{
	ERR_NULL(h2mem);
	ERR_NULL(avm);

	if(avm->config.trace.alloc) LOG();

	armvm_mem_p mem = handle_calloc((void*)h2mem, 1, sizeof(armvm_mem_t));
	ERR_NULL(mem);

	avm->config.mem = &mem->config;

	mem->avm = avm;
	mem->h2mem = h2mem;

	/* **** */

	queue_init(&mem->l2free);

	for(unsigned i = 0; i < PAGE_SIZE; i++)
		queue_enqueue((qelem_p)&mem->l2heap[i][0], &mem->l2free);

	memset(&mem->l2malloc, 0, sizeof(mem->l2malloc));

	/* **** */

	return(mem);
}

uint32_t armvm_mem_generic_page_ro(const uint32_t ppa, const size_t size, uint32_t *const write, void *const param)
{
//	uint8_t *const p = param + PAGE_OFFSET(ppa);
	uint8_t *const p = param + ppa;
	return(mem_access_le(p, size, 0));
	UNUSED(write);
}

uint32_t armvm_mem_generic_page_rw(const uint32_t ppa, const size_t size, uint32_t *const write, void *const param)
{
//	uint8_t *const p = param + PAGE_OFFSET(ppa);
	uint8_t *const p = param + ppa;
	return(mem_access_le(p, size, write));
}

void armvm_mem_mmap(const uint32_t base, const uint32_t end,
	armvm_mem_fn const fn, void *const param, armvm_mem_p const mem)
{
	const uint32_t start = base & PAGE_MASK;
	const uint32_t stop = end & PAGE_MASK;

	for(uint32_t ppa = start; ppa <= stop; ppa += PAGE_SIZE) {
		armvm_mem_callback_p cb = _armvm_mem_mmap_alloc(ppa, mem);

		if(mem->config.trace_mmap) {
			LOG_START(">> cb: 0x%016" PRIxPTR, (uintptr_t)cb);
			_LOG_(", ppa: 0x%08x", ppa);
			_LOG_(", fn: 0x%016" PRIxPTR, (uintptr_t)fn);
			LOG_END(", param: 0x%016" PRIxPTR, (uintptr_t)param);
		}

//		uint8_t *const data_offset = param + (ppa - base);
		uint8_t *const data_offset = param - base;

		if(((armvm_mem_fn)~0U) == fn) {
			cb->fn = armvm_mem_generic_page_ro;
			cb->param = data_offset;
		} else if(((armvm_mem_fn)0U) == fn) {
			cb->fn = armvm_mem_generic_page_rw;
			cb->param = data_offset;
		} else {
			cb->fn = fn;
			cb->param = param;
		}
	}
}
