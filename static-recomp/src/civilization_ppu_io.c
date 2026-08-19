#include "civilization_internal.h"

int civ_ppu_read_reached(CivRecomp *i, uint16_t local, uint8_t *value) {
    int32_t product;
    uint32_t bits;
    unsigned shift;
    if (!i || !value || local < 0x2134u || local > 0x2136u) return 0;

    /* Civilization's closed CPU graph reaches exactly $2134-$2136 on the
       PPU read side.  The result is the signed 16-bit M7A matrix value times
       the signed high byte of M7B, exposed as a 24-bit two's-complement
       product.  $211B-$2120 share mode7_value_latch in the write path above,
       matching the real PPU's single latch rather than inventing per-register
       state. */
    product=(int32_t)(int16_t)i->ppu.mode7_matrix[0] *
            (int32_t)(int8_t)(i->ppu.mode7_matrix[1] >> 8);
    bits=((uint32_t)product)&0x00FFFFFFu;
    shift=(unsigned)(local-0x2134u)*8u;
    i->ppu.ppu1_open_bus=(uint8_t)(bits>>shift);
    i->ppu_multiply_result_read_count[local-0x2134u]++;
    *value=i->ppu.ppu1_open_bus;
    return 1;
}

int civ_ppu_write_reached(CivRecomp *i, uint16_t local, uint8_t value) {
    unsigned index;
    if (!i || local < 0x2100u || local > 0x2133u) return 0;
    index=(unsigned)(local-0x2100u);
    i->ppu_regs[index]=value;
    if (i->ppu_write_count[index] != 0xFFFFu) i->ppu_write_count[index]++;

    /* Decode the already-reached configuration register state so a
       later renderer can consume target-owned state.  This is not a renderer and
       does not claim scanline/cycle effects beyond the reached register latches. */
    switch(local) {
        case 0x2100u:
            if(i->ppu.forced_blank && !(value&0x80u) && !i->v19_forced_blank_clear_observed) {
                i->v19_forced_blank_clear_observed=1u;
                i->v19_forced_blank_clear_value=value;
                i->v19_forced_blank_clear_frame=i->frame_count;
                i->v19_forced_blank_clear_master_clock=i->master_clock;
                i->v19_forced_blank_clear_hcounter=i->hcounter;
                i->v19_forced_blank_clear_vcounter=i->vcounter;
                i->v19_visible_capture_armed=1u;
            }
            i->reg_2100=value;
            i->ppu.forced_blank=(uint8_t)((value>>7)&1u);
            i->ppu.brightness=(uint8_t)(value&0x0Fu);
            break;
        case 0x2101u:
            i->ppu.oam_mode=(uint8_t)((value>>5)&7u);
            i->ppu.oam_base_address=(uint16_t)((uint16_t)(value&7u)<<13);
            i->ppu.oam_address_offset=(uint16_t)((uint16_t)(((value>>3)&3u)+1u)<<12);
            break;
        case 0x2102u:
            i->ppu.oam_ram_address=(uint16_t)((i->ppu.oam_ram_address&0x0100u)|value);
            i->ppu.oam_internal_address=(uint16_t)(i->ppu.oam_ram_address<<1);
            break;
        case 0x2103u:
            i->ppu.oam_ram_address=(uint16_t)((i->ppu.oam_ram_address&0x00FFu)|((uint16_t)(value&1u)<<8));
            i->ppu.oam_internal_address=(uint16_t)(i->ppu.oam_ram_address<<1);
            i->ppu.oam_priority_rotation=(uint8_t)((value>>7)&1u);
            break;
        case 0x2104u: {
            uint16_t oam_addr=i->ppu.oam_internal_address;
            /* The reached route performs OAM DMA while forced blank is active, so the
               CPU-visible internal OAM address is authoritative.  Low-table writes
               commit as an even/odd pair; high-table addresses mirror every 32 bytes. */
            if(oam_addr<512u) {
                if((oam_addr&1u)==0u) {
                    i->oam_write_buffer=value;
                } else {
                    i->oam[oam_addr-1u]=i->oam_write_buffer;
                    i->oam[oam_addr]=value;
                }
            } else {
                uint16_t high=(uint16_t)(0x200u|(oam_addr&0x1Fu));
                if((oam_addr&1u)==0u) i->oam_write_buffer=value;
                i->oam[high]=value;
            }
            i->ppu.oam_internal_address=(uint16_t)((oam_addr+1u)&0x03FFu);
            i->oam_data_write_count++;
            break;
        }
        case 0x2105u: {
            unsigned n;
            i->ppu.bg_mode=(uint8_t)(value&7u);
            i->ppu.mode1_bg3_priority=(uint8_t)((value>>3)&1u);
            for(n=0u;n<4u;n++) i->ppu.bg_large_tiles[n]=(uint8_t)((value>>(4u+n))&1u);
            break;
        }
        case 0x2106u:
            i->ppu.mosaic_size=(uint8_t)(((value>>4)&0x0Fu)+1u);
            i->ppu.mosaic_enabled_mask=(uint8_t)(value&0x0Fu);
            break;
        case 0x2107u: case 0x2108u: case 0x2109u: case 0x210Au: {
            unsigned n=(unsigned)(local-0x2107u);
            i->ppu.bg_tilemap_address[n]=(uint16_t)((uint16_t)(value&0x7Cu)<<8);
            i->ppu.bg_double_width[n]=(uint8_t)(value&1u);
            i->ppu.bg_double_height[n]=(uint8_t)((value>>1)&1u);
            break;
        }
        case 0x210Bu:
            i->ppu.bg_chr_address[0]=(uint16_t)((uint16_t)(value&0x07u)<<12);
            i->ppu.bg_chr_address[1]=(uint16_t)((uint16_t)(value&0x70u)<<8);
            break;
        case 0x210Cu:
            i->ppu.bg_chr_address[2]=(uint16_t)((uint16_t)(value&0x07u)<<12);
            i->ppu.bg_chr_address[3]=(uint16_t)((uint16_t)(value&0x70u)<<8);
            break;
        case 0x210Du:
            i->ppu.mode7_hscroll=(uint16_t)((((uint16_t)value<<8)|i->ppu.mode7_value_latch)&0x1FFFu);
            i->ppu.mode7_value_latch=value;
            i->ppu.bg_hscroll[0]=(uint16_t)((((uint16_t)value<<8)|(i->ppu.hv_scroll_latch&0xF8u)|(i->ppu.h_scroll_latch&0x07u))&0x03FFu);
            i->ppu.hv_scroll_latch=value; i->ppu.h_scroll_latch=value;
            break;
        case 0x210Fu: case 0x2111u: case 0x2113u: {
            unsigned n=(unsigned)((local-0x210Du)>>1);
            i->ppu.bg_hscroll[n]=(uint16_t)((((uint16_t)value<<8)|(i->ppu.hv_scroll_latch&0xF8u)|(i->ppu.h_scroll_latch&0x07u))&0x03FFu);
            i->ppu.hv_scroll_latch=value; i->ppu.h_scroll_latch=value;
            break;
        }
        case 0x210Eu:
            i->ppu.mode7_vscroll=(uint16_t)((((uint16_t)value<<8)|i->ppu.mode7_value_latch)&0x1FFFu);
            i->ppu.mode7_value_latch=value;
            i->ppu.bg_vscroll[0]=(uint16_t)((((uint16_t)value<<8)|i->ppu.hv_scroll_latch)&0x03FFu);
            i->ppu.hv_scroll_latch=value;
            break;
        case 0x2110u: case 0x2112u: case 0x2114u: {
            unsigned n=(unsigned)((local-0x210Eu)>>1);
            i->ppu.bg_vscroll[n]=(uint16_t)((((uint16_t)value<<8)|i->ppu.hv_scroll_latch)&0x03FFu);
            i->ppu.hv_scroll_latch=value;
            break;
        }
        case 0x211Au:
            i->ppu.mode7_select=value;
            break;
        case 0x211Bu: case 0x211Cu: case 0x211Du: case 0x211Eu: {
            unsigned n=(unsigned)(local-0x211Bu);
            i->ppu.mode7_matrix[n]=(uint16_t)(((uint16_t)value<<8)|i->ppu.mode7_value_latch);
            i->ppu.mode7_value_latch=value;
            break;
        }
        case 0x211Fu:
            i->ppu.mode7_center_x=(uint16_t)(((uint16_t)value<<8)|i->ppu.mode7_value_latch);
            i->ppu.mode7_value_latch=value;
            break;
        case 0x2120u:
            i->ppu.mode7_center_y=(uint16_t)(((uint16_t)value<<8)|i->ppu.mode7_value_latch);
            i->ppu.mode7_value_latch=value;
            break;
        case 0x2123u: case 0x2124u: case 0x2125u: {
            unsigned off=(unsigned)(local-0x2123u)*2u;
            i->ppu.window_active[0][off]=(uint8_t)((value>>1)&1u);
            i->ppu.window_active[0][off+1u]=(uint8_t)((value>>5)&1u);
            i->ppu.window_inverted[0][off]=(uint8_t)(value&1u);
            i->ppu.window_inverted[0][off+1u]=(uint8_t)((value>>4)&1u);
            i->ppu.window_active[1][off]=(uint8_t)((value>>3)&1u);
            i->ppu.window_active[1][off+1u]=(uint8_t)((value>>7)&1u);
            i->ppu.window_inverted[1][off]=(uint8_t)((value>>2)&1u);
            i->ppu.window_inverted[1][off+1u]=(uint8_t)((value>>6)&1u);
            break;
        }
        case 0x2126u: i->ppu.window_left[0]=value; break;
        case 0x2127u: i->ppu.window_right[0]=value; break;
        case 0x2128u: i->ppu.window_left[1]=value; break;
        case 0x2129u: i->ppu.window_right[1]=value; break;
        case 0x212Au: {
            unsigned n; for(n=0u;n<4u;n++) i->ppu.mask_logic[n]=(uint8_t)((value>>(2u*n))&3u); break;
        }
        case 0x212Bu:
            i->ppu.mask_logic[4]=(uint8_t)(value&3u); i->ppu.mask_logic[5]=(uint8_t)((value>>2)&3u); break;
        case 0x212Cu: i->ppu.main_screen_layers=(uint8_t)(value&0x1Fu); break;
        case 0x212Du: i->ppu.sub_screen_layers=(uint8_t)(value&0x1Fu); break;
        case 0x212Eu: i->ppu.window_mask_main=(uint8_t)(value&0x1Fu); break;
        case 0x212Fu: i->ppu.window_mask_sub=(uint8_t)(value&0x1Fu); break;
        case 0x2130u:
            i->ppu.color_math_clip_mode=(uint8_t)((value>>6)&3u);
            i->ppu.color_math_prevent_mode=(uint8_t)((value>>4)&3u);
            i->ppu.color_math_add_subscreen=(uint8_t)((value>>1)&1u);
            i->ppu.direct_color_mode=(uint8_t)(value&1u);
            break;
        case 0x2131u:
            i->ppu.color_math_enabled=(uint8_t)(value&0x3Fu);
            i->ppu.color_math_subtract=(uint8_t)((value>>7)&1u);
            i->ppu.color_math_halve=(uint8_t)((value>>6)&1u);
            break;
        case 0x2132u:
            if(value&0x80u) i->ppu.fixed_color=(uint16_t)((i->ppu.fixed_color&0x03FFu)|((uint16_t)(value&0x1Fu)<<10));
            if(value&0x40u) i->ppu.fixed_color=(uint16_t)((i->ppu.fixed_color&0x7C1Fu)|((uint16_t)(value&0x1Fu)<<5));
            if(value&0x20u) i->ppu.fixed_color=(uint16_t)((i->ppu.fixed_color&0x7FE0u)|(value&0x1Fu));
            break;
        case 0x2133u:
            i->ppu.extbg_enabled=(uint8_t)((value>>6)&1u);
            i->ppu.pseudo_hires=(uint8_t)((value>>3)&1u);
            i->ppu.overscan=(uint8_t)((value>>2)&1u);
            i->ppu.obj_interlace=(uint8_t)((value>>1)&1u);
            i->ppu.screen_interlace=(uint8_t)(value&1u);
            break;
        default: break;
    }

    /* Reached PPU memory-port semantics are part of the current hardware layer. */
    if (local==0x2115u) {
        static const uint16_t sizes[4]={1u,32u,128u,128u};
        i->vram_increment_size=sizes[value&3u];
        i->vram_mapping=(uint8_t)((value>>2)&3u);
        i->vram_increment_on_high=(uint8_t)((value>>7)&1u);
    } else if (local==0x2116u) {
        i->vram_address=(uint16_t)((i->vram_address&0xFF00u)|value);
    } else if (local==0x2117u) {
        i->vram_address=(uint16_t)((i->vram_address&0x00FFu)|((uint16_t)value<<8));
    } else if (local==0x2118u || local==0x2119u) {
        unsigned byte=(unsigned)(local-0x2118u);
        uint32_t phys;
        if (i->vram_mapping!=0u)
            return civ_fail_frontier(i,"Reached an unproved VRAM remapping mode.",NULL);
        phys=((((uint32_t)i->vram_address)<<1)+byte)&0xFFFFu;
        i->vram[phys]=value;
        i->vram_data_write_count++;
        if ((byte==0u && !i->vram_increment_on_high) || (byte==1u && i->vram_increment_on_high))
            i->vram_address=(uint16_t)(i->vram_address+i->vram_increment_size);
    } else if (local==0x2121u) {
        i->cgram_address=value;
        i->cgram_latch_phase=0u;
    } else if (local==0x2122u) {
        i->cgram_data_write_count++;
        if (i->cgram_latch_phase==0u) {
            i->cgram_latch_low=value;
            i->cgram_latch_phase=1u;
        } else {
            uint16_t off=(uint16_t)((uint16_t)i->cgram_address*2u);
            i->cgram[off]=i->cgram_latch_low;
            i->cgram[(uint16_t)(off+1u)]=(uint8_t)(value&0x7Fu);
            i->cgram_address=(uint8_t)(i->cgram_address+1u);
            i->cgram_latch_phase=0u;
        }
    }
    return 1;
}

