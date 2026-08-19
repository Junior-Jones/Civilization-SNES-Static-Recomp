#ifndef CIVILIZATION_INTERNAL_H
#define CIVILIZATION_INTERNAL_H

#define CIVILIZATION_CORE_INTERNAL 1
#include "civilization_static_recomp.h"
#undef CIVILIZATION_CORE_INTERNAL
#include <stdint.h>
#include <stddef.h>

#define CIV_P_C 0x01u
#define CIV_P_Z 0x02u
#define CIV_P_I 0x04u
#define CIV_P_D 0x08u
#define CIV_P_X 0x10u
#define CIV_P_M 0x20u
#define CIV_P_V 0x40u
#define CIV_P_N 0x80u

void civ_sha256_hex(const uint8_t *data, size_t length, char output[65]);
uint32_t civ_crc32(const uint8_t *data, size_t length);
void civ_set_nz8(CivRecomp *i, uint8_t value);
void civ_set_nz16(CivRecomp *i, uint16_t value);
void civ_cmp8(CivRecomp *i, uint8_t left, uint8_t right);
void civ_cmp16(CivRecomp *i, uint16_t left, uint16_t right);
int civ_adc8_binary(CivRecomp *i, uint8_t value, const char *address);
int civ_adc16_binary(CivRecomp *i, uint16_t value, const char *address);
int civ_sbc8_binary(CivRecomp *i, uint8_t value, const char *address);
int civ_sbc16_binary(CivRecomp *i, uint16_t value, const char *address);
int civ_irq_enter_native(CivRecomp *i);
int civ_nmi_enter_native(CivRecomp *i);
int civ_rti_native(CivRecomp *i);
int civ_bus_read8(CivRecomp *i, uint32_t address, uint8_t *value);
int civ_bus_read16(CivRecomp *i, uint32_t address, uint16_t *value);
int civ_bus_write8(CivRecomp *i, uint32_t address, uint8_t value);
int civ_bus_write16(CivRecomp *i, uint32_t address, uint16_t value);
int civ_ppu_read_reached(CivRecomp *i,uint16_t local,uint8_t *value);
int civ_ppu_write_reached(CivRecomp *i,uint16_t local,uint8_t value);
int civ_cartridge_read(CivRecomp *i,uint8_t bank,uint16_t local,uint8_t *value);
int civ_cartridge_write(CivRecomp *i,uint32_t address,uint8_t value);
int civ_push8(CivRecomp *i, uint8_t value);
int civ_pull8(CivRecomp *i, uint8_t *value);
int civ_block_move_step(CivRecomp *i, uint8_t destination_bank, uint8_t source_bank, int adjust, const char *address);
int civ_require_width(CivRecomp *i, unsigned e, unsigned m, unsigned x, const char *address);
uint8_t civ_cpu_bus_speed(const CivRecomp *i, uint32_t address);
uint8_t civ_dma_process_cpu_cycle(CivRecomp *i, uint8_t cpu_speed);
void civ_hdma_schedule_init(CivRecomp *i);
void civ_hdma_schedule_scanline(CivRecomp *i);
int civ_hdma2_config_is_proved(const CivRecomp *i);
void civ_controller_latch(CivRecomp *i);
uint8_t civ_controller_serial_read(CivRecomp *i,unsigned controller);
void civ_cpu_data_access(CivRecomp *i, uint32_t address);
void civ_cpu_internal_cycles(CivRecomp *i, unsigned count);
void civ_cpu_begin_static_instruction(CivRecomp *i, uint8_t opcode,
                                      unsigned length, uint32_t operand);
int civ_fail_frontier(CivRecomp *i, const char *reason, const char *address);
int civ_headless_frame_stop(CivRecomp *i);
void civ_video_begin_visible_frame_capture(CivRecomp *i);
void civ_video_freeze_visible_frame(CivRecomp *i);
#endif
