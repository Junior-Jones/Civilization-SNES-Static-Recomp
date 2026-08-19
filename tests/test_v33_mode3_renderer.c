#include "civilization_internal.h"

#include <stdint.h>
#include <stdio.h>

static void set_tile_color(CivRecomp *c, uint32_t base, unsigned bpp, uint8_t color)
{
    unsigned plane;
    for (plane = 0u; plane < bpp; ++plane) {
        if ((color & (1u << plane)) != 0u) {
            uint32_t address = base + (plane >> 1) * 16u + (plane & 1u);
            c->vram[address & 0xFFFFu] = 0x80u;
        }
    }
}

static void set_map_entry(CivRecomp *c, uint16_t word_address, uint16_t entry)
{
    uint32_t a = (uint32_t)word_address * 2u;
    c->vram[a & 0xFFFFu] = (uint8_t)entry;
    c->vram[(a + 1u) & 0xFFFFu] = (uint8_t)(entry >> 8);
}

static void set_cgram(CivRecomp *c, unsigned index, uint16_t color)
{
    unsigned a = (index & 0xFFu) * 2u;
    c->cgram[a] = (uint8_t)color;
    c->cgram[a + 1u] = (uint8_t)(color >> 8);
}

static uint32_t rgba(uint16_t color)
{
    unsigned r = color & 31u;
    unsigned g = (color >> 5) & 31u;
    unsigned b = (color >> 10) & 31u;
    r = (r << 3) | (r >> 2);
    g = (g << 3) | (g >> 2);
    b = (b << 3) | (b >> 2);
    return UINT32_C(0xFF000000) | (r << 16) | (g << 8) | b;
}

static uint16_t direct_color(unsigned palette, uint8_t color)
{
    return (uint16_t)(((((color & 0x07u) << 1) | (palette & 0x01u)) << 1) |
                      (((color & 0x38u) | ((palette & 0x02u) << 1)) << 4) |
                      (((color & 0xC0u) | ((palette & 0x04u) << 3)) << 7));
}

static void setup_mode3(CivRecomp *c)
{
    civ_reset(c);
    c->ppu.forced_blank = 0u;
    c->ppu.brightness = 15u;
    c->ppu.bg_mode = 3u;
    c->ppu.main_screen_layers = 0x01u;
    c->ppu.sub_screen_layers = 0u;
    c->ppu.window_mask_main = 0u;
    c->ppu.window_mask_sub = 0u;
    c->ppu.bg_tilemap_address[0] = 0x1000u;
    c->ppu.bg_chr_address[0] = 0u;
}

int main(void)
{
    CivRecomp c = {0};
    const uint32_t *fb;
    const uint8_t bg1_color = 0xA5u;

    setup_mode3(&c);
    set_map_entry(&c, 0x1000u, (uint16_t)(5u << 10));
    set_tile_color(&c, 0u, 8u, bg1_color);
    set_cgram(&c, bg1_color, 0x001Fu);
    if (!civ_render_current_frame(&c)) return 1;
    fb = civ_get_framebuffer_rgba(&c);
    if (!fb || fb[0] != UINT32_C(0xFFFF0000)) {
        fprintf(stderr, "mode3 8bpp CGRAM mismatch got=%08X\n", fb ? fb[0] : 0u);
        return 1;
    }

    setup_mode3(&c);
    c.ppu.direct_color_mode = 1u;
    set_map_entry(&c, 0x1000u, (uint16_t)(5u << 10));
    set_tile_color(&c, 0u, 8u, bg1_color);
    if (!civ_render_current_frame(&c)) return 1;
    fb = civ_get_framebuffer_rgba(&c);
    if (!fb || fb[0] != rgba(direct_color(5u, bg1_color))) {
        fprintf(stderr, "mode3 direct-color mismatch got=%08X expected=%08X\n",
                fb ? fb[0] : 0u, rgba(direct_color(5u, bg1_color)));
        return 1;
    }

    setup_mode3(&c);
    c.ppu.main_screen_layers = 0x03u;
    c.ppu.bg_tilemap_address[1] = 0x1200u;
    c.ppu.bg_chr_address[1] = 0x1800u;
    set_map_entry(&c, 0x1000u, 0u); /* BG1 low priority. */
    set_tile_color(&c, 0u, 8u, 1u);
    set_cgram(&c, 1u, 0x001Fu);
    set_map_entry(&c, 0x1200u, 0x2000u); /* BG2 high priority, palette 0. */
    set_tile_color(&c, 0x3000u, 4u, 1u);
    set_cgram(&c, 1u, 0x03E0u);
    if (!civ_render_current_frame(&c)) return 1;
    fb = civ_get_framebuffer_rgba(&c);
    if (!fb || fb[0] != UINT32_C(0xFF00FF00)) {
        fprintf(stderr, "mode3 BG2 priority mismatch got=%08X\n", fb ? fb[0] : 0u);
        return 1;
    }

    puts("PASS Civilization Version 33 Mode 3 renderer");
    return 0;
}
