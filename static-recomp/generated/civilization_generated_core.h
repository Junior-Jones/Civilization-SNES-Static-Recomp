#ifndef CIVILIZATION_GENERATED_CORE_H
#define CIVILIZATION_GENERATED_CORE_H
#include <stdint.h>
#include "civilization_static_recomp.h"
int civ_generated_core_step(CivRecomp *i);
unsigned civ_generated_core_context_count(void);
uint32_t civ_generated_core_context_key(const CivRecomp *i);
int civ_generated_return_allowed(uint16_t proof_id,uint32_t target);
#endif
