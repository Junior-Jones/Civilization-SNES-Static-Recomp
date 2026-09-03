#include "civilization_internal.h"

enum {
    CIV_PAD_B=0x0001u,CIV_PAD_Y=0x0002u,CIV_PAD_SELECT=0x0004u,
    CIV_PAD_UP=0x0010u,CIV_PAD_DOWN=0x0020u,
    CIV_PAD_LEFT=0x0040u,CIV_PAD_RIGHT=0x0080u,CIV_PAD_A=0x0100u,
    CIV_PAD_X=0x0200u,CIV_PAD_L=0x0400u,CIV_PAD_R=0x0800u
};

static const uint16_t widescreen_rebase_offsets[9]={
    0x06EEu,0x06F0u,0x06F2u,0x06F4u,0x06F6u,
    0x06F8u,0x06FAu,0x06FCu,0x06FEu
};

static uint16_t widescreen_word(const CivRecomp *i,uint16_t offset)
{return (uint16_t)(i->wram[offset]|((uint16_t)i->wram[offset+1u]<<8));}

static void widescreen_set_word(CivRecomp *i,uint16_t offset,uint16_t value)
{i->wram[offset]=(uint8_t)value;i->wram[offset+1u]=(uint8_t)(value>>8);}

static int widescreen_cursor_x(const CivRecomp *i)
{
    unsigned high=(i->oam[0x200u]&1u);
    return high?(int)i->oam[0]-256:(int)i->oam[0];
}

static int widescreen_cursor_y(const CivRecomp *i)
{return i?(int)i->oam[1]:0;}

static int widescreen_map_focus(const CivRecomp *i)
{
    return i&&i->widescreen_enabled&&civ_widescreen_live_map_state(i)&&
           i->oam[2]==0xEEu&&i->oam[1]>=16u&&i->oam[1]<208u;
}

static void widescreen_restore_rebase(CivRecomp *i)
{
    unsigned n;
    if(!i||!i->widescreen_input_rebased)return;
    for(n=0u;n<9u;++n)
        widescreen_set_word(i,widescreen_rebase_offsets[n],
                            i->widescreen_rebase_words[n]);
    i->widescreen_input_rebased=0u;
}

static int widescreen_rebase_delta(unsigned index,int tiles_x,int tiles_y)
{
    if(index==0u||index==5u||index==7u)return tiles_x;
    if(index==1u||index==6u||index==8u)return tiles_y;
    return tiles_x+80*tiles_y;
}

static void widescreen_apply_rebase(CivRecomp *i)
{
    int tiles_x=i->widescreen_cursor_extension_x/16;
    int tiles_y=i->widescreen_cursor_extension_y/16;
    unsigned n;
    if(!i||(tiles_x==0&&tiles_y==0))return;
    for(n=0u;n<9u;++n) {
        uint16_t value=widescreen_word(i,widescreen_rebase_offsets[n]);
        i->widescreen_rebase_words[n]=value;
        widescreen_set_word(i,widescreen_rebase_offsets[n],
            (uint16_t)(value+widescreen_rebase_delta(n,tiles_x,tiles_y)));
    }
    i->widescreen_input_rebased=1u;
}

static void widescreen_commit_rebase(CivRecomp *i)
{
    int tiles_x=i->widescreen_cursor_extension_x/16;
    int tiles_y=i->widescreen_cursor_extension_y/16;
    unsigned n;
    if(!i||(tiles_x==0&&tiles_y==0))return;
    for(n=0u;n<9u;++n) {
        uint16_t value=widescreen_word(i,widescreen_rebase_offsets[n]);
        widescreen_set_word(i,widescreen_rebase_offsets[n],
            (uint16_t)(value+widescreen_rebase_delta(n,tiles_x,tiles_y)));
    }
    i->widescreen_cursor_extension_x=0;
    i->widescreen_cursor_extension_y=0;
    i->widescreen_clear_after_release=0u;
}

static int widescreen_vertical_target_valid(const CivRecomp *i,int extension_y)
{
    int camera_y=(int)widescreen_word(i,0x06FEu);
    int cursor_tile=(widescreen_cursor_y(i)-19)/16;
    int world_y=camera_y+cursor_tile+extension_y/16;
    return world_y>=0&&world_y<40;
}

