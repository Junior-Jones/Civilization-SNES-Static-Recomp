#include "civilization_internal.h"

#include <string.h>

typedef struct CivV20Pixel {
    uint16_t color;
    uint8_t priority;
    uint8_t layer;
    uint8_t obj_palette;
    uint8_t transparent;
} CivV20Pixel;

static uint16_t v20_vram_word(const CivRecomp *i, uint32_t byte_address)
{
    uint32_t a = byte_address & 0xFFFFu;
    return (uint16_t)(i->vram[a] | ((uint16_t)i->vram[(a + 1u) & 0xFFFFu] << 8));
}

static uint16_t v20_cgram_color(const CivRecomp *i, unsigned index)
{
    unsigned a = (index & 0xFFu) * 2u;
    return (uint16_t)(i->cgram[a] | ((uint16_t)(i->cgram[a + 1u] & 0x7Fu) << 8));
}

static uint8_t v20_tile_pixel(const CivRecomp *i, unsigned bpp,
                              uint32_t chr_base, unsigned tile,
                              unsigned row, unsigned col)
{
    uint32_t base = chr_base + (uint32_t)tile * (bpp * 8u) + (row & 7u) * 2u;
    unsigned bit = 7u - (col & 7u);
    uint8_t color = 0u;
    unsigned plane;
    for (plane = 0u; plane < bpp; ++plane) {
        uint32_t address = base + (plane >> 1) * 16u + (plane & 1u);
        color |= (uint8_t)(((i->vram[address & 0xFFFFu] >> bit) & 1u) << plane);
    }
    return color;
}

static uint8_t v20_bg_priority(const CivRecomp *i, unsigned layer, unsigned high)
{
    static const uint8_t mode1_priority[3][2] = {{6u, 9u}, {5u, 8u}, {1u, 3u}};
    static const uint8_t mode3_priority[2][2] = {{3u, 7u}, {1u, 5u}};
    if (i->ppu.bg_mode == 3u) return mode3_priority[layer][high != 0u];
    if (layer == 2u && high && i->ppu.mode1_bg3_priority) return 11u;
    return mode1_priority[layer][high != 0u];
}

static uint16_t v20_direct_color(unsigned palette, uint8_t color)
{
    return (uint16_t)(((((color & 0x07u) << 1) | (palette & 0x01u)) << 1) |
                      (((color & 0x38u) | ((palette & 0x02u) << 1)) << 4) |
                      (((color & 0xC0u) | ((palette & 0x04u) << 3)) << 7));
}

static void v20_bg_pixel(const CivRecomp *i, unsigned layer,
                         unsigned x, unsigned y, CivV20Pixel *pixel)
{
    unsigned bpp;
    if (i->ppu.bg_mode == 3u) {
        if (layer >= 2u) { memset(pixel, 0, sizeof(*pixel)); pixel->transparent = 1u; return; }
        bpp = layer == 0u ? 8u : 4u;
    } else {
        bpp = layer == 2u ? 2u : 4u;
    }
    unsigned tile_size = i->ppu.bg_large_tiles[layer] ? 16u : 8u;
    unsigned map_width = i->ppu.bg_double_width[layer] ? 64u : 32u;
    unsigned map_height = i->ppu.bg_double_height[layer] ? 64u : 32u;
    unsigned world_width = map_width * tile_size;
    unsigned world_height = map_height * tile_size;
    unsigned wx = (x + i->ppu.bg_hscroll[layer]) % world_width;
    unsigned wy = (y + i->ppu.bg_vscroll[layer]) % world_height;
    unsigned tx = wx / tile_size;
    unsigned ty = wy / tile_size;
    unsigned px = wx % tile_size;
    unsigned py = wy % tile_size;
    unsigned screen = (ty >> 5) * (map_width == 64u ? 2u : 1u) + (tx >> 5);
    uint32_t map_byte = ((uint32_t)i->ppu.bg_tilemap_address[layer] + screen * 1024u +
                         (ty & 31u) * 32u + (tx & 31u)) * 2u;
    uint16_t entry = v20_vram_word(i, map_byte);
    unsigned tile = entry & 0x03FFu;
    unsigned palette = (entry >> 10) & 7u;
    uint8_t color;

    memset(pixel, 0, sizeof(*pixel));
    pixel->transparent = 1u;
    if (entry & 0x4000u) px = tile_size - 1u - px;
    if (entry & 0x8000u) py = tile_size - 1u - py;
    if (tile_size == 16u) {
        tile = (tile + (px >> 3) + ((py >> 3) << 4)) & 0x03FFu;
        px &= 7u;
        py &= 7u;
    }
    color = v20_tile_pixel(i, bpp, (uint32_t)i->ppu.bg_chr_address[layer] * 2u,
                           tile, py, px);
    if (!color) return;
    pixel->transparent = 0u;
    pixel->priority = v20_bg_priority(i, layer, (entry >> 13) & 1u);
    pixel->layer = (uint8_t)layer;
    if (bpp == 2u) pixel->color = v20_cgram_color(i, palette * 4u + color);
    else if (bpp == 4u) pixel->color = v20_cgram_color(i, palette * 16u + color);
    else if (i->ppu.direct_color_mode) pixel->color = v20_direct_color(palette, color);
    else pixel->color = v20_cgram_color(i, color);
}

