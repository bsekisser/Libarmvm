#pragma once

/* **** */

#include <stdint.h>

/* **** */

static uint32_t armcc_gen__ir(armcc_p const p2cc, const uint32_t ir)
{
	p2cc->ip = p2cc->pc;
	p2cc->pc += 4;

	const uint32_t cs_ip = p2cc->cs + p2cc->ip;
	const uint32_t cs_pc = p2cc->cs + p2cc->pc;

	if(0) {
		LOG_START("(cs: 0x%08x, ip: 0x%08x): 0x%08x", p2cc->cs, p2cc->ip, cs_ip);
		_LOG_(", pc: 0x%08x", p2cc->pc);
		_LOG_(", (cs + pc): 0x%08x", cs_pc);
		LOG_END(", ir: 0x%08x", ir);
	}

	uint8_t* p2cs = (uint8_t*)p2cc->p2cs;
	uint32_t* p = (uint32_t*)(p2cs + (p2cc->ip & 0xffff));

	if(0) {
		LOG_START("p2cs: 0x%016" PRIxPTR, (uintptr_t)p2cs);
		LOG_END(", p: 0x%016" PRIxPTR, (uintptr_t)p);
	}

	*p = ir;

	return(cs_pc);
}
