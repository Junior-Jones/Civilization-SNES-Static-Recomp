#include "civilization_internal.h"
#include "civilization_generated_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int load_file(const char *path,uint8_t **data,size_t *size)
{
    FILE *f=fopen(path,"rb"); long n;
    if(!f)return 0;
    if(fseek(f,0,SEEK_END)!=0){fclose(f);return 0;} n=ftell(f);
    if(n<0||fseek(f,0,SEEK_SET)!=0){fclose(f);return 0;}
    *data=(uint8_t*)malloc((size_t)n); if(!*data){fclose(f);return 0;}
    if(fread(*data,1,(size_t)n,f)!=(size_t)n){free(*data);fclose(f);return 0;}
    fclose(f); *size=(size_t)n; return 1;
}

int main(int argc,char **argv)
{
    static const uint8_t route_bytes[]={
        0x8D,0x1B,0x21, 0x9C,0x1B,0x21, 0xA9,0x0D,
        0x8D,0x1C,0x21, 0xAD,0x26,0x19, 0x4A,0x4A,
        0x4A,0x4A,0x3A,0x3A,0x18, 0x6D,0x34,0x21,
        0xC2,0x20, 0x8D,0x05,0x01
    };
    CivRecomp c = {0}; uint8_t *rom=NULL; size_t rom_size=0; char err[256]; unsigned steps=0;
    if(argc!=2||!load_file(argv[1],&rom,&rom_size))return 2;
    if(rom_size!=CIV_ROM_SIZE||memcmp(rom+0x24353u,route_bytes,sizeof(route_bytes))!=0){free(rom);return 3;}
    civ_reset(&c);
    if(!civ_attach_verified_rom(&c,rom,rom_size,err,sizeof(err))){free(rom);return 4;}

    /* Exact closed context C2:4353 E0M1X0.  Choose A=5 and $1926=$90 so
       the ROM sequence computes (($90>>4)-2) + (5*13) = 7+65 = 72. */
    c.cpu.pbr=0xC2u; c.cpu.pc=0x4353u; c.cpu.dbr=0u; c.cpu.e=0u;
    c.cpu.p=(uint8_t)((c.cpu.p|CIV_P_M)&(uint8_t)~CIV_P_X);
    c.cpu.a=0x0005u; c.cpu.x=0u; c.wram[0x1926u]=0x90u;
    while(!c.failed && !(c.cpu.pbr==0xC2u&&c.cpu.pc==0x4370u) && steps<32u) {
        if(!civ_generated_core_step(&c)){free(rom);return 5;}
        steps++;
    }
    if(c.failed||c.cpu.pc!=0x4370u||steps!=15u){free(rom);return 6;}
    if(c.ppu.mode7_matrix[0]!=0x0005u||c.ppu.mode7_matrix[1]!=0x0D00u){free(rom);return 7;}
    if(c.ppu_multiply_result_read_count[0]!=1u||c.ppu_multiply_result_read_count[1]!=0u||c.ppu_multiply_result_read_count[2]!=0u){free(rom);return 8;}
    if(c.wram[0x0105u]!=0x48u||c.wram[0x0106u]!=0x00u){free(rom);return 9;}
    free(rom);
    puts("v33 exact-ROM C2:4353-C2:436D multiply route PASS");
    return 0;
}