static unsigned v20_sprite_width(unsigned mode, unsigned large)
{
    static const uint8_t small[8] = {8u, 8u, 8u, 16u, 16u, 32u, 16u, 16u};
    static const uint8_t big[8] = {16u, 32u, 64u, 32u, 64u, 64u, 32u, 32u};
    return large ? big[mode & 7u] : small[mode & 7u];
}

static unsigned v20_sprite_height(unsigned mode, unsigned large)
{
    static const uint8_t small[8] = {8u, 8u, 8u, 16u, 16u, 32u, 32u, 32u};
    static const uint8_t big[8] = {16u, 32u, 64u, 32u, 64u, 64u, 64u, 32u};
    return large ? big[mode & 7u] : small[mode & 7u];
}

static int v20_sprite_visible(unsigned y, int x, unsigned sprite_y,
                              unsigned width, unsigned height)
{
    unsigned line = (y + 256u - ((sprite_y + 1u) & 0xFFu)) & 0xFFu;
    if (x != -256 && (x + (int)width <= 0 || x > 255)) return 0;
    return line < height;
}

static void v20_sprite_line(const CivRecomp *i, unsigned y,
                            uint8_t palettes[256], uint8_t priorities[256])
{
    unsigned mode = i->ppu.oam_mode & 7u;
    unsigned first = i->ppu.oam_priority_rotation ? ((unsigned)i->ppu.oam_ram_address >> 1) & 127u : 0u;
    unsigned scanned;
    unsigned visible_count = 0u;
    unsigned tile_count = 0u;
    memset(palettes, 0, 256u);
    memset(priorities, 0, 256u);

    for (scanned = 0u; scanned < 128u && visible_count < 32u && tile_count < 34u; ++scanned) {
        unsigned index = (first + scanned) & 127u;
        uint8_t high = (uint8_t)((i->oam[0x200u + (index >> 2)] >> ((index & 3u) * 2u)) & 3u);
        int sx = (high & 1u) ? (int)i->oam[index * 4u] - 256 : (int)i->oam[index * 4u];
        unsigned sy = i->oam[index * 4u + 1u];
        uint8_t tile_name = i->oam[index * 4u + 2u];
        uint8_t attr = i->oam[index * 4u + 3u];
        unsigned width = v20_sprite_width(mode, (high >> 1) & 1u);
        unsigned height = v20_sprite_height(mode, (high >> 1) & 1u);
        unsigned source_y;
        unsigned columns;
        unsigned column;

        if (!v20_sprite_visible(y, sx, sy, width, height)) continue;
        ++visible_count;
        source_y = (y + 256u - ((sy + 1u) & 0xFFu)) & 0xFFu;
        if (attr & 0x80u) source_y = height - 1u - source_y;
        columns = width >> 3;
        for (column = 0u; column < columns && tile_count < 34u; ++column) {
            unsigned tile_column = (attr & 0x40u) ? columns - 1u - column : column;
            unsigned tile_row = (((unsigned)tile_name >> 4) + (source_y >> 3)) & 15u;
            unsigned tile_index = (tile_row << 4) | (((unsigned)tile_name + tile_column) & 15u);
            uint32_t chr_base = (uint32_t)i->ppu.oam_base_address * 2u;
            unsigned px;
            if (attr & 1u) chr_base += (uint32_t)(((i->ppu_regs[1] >> 3) & 3u) + 1u) << 13;
            ++tile_count;
            for (px = 0u; px < 8u; ++px) {
                int screen_x = sx + (int)(column * 8u + px);
                unsigned source_x = (attr & 0x40u) ? 7u - px : px;
                uint8_t color;
                if (screen_x < 0 || screen_x > 255 || palettes[screen_x]) continue;
                color = v20_tile_pixel(i, 4u, chr_base, tile_index, source_y & 7u, source_x);
                if (!color) continue;
                palettes[screen_x] = (uint8_t)(128u + (((attr >> 1) & 7u) << 4) + color);
                priorities[screen_x] = (uint8_t)((attr >> 4) & 3u);
            }
        }
    }
}