static uint16_t widescreen_connect_input(CivRecomp *i,uint16_t input)
{
    uint16_t edge=(uint16_t)(input&~i->widescreen_previous_input);
    uint16_t output=input;
    int x,y,step=(input&CIV_PAD_X)?32:16;
    widescreen_restore_rebase(i);
    if(input==0u&&i->widescreen_clear_after_release) {
        i->widescreen_cursor_extension_x=0;
        i->widescreen_cursor_extension_y=0;
        i->widescreen_clear_after_release=0u;
    }
    if(!widescreen_map_focus(i)) {
        i->widescreen_cursor_extension_x=0;
        i->widescreen_cursor_extension_y=0;
        i->widescreen_consumed_direction=0u;
        i->widescreen_clear_after_release=0u;
        i->widescreen_previous_input=input;
        return input;
    }
    x=widescreen_cursor_x(i);
    y=widescreen_cursor_y(i);
    i->widescreen_consumed_direction=(uint16_t)(
        i->widescreen_consumed_direction&input&
        (CIV_PAD_LEFT|CIV_PAD_RIGHT|CIV_PAD_UP|CIV_PAD_DOWN));
    if(!(input&(CIV_PAD_L|CIV_PAD_R|CIV_PAD_A))) {
        if((edge&CIV_PAD_LEFT)&&i->widescreen_cursor_extension_x<=0&&
           (i->widescreen_cursor_extension_x<0||x<=36)) {
            if(i->widescreen_cursor_extension_x>-64) {
                i->widescreen_cursor_extension_x=(int16_t)(
                    i->widescreen_cursor_extension_x-step);
                if(i->widescreen_cursor_extension_x<-64)
                    i->widescreen_cursor_extension_x=-64;
            }
            i->widescreen_consumed_direction|=CIV_PAD_LEFT;
        } else if((edge&CIV_PAD_RIGHT)&&i->widescreen_cursor_extension_x<0) {
            i->widescreen_cursor_extension_x=(int16_t)(
                i->widescreen_cursor_extension_x+step);
            if(i->widescreen_cursor_extension_x>0)
                i->widescreen_cursor_extension_x=0;
            i->widescreen_consumed_direction|=CIV_PAD_RIGHT;
        } else if((edge&CIV_PAD_RIGHT)&&i->widescreen_cursor_extension_x>=0&&
                  (i->widescreen_cursor_extension_x>0||x>=212)) {
            if(i->widescreen_cursor_extension_x<64) {
                i->widescreen_cursor_extension_x=(int16_t)(
                    i->widescreen_cursor_extension_x+step);
                if(i->widescreen_cursor_extension_x>64)
                    i->widescreen_cursor_extension_x=64;
            }
            i->widescreen_consumed_direction|=CIV_PAD_RIGHT;
        } else if((edge&CIV_PAD_LEFT)&&i->widescreen_cursor_extension_x>0) {
            i->widescreen_cursor_extension_x=(int16_t)(
                i->widescreen_cursor_extension_x-step);
            if(i->widescreen_cursor_extension_x<0)
                i->widescreen_cursor_extension_x=0;
            i->widescreen_consumed_direction|=CIV_PAD_LEFT;
        }
        if((edge&CIV_PAD_UP)&&i->widescreen_cursor_extension_y<=0&&
           (i->widescreen_cursor_extension_y<0||y<=19)) {
            if(i->widescreen_cursor_extension_y==0&&
               widescreen_vertical_target_valid(i,-16))
                i->widescreen_cursor_extension_y=-16;
            i->widescreen_consumed_direction|=CIV_PAD_UP;
        } else if((edge&CIV_PAD_DOWN)&&i->widescreen_cursor_extension_y<0) {
            i->widescreen_cursor_extension_y=0;
            i->widescreen_consumed_direction|=CIV_PAD_DOWN;
        } else if((edge&CIV_PAD_DOWN)&&i->widescreen_cursor_extension_y>=0&&
                  (i->widescreen_cursor_extension_y>0||y>=195)) {
            if(i->widescreen_cursor_extension_y==0&&
               widescreen_vertical_target_valid(i,16))
                i->widescreen_cursor_extension_y=16;
            i->widescreen_consumed_direction|=CIV_PAD_DOWN;
        } else if((edge&CIV_PAD_UP)&&i->widescreen_cursor_extension_y>0) {
            i->widescreen_cursor_extension_y=0;
            i->widescreen_consumed_direction|=CIV_PAD_UP;
        }
    }
    output=(uint16_t)(output&~i->widescreen_consumed_direction);
    if((i->widescreen_cursor_extension_x!=0||
        i->widescreen_cursor_extension_y!=0)&&
       (input&(CIV_PAD_A|CIV_PAD_B|CIV_PAD_Y|CIV_PAD_SELECT))) {
        if(input&(CIV_PAD_Y|CIV_PAD_SELECT))
            widescreen_commit_rebase(i);
        else {
            widescreen_apply_rebase(i);
            i->widescreen_clear_after_release=1u;
        }
    }
    i->widescreen_previous_input=input;
    return output;
}

static uint8_t mouse_scaled_axis(uint8_t sensitivity,int16_t raw)
{
    static const uint8_t lut[2][8]={{0u,1u,2u,3u,8u,10u,12u,21u},{0u,1u,4u,9u,12u,20u,24u,28u}};
    unsigned magnitude=(unsigned)(raw<0 ? -(int32_t)raw : (int32_t)raw);
    if(magnitude>127u)magnitude=127u;
    if(sensitivity>0u)magnitude=lut[(sensitivity-1u)%2u][magnitude>7u?7u:magnitude];
    return (uint8_t)magnitude;
}

