#include "civilization_internal.h"

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
{if(i&&controller<2u){i->controller_device[controller]=CIV_INPUT_DEVICE_JOYPAD;i->controller_live[controller]=serial_mask;}}

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