static uint8_t v20_window_nibble(const CivRecomp *i, unsigned layer)
{
    if (layer < 2u) return (uint8_t)((i->ppu_regs[0x23u] >> (layer * 4u)) & 15u);
    if (layer < 4u) return (uint8_t)((i->ppu_regs[0x24u] >> ((layer - 2u) * 4u)) & 15u);
    if (layer == 4u) return (uint8_t)(i->ppu_regs[0x25u] & 15u);
    return (uint8_t)(i->ppu_regs[0x25u] >> 4);
}

static unsigned v20_window_logic(const CivRecomp *i, unsigned layer)
{
    if (layer < 4u) return (i->ppu_regs[0x2Au] >> (layer * 2u)) & 3u;
    if (layer == 4u) return i->ppu_regs[0x2Bu] & 3u;
    return (i->ppu_regs[0x2Bu] >> 2) & 3u;
}

static int v20_inside_window(unsigned x, unsigned left, unsigned right)
{
    return left <= right && x >= left && x <= right;
}

static int v20_window_masked(const CivRecomp *i, unsigned layer, unsigned x)
{
    uint8_t nibble = v20_window_nibble(i, layer);
    int enabled1 = (nibble & 2u) != 0u;
    int enabled2 = (nibble & 8u) != 0u;
    int value1 = v20_inside_window(x, i->ppu_regs[0x26u], i->ppu_regs[0x27u]);
    int value2 = v20_inside_window(x, i->ppu_regs[0x28u], i->ppu_regs[0x29u]);
    unsigned logic;
    if (nibble & 1u) value1 = !value1;
    if (nibble & 4u) value2 = !value2;
    if (!enabled1) return enabled2 ? value2 : 0;
    if (!enabled2) return value1;
    logic = v20_window_logic(i, layer);
    if (logic == 0u) return value1 || value2;
    if (logic == 1u) return value1 && value2;
    if (logic == 2u) return value1 != value2;
    return value1 == value2;
}

static CivV20Pixel v20_screen_pixel(const CivRecomp *i, unsigned mask,
                                    unsigned window_enable, unsigned x, unsigned y,
                                    uint8_t obj_palette, uint8_t obj_priority)
{
    static const uint8_t mode1_obj_priority[4] = {2u, 4u, 7u, 10u};
    static const uint8_t mode3_obj_priority[4] = {2u, 4u, 6u, 8u};
    const uint8_t *obj_priority_map = i->ppu.bg_mode == 3u ? mode3_obj_priority : mode1_obj_priority;
    CivV20Pixel best;
    unsigned layer;
    memset(&best, 0, sizeof(best));
    best.color = v20_cgram_color(i, 0u);
    best.layer = 5u;
    for (layer = 0u; layer < (i->ppu.bg_mode == 3u ? 2u : 3u); ++layer) {
        CivV20Pixel candidate;
        if ((mask & (1u << layer)) == 0u) continue;
        if ((window_enable & (1u << layer)) && v20_window_masked(i, layer, x)) continue;
        v20_bg_pixel(i, layer, x, y, &candidate);
        if (!candidate.transparent && candidate.priority > best.priority) best = candidate;
    }
    if ((mask & 0x10u) && obj_palette &&
        !((window_enable & 0x10u) && v20_window_masked(i, 4u, x)) &&
        obj_priority_map[obj_priority & 3u] > best.priority) {
        best.color = v20_cgram_color(i, obj_palette);
        best.priority = obj_priority_map[obj_priority & 3u];
        best.layer = 4u;
        best.obj_palette = (uint8_t)((obj_palette - 128u) >> 4);
        best.transparent = 0u;
    }
    return best;
}

static int v20_math_applies(unsigned setting, int inside)
{
    if (setting == 0u) return 0;
    if (setting == 1u) return !inside;
    if (setting == 2u) return inside;
    return 1;
}

static uint16_t v20_color_add(uint16_t a, uint16_t b, int halve)
{
    unsigned r = (a & 31u) + (b & 31u);
    unsigned g = ((a >> 5) & 31u) + ((b >> 5) & 31u);
    unsigned blue = ((a >> 10) & 31u) + ((b >> 10) & 31u);
    if (halve) { r >>= 1; g >>= 1; blue >>= 1; }
    else { if (r > 31u) r = 31u; if (g > 31u) g = 31u; if (blue > 31u) blue = 31u; }
    return (uint16_t)(r | (g << 5) | (blue << 10));
}

