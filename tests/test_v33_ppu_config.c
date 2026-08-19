#include "civilization_internal.h"
#include <stdio.h>
#include <string.h>

#define W(a,v) do{if(!civ_bus_write8(&c,(a),(v)))return __LINE__;}while(0)
int main(void)
{
    CivRecomp c = {0}, bad = {0}; civ_reset(&c);
    W(0x002100u,0x8Fu); if(!c.ppu.forced_blank||c.ppu.brightness!=15u)return 10;
    W(0x002101u,0x62u); if(c.ppu.oam_mode!=3u||c.ppu.oam_base_address!=0x4000u||c.ppu.oam_address_offset!=0x1000u)return 11;
    W(0x002102u,0x34u);W(0x002103u,0x81u);if(c.ppu.oam_ram_address!=0x0134u||c.ppu.oam_internal_address!=0x0268u||!c.ppu.oam_priority_rotation)return 12;
    W(0x002105u,0xB9u);if(c.ppu.bg_mode!=1u||!c.ppu.mode1_bg3_priority||!c.ppu.bg_large_tiles[0]||!c.ppu.bg_large_tiles[1]||c.ppu.bg_large_tiles[2]||!c.ppu.bg_large_tiles[3])return 13;
    W(0x002106u,0xA5u);if(c.ppu.mosaic_size!=11u||c.ppu.mosaic_enabled_mask!=5u)return 14;
    W(0x002107u,0x63u);if(c.ppu.bg_tilemap_address[0]!=0x6000u||!c.ppu.bg_double_width[0]||!c.ppu.bg_double_height[0])return 15;
    W(0x00210Bu,0x74u);if(c.ppu.bg_chr_address[0]!=0x4000u||c.ppu.bg_chr_address[1]!=0x7000u)return 16;
    W(0x00210Du,0x12u);W(0x00210Du,0x34u);if(c.ppu.bg_hscroll[0]!=0x0012u||c.ppu.mode7_hscroll!=0x1412u)return 17;
    W(0x00210Eu,0x56u);W(0x00210Eu,0x78u);if(c.ppu.bg_vscroll[0]!=0x0056u||c.ppu.mode7_vscroll!=0x1856u)return 18;
    W(0x00211Au,0xC3u);if(c.ppu.mode7_select!=0xC3u)return 19;
    W(0x00211Bu,0x9Au);W(0x00211Bu,0xBCu);if(c.ppu.mode7_matrix[0]!=0xBC9Au)return 20;
    W(0x002123u,0x99u);if(!c.ppu.window_inverted[0][0]||c.ppu.window_active[0][0]||!c.ppu.window_active[1][0]||c.ppu.window_inverted[1][0])return 21;
    W(0x002126u,10u);W(0x002127u,20u);W(0x002128u,30u);W(0x002129u,40u);if(c.ppu.window_left[0]!=10u||c.ppu.window_right[0]!=20u||c.ppu.window_left[1]!=30u||c.ppu.window_right[1]!=40u)return 22;
    W(0x00212Au,0xE4u);if(c.ppu.mask_logic[0]!=0u||c.ppu.mask_logic[1]!=1u||c.ppu.mask_logic[2]!=2u||c.ppu.mask_logic[3]!=3u)return 23;
    W(0x00212Cu,0x17u);W(0x00212Du,0x03u);if(c.ppu.main_screen_layers!=0x17u||c.ppu.sub_screen_layers!=3u)return 24;
    W(0x002130u,0xB3u);if(c.ppu.color_math_clip_mode!=2u||c.ppu.color_math_prevent_mode!=3u||!c.ppu.color_math_add_subscreen||!c.ppu.direct_color_mode)return 25;
    W(0x002131u,0xC7u);if(c.ppu.color_math_enabled!=7u||!c.ppu.color_math_subtract||!c.ppu.color_math_halve)return 26;
    W(0x002132u,0x3Fu);W(0x002132u,0x5Au);W(0x002132u,0x9Cu);if(c.ppu.fixed_color!=0x735Fu)return 27;
    W(0x002133u,0x4Fu);if(!c.ppu.extbg_enabled||!c.ppu.pseudo_hires||!c.ppu.overscan||!c.ppu.obj_interlace||!c.ppu.screen_interlace)return 28;
    /* Version 16 has now reached $2104. Preserve all Version 15 configuration
       checks while confirming the upgraded OAM authority no longer fail-closes. */
    civ_reset(&bad);
    if(!civ_bus_write8(&bad,0x002102u,0x00u)||!civ_bus_write8(&bad,0x002103u,0x00u)||
       !civ_bus_write8(&bad,0x002104u,0x12u)||!civ_bus_write8(&bad,0x002104u,0x34u)||
       bad.failed||bad.oam[0]!=0x12u||bad.oam[1]!=0x34u)return 29;
    puts("PASS Version 15 decoded PPU configuration regressions; Version 16 OAM upgrade remains compatible"); return 0;
}