static void mouse_refresh_packet(CivRecomp *i,unsigned controller)
{
    uint8_t byte2,byte3,byte4;
    if(controller>=2u)return;
    if(i->mouse_dx[controller]!=0)i->mouse_left_flag[controller]=(uint8_t)(i->mouse_dx[controller]<0?0x80u:0u);
    if(i->mouse_dy[controller]!=0)i->mouse_up_flag[controller]=(uint8_t)(i->mouse_dy[controller]<0?0x80u:0u);
    byte2=(uint8_t)(0x01u|((i->mouse_sensitivity[controller]&3u)<<4));
    if(i->mouse_buttons[controller]&CIV_MOUSE_BUTTON_LEFT)byte2|=0x40u;
    if(i->mouse_buttons[controller]&CIV_MOUSE_BUTTON_RIGHT)byte2|=0x80u;
    byte3=(uint8_t)(mouse_scaled_axis(i->mouse_sensitivity[controller],i->mouse_dy[controller])|i->mouse_up_flag[controller]);
    byte4=(uint8_t)(mouse_scaled_axis(i->mouse_sensitivity[controller],i->mouse_dx[controller])|i->mouse_left_flag[controller]);
    i->mouse_shift_packet[controller]=((uint32_t)byte2<<16)|((uint32_t)byte3<<8)|byte4;
}

void civ_controller_latch(CivRecomp *i)
{
    unsigned controller;
    for(controller=0u;controller<2u;controller++) {
        i->controller_shift_index[controller]=0u;
        if(i->controller_device[controller]==CIV_INPUT_DEVICE_MOUSE)mouse_refresh_packet(i,controller);
        else i->controller_latched[controller]=i->controller_live[controller];
    }
}

uint8_t civ_controller_serial_read(CivRecomp *i,unsigned controller)
{
    uint8_t bit;
    if(controller>=2u)return 1u;
    if(i->controller_device[controller]==CIV_INPUT_DEVICE_MOUSE) {
        if(i->controller_strobe){i->mouse_sensitivity[controller]=(uint8_t)((i->mouse_sensitivity[controller]+1u)%3u);mouse_refresh_packet(i,controller);}
        bit=(uint8_t)((i->mouse_shift_packet[controller]>>31)&1u);
        i->mouse_shift_packet[controller]=(i->mouse_shift_packet[controller]<<1)|1u;
        if(i->controller_shift_index[controller]<0xFFu)i->controller_shift_index[controller]++;
    } else if(i->controller_strobe)bit=(uint8_t)(i->controller_live[controller]&1u);
    else {uint8_t index=i->controller_shift_index[controller];bit=(index<16u)?(uint8_t)((i->controller_latched[controller]>>index)&1u):1u;if(index<0xFFu)i->controller_shift_index[controller]=(uint8_t)(index+1u);}
    i->controller_serial_read_count[controller]++;
    return bit;
}

static uint8_t controller_auto_read_bit(CivRecomp *i,unsigned controller)
{
    uint8_t bit;
    if(controller>=2u)return 1u;
    if(i->controller_device[controller]==CIV_INPUT_DEVICE_MOUSE){bit=(uint8_t)((i->mouse_shift_packet[controller]>>31)&1u);i->mouse_shift_packet[controller]=(i->mouse_shift_packet[controller]<<1)|1u;}
    else {uint8_t index=i->controller_shift_index[controller];bit=(index<16u)?(uint8_t)((i->controller_latched[controller]>>index)&1u):1u;}
    if(i->controller_shift_index[controller]<0xFFu)i->controller_shift_index[controller]++;
    i->auto_joypad_serial_read_count[controller]++;
    return bit;
}

void civ_set_controller_input(CivRecomp *i,unsigned controller,uint16_t serial_mask)
{if(i&&controller<2u){i->controller_device[controller]=CIV_INPUT_DEVICE_JOYPAD;if(controller==0u&&i->widescreen_enabled)serial_mask=widescreen_connect_input(i,serial_mask);i->controller_live[controller]=serial_mask;}}

void civ_set_mouse_input(CivRecomp *i,unsigned controller,int16_t dx,int16_t dy,uint8_t buttons)
{if(i&&controller<2u){i->controller_device[controller]=CIV_INPUT_DEVICE_MOUSE;i->mouse_dx[controller]=dx;i->mouse_dy[controller]=dy;i->mouse_buttons[controller]=(uint8_t)(buttons&(CIV_MOUSE_BUTTON_LEFT|CIV_MOUSE_BUTTON_RIGHT));}}

int civ_auto_joypad_poll(CivRecomp *i)
{
    unsigned bit,controller;
    if(!i||!i->auto_joypad_enable)return 0;
    i->auto_joypad_busy=1u;
    for(controller=0u;controller<4u;controller++)i->auto_joypad_data[controller]=0u;
    civ_controller_latch(i);
    for(bit=0u;bit<16u;bit++)for(controller=0u;controller<2u;controller++)i->auto_joypad_data[controller]=(uint16_t)((i->auto_joypad_data[controller]<<1)|(controller_auto_read_bit(i,controller)&1u));
    i->auto_joypad_busy=0u;i->auto_joypad_poll_count++;
    return 1;
}