static uint16_t v20_color_sub(uint16_t a, uint16_t b, int halve)
{
    int r = (int)(a & 31u) - (int)(b & 31u);
    int g = (int)((a >> 5) & 31u) - (int)((b >> 5) & 31u);
    int blue = (int)((a >> 10) & 31u) - (int)((b >> 10) & 31u);
    if (r < 0) r = 0;
    if (g < 0) g = 0;
    if (blue < 0) blue = 0;
    if (halve) { r >>= 1; g >>= 1; blue >>= 1; }
    return (uint16_t)((unsigned)r | ((unsigned)g << 5) | ((unsigned)blue << 10));
}

static uint32_t v20_rgba(uint16_t color, unsigned brightness)
{
    unsigned r = ((color & 31u) * brightness) / 15u;
    unsigned g = (((color >> 5) & 31u) * brightness) / 15u;
    unsigned b = (((color >> 10) & 31u) * brightness) / 15u;
    r = (r << 3) | (r >> 2); g = (g << 3) | (g >> 2); b = (b << 3) | (b >> 2);
    return UINT32_C(0xFF000000) | (r << 16) | (g << 8) | b;
}

static uint64_t v20_frame_hash(const uint8_t *data, size_t size)
{
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t n;
    for (n = 0u; n < size; ++n) { hash ^= data[n]; hash *= UINT64_C(1099511628211); }
    return hash;
}

int civ_render_current_frame(CivRecomp *i)
{
    unsigned y;
    CivPpuDecodedState saved_ppu;
    uint8_t saved_regs[CIV_PPU_WRITE_REG_COUNT];
    if (!i || ((i->ppu.bg_mode != 1u && i->ppu.bg_mode != 3u) && !i->ppu.forced_blank)) return 0;
    saved_ppu=i->ppu;
    memcpy(saved_regs,i->ppu_regs,sizeof(saved_regs));
    for (y = 0u; y < CIV_FRAME_HEIGHT; ++y) {
        uint8_t obj_palettes[256];
        uint8_t obj_priorities[256];
        unsigned x;
        if(i->scanline_state_valid[y]) {
            i->ppu=i->scanline_ppu[y];
            memcpy(i->ppu_regs,i->scanline_ppu_regs[y],CIV_PPU_WRITE_REG_COUNT);
        }
        v20_sprite_line(i, y, obj_palettes, obj_priorities);
        for (x = 0u; x < CIV_FRAME_WIDTH; ++x) {
            uint16_t color = 0u;
            if (!i->ppu.forced_blank) {
                CivV20Pixel main_pixel = v20_screen_pixel(i, i->ppu.main_screen_layers,
                                                          i->ppu.window_mask_main, x, y,
                                                          obj_palettes[x], obj_priorities[x]);
                uint8_t cgwsel = i->ppu_regs[0x30u];
                uint8_t cgadsub = i->ppu_regs[0x31u];
                int color_window = v20_window_masked(i, 5u, x);
                int clip = v20_math_applies((cgwsel >> 6) & 3u, color_window);
                int prevent = v20_math_applies((cgwsel >> 4) & 3u, color_window);
                int math = (cgadsub & (1u << main_pixel.layer)) != 0u;
                uint16_t second = i->ppu.fixed_color;
                color = clip ? 0u : main_pixel.color;
                if (cgwsel & 2u) {
                    CivV20Pixel sub_pixel = v20_screen_pixel(i, i->ppu.sub_screen_layers,
                                                             i->ppu.window_mask_sub, x, y,
                                                             obj_palettes[x], obj_priorities[x]);
                    second = sub_pixel.color;
                }
                if (main_pixel.layer == 4u && main_pixel.obj_palette < 4u) math = 0;
                if (prevent) math = 0;
                if (math) color = (cgadsub & 0x80u) ?
                    v20_color_sub(color, second, (cgadsub & 0x40u) != 0u) :
                    v20_color_add(color, second, (cgadsub & 0x40u) != 0u);
            }
            i->v19_framebuffer_rgba[y * CIV_FRAME_WIDTH + x] =
                v20_rgba(color, i->ppu.brightness);
        }
    }
    i->ppu=saved_ppu;
    memcpy(i->ppu_regs,saved_regs,sizeof(saved_regs));
    i->v19_framebuffer_fnv1a64 = v20_frame_hash((const uint8_t *)i->v19_framebuffer_rgba,
                                                 sizeof(i->v19_framebuffer_rgba));
    i->v19_framebuffer_ready = 1u;
    return 1;
}

const uint32_t *civ_get_framebuffer_rgba(const CivRecomp *i)
{
    return (i && i->v19_framebuffer_ready)?i->v19_framebuffer_rgba:NULL;
}

int civ_v20_render_current_frame(CivRecomp *i)
{
    return civ_render_current_frame(i);
}
