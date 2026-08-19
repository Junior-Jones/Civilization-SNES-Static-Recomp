#include "civilization_internal.h"
#include "civilization_audio.h"

static int wram_offset(uint32_t address, uint32_t *offset) {
    uint8_t bank=(uint8_t)(address>>16); uint16_t local=(uint16_t)address;
    if (bank==0x7Eu) { *offset=local; return 1; }
    if (bank==0x7Fu) { *offset=0x10000u+local; return 1; }
    if (((bank<=0x3Fu)||(bank>=0x80u && bank<=0xBFu)) && local<0x2000u) { *offset=local; return 1; }
    return 0;
}

static int io_bank(uint8_t bank) {
    return bank <= 0x3Fu || (bank >= 0x80u && bank <= 0xBFu);
}





int civ_bus_read8(CivRecomp *i, uint32_t address, uint8_t *value) {
    uint32_t off;
    int cartridge_result;
    uint8_t bank;
    uint16_t local;
    if (!i||!value) return 0;
    address &= 0xFFFFFFu; bank=(uint8_t)(address>>16); local=(uint16_t)address;
    civ_cpu_data_access(i,address);
    if (wram_offset(address,&off)) { *value=i->wram[off]; return 1; }
    if (io_bank(bank) && local>=0x2134u && local<=0x2136u) {
        return civ_ppu_read_reached(i,local,value);
    }
    if (io_bank(bank) && local>=0x2140u && local<=0x2143u) {
        unsigned port=(unsigned)(local-0x2140u);
        if(!i->v20_full_static_audio_enabled)
            return civ_fail_frontier(i,"Production APUIO read occurred before Full Static S-SMP ownership was acquired.",NULL);
        if(!civ_v20_audio_cpu_read(i,port,value))return 0;
        return 1;
    }
    if (io_bank(bank) && (local==0x4016u || local==0x4017u)) {
        *value=civ_controller_serial_read(i,(unsigned)(local-0x4016u));
        return 1;
    }
    if (io_bank(bank) && local==0x4210u) {
        uint16_t nmi_scanline=(uint16_t)((i->ppu_regs[0x33u]&0x04u)?240u:225u);
        *value=i->nmi_flag?0x80u:0u;
        /* On the NMI scanline the CPU forces RDNMI high through H=5; reads
           can clear it starting at H=6. */
        if(i->nmi_flag && (i->vcounter!=nmi_scanline || i->hcounter>=6u))
            i->nmi_flag=0u;
        return 1;
    }
    if (io_bank(bank) && local==0x4211u) {
        *value=i->irq_line?0x80u:0u;
        if (i->irq_line && i->irq_hold_master_clocks==0u) { i->irq_line=0u; i->irq_transition=0u; }
        i->timeup_read_count++;
        return 1;
    }
    if (io_bank(bank) && local==0x4212u) {
        uint8_t v=(uint8_t)(i->auto_joypad_busy?0x01u:0u);
        /* Current Civilization IRQ callbacks consume HVBJOY bit 6 (HBlank).
           Historical certification also exposed bit 0 for a target-specific mouse probe. */
        if(!i->natural_timing_enabled)civ_timing_advance_master(i,2u);
        if (i->hcounter<=2u || i->hcounter>=1096u) v|=0x40u;
        if(!i->natural_timing_enabled)civ_timing_advance_master(i,4u);
        i->hvbjoy_read_count++;
        *value=v;
        return 1;
    }
    if(io_bank(bank) && local>=0x4218u && local<=0x421Fu) {
        unsigned slot=(unsigned)((local-0x4218u)>>1);
        uint16_t data=i->auto_joypad_data[slot];
        *value=(local&1u)?(uint8_t)(data>>8):(uint8_t)data;
        return 1;
    }
    if(io_bank(bank) && local>=0x4214u && local<=0x4217u) {
        uint16_t result=(local<0x4216u)?i->cpu_math_quotient:i->cpu_math_remainder;
        *value=(local&1u)?(uint8_t)(result>>8):(uint8_t)result;
        return 1;
    }
    if(io_bank(bank)&&((local>=0x4300u&&local<=0x4306u)||(local>=0x4310u&&local<=0x4316u)||
                        (local>=0x4320u&&local<=0x432Au)||(local>=0x4370u&&local<=0x4376u))) {
        CivDmaChannel0 *ch;
        unsigned reg=(unsigned)(local&0x000Fu);
        if(local>=0x4370u)ch=&i->dma7;
        else if(local>=0x4320u)ch=&i->dma2;
        else if(local>=0x4310u)ch=&i->dma1;
        else ch=&i->dma0;
        switch(reg) {
            case 0u:*value=ch->dmap;break;
            case 1u:*value=ch->bbad;break;
            case 2u:*value=(uint8_t)ch->source_address;break;
            case 3u:*value=(uint8_t)(ch->source_address>>8);break;
            case 4u:*value=ch->source_bank;break;
            case 5u:*value=(uint8_t)ch->transfer_size;break;
            case 6u:*value=(uint8_t)(ch->transfer_size>>8);break;
            case 7u:if(local>=0x4320u&&local<0x4330u)*value=ch->indirect_bank;else return civ_fail_frontier(i,"Unproved DMA indirect-bank register read.",NULL);break;
            case 8u:*value=(uint8_t)i->hdma2.table_address;break;
            case 9u:*value=(uint8_t)(i->hdma2.table_address>>8);break;
            case 10u:*value=i->hdma2.line_counter_repeat;break;
            default:return civ_fail_frontier(i,"Unproved DMA register read.",NULL);
        }
        return 1;
    }
    cartridge_result=civ_cartridge_read(i,bank,local,value);
    if(cartridge_result>=0)return cartridge_result;
    return civ_fail_frontier(i,"Static bus read reached an unsupported/unproved address.",NULL);
}
int civ_bus_read16(CivRecomp *i, uint32_t address, uint16_t *value) {
    uint8_t lo,hi; uint32_t next=(address&0xFF0000u)|((address+1u)&0xFFFFu);
    if(!civ_bus_read8(i,address,&lo)||!civ_bus_read8(i,next,&hi))return 0;
    *value=(uint16_t)(lo|((uint16_t)hi<<8)); return 1;
}
int civ_bus_write8(CivRecomp *i, uint32_t address, uint8_t value) {
    uint32_t off;
    int cartridge_result;
    uint8_t bank;
    uint16_t local;
    if (!i) return 0;
    address &= 0xFFFFFFu; bank=(uint8_t)(address>>16); local=(uint16_t)address;
    civ_cpu_data_access(i,address);
    if (wram_offset(address,&off)) { i->wram[off]=value; return 1; }
    cartridge_result=civ_cartridge_write(i,address,value);
    if(cartridge_result>=0)return cartridge_result;
    if (io_bank(bank) && local==0x4016u) {
        uint8_t prev=i->controller_strobe;
        uint8_t next=(uint8_t)(value&1u);
        i->controller_strobe_write_count++;
        if(next) {
            unsigned controller;
            for(controller=0u;controller<2u;controller++) {
                if(i->controller_device[controller]!=CIV_INPUT_DEVICE_MOUSE)i->controller_latched[controller]=i->controller_live[controller];
                i->controller_shift_index[controller]=0u;
            }
        }
        i->controller_strobe=next;
        if(prev && !next)civ_controller_latch(i);
        return 1;
    }
    if (io_bank(bank) && local>=0x2140u && local<=0x2143u) {
        unsigned port=(unsigned)(local-0x2140u);
        if(!i->v20_full_static_audio_enabled)
            return civ_fail_frontier(i,"Production APUIO write occurred before Full Static S-SMP ownership was acquired.",NULL);
        return civ_v20_audio_cpu_write(i,port,value);
    }
    if (io_bank(bank) && local>=0x2100u && local<=0x2133u)
        return civ_ppu_write_reached(i,local,value);
    if (io_bank(bank) && local>=0x4200u && local<=0x420Du) {
        unsigned index=(unsigned)(local-0x4200u);
        if (local==0x420Cu) {
            if(value!=0u && value!=0x04u)
                return civ_fail_frontier(i,"HDMAEN reached outside the source-proved Civilization {0,$04} domain.",NULL);
            if(value==0x04u && !civ_hdma2_config_is_proved(i))
                return civ_fail_frontier(i,"HDMAEN enabled channel 2 before its exact Civilization configuration was established.",NULL);
        }
        i->cpu_io[index]=value;
        if (i->cpu_io_write_count[index] != 0xFFFFu) i->cpu_io_write_count[index]++;
        if(local==0x4202u) i->cpu_math_mul_a=value;
        else if(local==0x4203u) {
            i->cpu_math_mul_b=value;
            i->cpu_math_remainder=(uint16_t)((uint16_t)i->cpu_math_mul_a*(uint16_t)value);
            i->cpu_math_operation_count++;
        } else if(local==0x4204u) {
            i->cpu_math_dividend=(uint16_t)((i->cpu_math_dividend&0xFF00u)|value);
        } else if(local==0x4205u) {
            i->cpu_math_dividend=(uint16_t)((i->cpu_math_dividend&0x00FFu)|((uint16_t)value<<8));
        } else if(local==0x4206u) {
            i->cpu_math_divisor=value;
            i->cpu_math_operation_count++;
            if(value) {
                i->cpu_math_quotient=(uint16_t)(i->cpu_math_dividend/value);
                i->cpu_math_remainder=(uint16_t)(i->cpu_math_dividend%value);
            } else {
                i->cpu_math_quotient=0xFFFFu;
                i->cpu_math_remainder=i->cpu_math_dividend;
            }
        } else if (local==0x4200u) {
            uint8_t old_nmi_enable=i->nmi_enable;
            uint8_t new_nmi_enable=(uint8_t)((value>>7)&1u);
            i->reg_4200=value;
            i->auto_joypad_enable=(uint8_t)(value&1u);
            i->hirq_enable=(uint8_t)((value>>4)&1u);
            i->virq_enable=(uint8_t)((value>>5)&1u);
            i->nmi_enable=new_nmi_enable;
            /* Enabling NMI while RDNMI is already high produces the hardware
               two-sample delayed edge used by the S-CPU interrupt pipeline. */
            if(i->nmi_flag && new_nmi_enable && !old_nmi_enable)
                i->nmi_flag_counter=2u;
            if (!i->hirq_enable && !i->virq_enable) { i->irq_line=0u; i->irq_transition=0u; i->irq_hold_master_clocks=0u; }
        } else if (local==0x4207u) {
            i->htime_raw=(uint16_t)((i->htime_raw&0x0100u)|value);
        } else if (local==0x4208u) {
            i->htime_raw=(uint16_t)((i->htime_raw&0x00FFu)|((uint16_t)(value&1u)<<8));
        } else if (local==0x4209u) {
            i->vtime_raw=(uint16_t)((i->vtime_raw&0x0100u)|value);
        } else if (local==0x420Au) {
            i->vtime_raw=(uint16_t)((i->vtime_raw&0x00FFu)|((uint16_t)(value&1u)<<8));
        } else if(local==0x420Bu&&value!=0u) {
            /* MDMAEN schedules DMA; it does not run inline with the
               register write.  This preserves the hardware one-cycle start delay
               and lets the next CPU cycle supply the resume bus speed. */
            if(value!=1u && value!=2u && value!=0x80u)
                return civ_fail_frontier(i,"Simultaneous or unproved DMA-channel enable mask reached.",NULL);
            if(i->dma_pending_mask!=0u)
                return civ_fail_frontier(i,"A second MDMAEN write reached while manual DMA was still pending.",NULL);
            i->dma_pending_mask=value;
            i->dma_start_delay=1u;
        }
        if (local==0x420Du) i->reg_420d=value;
        return 1;
    }
    if(io_bank(bank)&&((local>=0x4300u&&local<=0x4307u)||(local>=0x4310u&&local<=0x4317u)||
                        (local>=0x4320u&&local<=0x4327u)||(local>=0x4370u&&local<=0x4377u))) {
        CivDmaChannel0 *ch;
        unsigned reg=(unsigned)(local&0x000Fu);
        unsigned channel;
        if(local>=0x4370u){ch=&i->dma7;channel=7u;}
        else if(local>=0x4320u){ch=&i->dma2;channel=2u;}
        else if(local>=0x4310u){ch=&i->dma1;channel=1u;}
        else {ch=&i->dma0;channel=0u;}
        switch(reg) {
            case 0u:ch->dmap=value;return 1;
            case 1u:ch->bbad=value;return 1;
            case 2u:ch->source_address=(uint16_t)((ch->source_address&0xFF00u)|value);return 1;
            case 3u:ch->source_address=(uint16_t)((ch->source_address&0x00FFu)|((uint16_t)value<<8));return 1;
            case 4u:ch->source_bank=value;return 1;
            case 5u:ch->transfer_size=(uint16_t)((ch->transfer_size&0xFF00u)|value);return 1;
            case 6u:ch->transfer_size=(uint16_t)((ch->transfer_size&0x00FFu)|((uint16_t)value<<8));return 1;
            case 7u:
                if(channel!=2u)return civ_fail_frontier(i,"Reached manual-DMA route has not proved indirect-bank register use.",NULL);
                ch->indirect_bank=value;return 1;
            default:return civ_fail_frontier(i,"Unproved DMA register write.",NULL);
        }
    }
    return civ_fail_frontier(i,"Static bus write reached an unsupported/unproved address.",NULL);
}
int civ_bus_write16(CivRecomp *i, uint32_t address, uint16_t value) {
    uint32_t next=(address&0xFF0000u)|((address+1u)&0xFFFFu);
    return civ_bus_write8(i,address,(uint8_t)value) && civ_bus_write8(i,next,(uint8_t)(value>>8));
}
