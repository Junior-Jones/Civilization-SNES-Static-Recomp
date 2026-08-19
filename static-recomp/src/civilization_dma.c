#include "civilization_internal.h"

static int civ_dma_process_nested_hdma(CivRecomp *i,uint8_t cpu_speed,
                                       uint32_t *dma_clock_counter);

static int civ_dma_channel_run(CivRecomp *i,CivDmaChannel0 *channel,unsigned channel_index,
                               uint8_t cpu_speed,uint32_t *dma_clock_counter) {
    static const uint8_t transfer_offsets[8][4]={
        {0u,0u,0u,0u},{0u,1u,0u,1u},{0u,0u,0u,0u},{0u,0u,1u,1u},
        {0u,1u,2u,3u},{0u,1u,0u,1u},{0u,0u,0u,0u},{0u,0u,1u,1u}
    };
    uint32_t remaining,n;
    uint16_t source;
    uint8_t mode,fixed,decrement;
    if(!i||!channel||!dma_clock_counter||channel_index>=8u)return 0;
    mode=(uint8_t)(channel->dmap&7u);
    fixed=(uint8_t)((channel->dmap>>3)&1u);
    decrement=(uint8_t)((channel->dmap>>4)&1u);
    if(channel->dmap&0x80u)
        return civ_fail_frontier(i,"Reached DMA implementation supports only A-bus to B-bus direction.",NULL);
    /* Civilization has reached modes 0/1/2 on channels 0/1 and mode 1 on
       channel 7.
       V19 first-fade NMI proves channel-0 mode 2 as a 512-byte CGRAM upload
       ($4300=$02, BBAD=$22, source 7E:2220).  Other modes remain fail-closed. */
    if((channel_index==0u && mode>2u) || (channel_index==1u && mode>2u) ||
       (channel_index==7u && mode!=1u) ||
       (channel_index!=0u && channel_index!=1u && channel_index!=7u))
        return civ_fail_frontier(i,"DMA transfer mode reached outside the target-proved channel/mode set.",NULL);
    remaining=channel->transfer_size ? (uint32_t)channel->transfer_size : 65536u;
    source=channel->source_address;
    for(n=0u;n<remaining;n++) {
        uint8_t data;
        uint8_t boff=(uint8_t)(channel->bbad+transfer_offsets[mode][n&3u]);
        uint32_t a=((uint32_t)channel->source_bank<<16)|source;
        uint32_t b=0x002100u|(uint32_t)boff;
        int transfer_ok;
        i->cpu_bus_timing_suppressed=1u;
        transfer_ok=civ_bus_read8(i,a,&data) && civ_bus_write8(i,b,data);
        i->cpu_bus_timing_suppressed=0u;
        if(!transfer_ok)return 0;
        if(!fixed)source=(uint16_t)(source+(decrement?(uint16_t)0xFFFFu:(uint16_t)1u));
        i->dma_transfer_byte_count++;
        i->dma_channel_transfer_byte_count[channel_index]++;
        /* One manual-DMA byte occupies eight master clocks.  SNES DMA/HDMA
           arbitration is checked after every byte so a frame/scanline HDMA
           request raised during a long transfer can run before the next byte. */
        *dma_clock_counter+=8u;
        civ_timing_advance_master(i,8u);
        if(!civ_dma_process_nested_hdma(i,cpu_speed,dma_clock_counter))return 0;
    }
    channel->source_address=source;
    channel->transfer_size=0u;
    i->dma_run_count++;
    i->dma_channel_run_count[channel_index]++;
    return 1;
}

int civ_hdma2_config_is_proved(const CivRecomp *i)
{
    if(!i)return 0;
    /* Exact ROM C1:996E-C1:998E programs one and only one non-zero HDMA
       configuration before copying $1B80 to the NMI restore byte $0006:
         $4320=$42  indirect, mode 2, A->B
         $4321=$0D  BG1HOFS ($210D)
         $4322/3=$4AE3, $4324=$C1, $4327=$00.
       Keep the hardware implementation target-specific and fail closed if a
       different configuration ever reaches HDMAEN. */
    return i->dma2.dmap==0x42u && i->dma2.bbad==0x0Du &&
           i->dma2.source_address==0x4AE3u && i->dma2.source_bank==0xC1u &&
           i->dma2.indirect_bank==0x00u;
}

static int civ_hdma2_read(CivRecomp *i,uint32_t address,uint8_t *value)
{
    int ok;
    if(!i||!value)return 0;
    i->cpu_bus_timing_suppressed=1u;
    ok=civ_bus_read8(i,address,value);
    i->cpu_bus_timing_suppressed=0u;
    if(!ok)return 0;
    i->hdma_table_read_count++;
    civ_timing_advance_master(i,8u);
    return !i->failed;
}

static int civ_hdma2_copy(CivRecomp *i,uint32_t source,uint16_t ppu_local)
{
    uint8_t value;
    int ok;
    if(!i)return 0;
    i->cpu_bus_timing_suppressed=1u;
    ok=civ_bus_read8(i,source,&value) && civ_bus_write8(i,(uint32_t)ppu_local,value);
    i->cpu_bus_timing_suppressed=0u;
    if(!ok)return 0;
    i->hdma_transfer_byte_count++;
    i->dma_transfer_byte_count++;
    i->dma_channel_transfer_byte_count[2]++;
    civ_timing_advance_master(i,8u);
    return !i->failed;
}

static int civ_hdma2_sync_start(CivRecomp *i,uint32_t *clock_counter)
{
    uint32_t start_sync;
    if(!i||!clock_counter)return 0;
    start_sync=(uint32_t)(8u-(uint32_t)(i->master_clock&7u));
    *clock_counter=start_sync;
    civ_timing_advance_master(i,start_sync);
    return !i->failed;
}

static int civ_hdma2_sync_end(CivRecomp *i,uint8_t cpu_speed,uint32_t clock_counter)
{
    uint32_t end_sync;
    if(!i)return 0;
    if(cpu_speed!=6u && cpu_speed!=8u && cpu_speed!=12u)
        return civ_fail_frontier(i,"HDMA resumed at an unproved CPU bus speed.",NULL);
    end_sync=(uint32_t)cpu_speed-(clock_counter%(uint32_t)cpu_speed);
    civ_timing_advance_master(i,end_sync);
    return !i->failed;
}

static int civ_hdma2_finish_timing(CivRecomp *i,uint8_t cpu_speed,
                                    uint32_t clocks,uint8_t nested_manual_dma,
                                    uint32_t *manual_dma_clock_counter)
{
    if(!i)return 0;
    if(nested_manual_dma) {
        if(!manual_dma_clock_counter)
            return civ_fail_frontier(i,"Nested HDMA lost the active manual-DMA clock counter.",NULL);
        *manual_dma_clock_counter+=clocks;
        return !i->failed;
    }
    return civ_hdma2_sync_end(i,cpu_speed,clocks);
}

static int civ_hdma2_init_impl(CivRecomp *i,uint8_t cpu_speed,
                               uint8_t nested_manual_dma,
                               uint32_t *manual_dma_clock_counter)
{
    uint32_t clocks=0u;
    uint8_t lo,hi;
    if(!i)return 0;
    i->hdma2.finished=0u;
    i->hdma2.do_transfer=0u;
    i->hdma2.initialized=0u;
    if((i->cpu_io[0x0Cu]&0x04u)==0u)return 1;
    if(i->cpu_io[0x0Cu]!=0x04u || !civ_hdma2_config_is_proved(i))
        return civ_fail_frontier(i,"HDMA init reached outside the source-proved Civilization channel-2 configuration.",NULL);
    if(!nested_manual_dma && !civ_hdma2_sync_start(i,&clocks))return 0;
    civ_timing_advance_master(i,8u); clocks+=8u; /* global HDMA overhead */
    i->hdma2.table_address=i->dma2.source_address;
    i->hdma2.do_transfer=1u;
    if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&i->hdma2.line_counter_repeat))return 0;
    clocks+=8u; i->hdma2.table_address++;
    if(i->hdma2.line_counter_repeat==0u)i->hdma2.finished=1u;
    /* DMAP2=$42 proves indirect addressing.  The SNES loads one low byte even
       for an immediately terminated descriptor, and the high byte only while
       the channel remains live. */
    if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&lo))return 0;
    clocks+=8u; i->hdma2.table_address++;
    if(!i->hdma2.finished) {
        if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&hi))return 0;
        clocks+=8u; i->hdma2.table_address++;
        i->dma2.transfer_size=(uint16_t)(lo|((uint16_t)hi<<8));
    } else {
        i->dma2.transfer_size=(uint16_t)((uint16_t)lo<<8);
    }
    i->hdma2.initialized=1u;
    i->hdma_init_count++;
    if(!civ_hdma2_finish_timing(i,cpu_speed,clocks,nested_manual_dma,manual_dma_clock_counter))return 0;
    i->dma_run_count++;
    i->dma_channel_run_count[2]++;
    return 1;
}

static int civ_hdma2_init(CivRecomp *i,uint8_t cpu_speed)
{
    return civ_hdma2_init_impl(i,cpu_speed,0u,NULL);
}

static int civ_hdma2_scanline_impl(CivRecomp *i,uint8_t cpu_speed,
                                   uint8_t nested_manual_dma,
                                   uint32_t *manual_dma_clock_counter)
{
    uint32_t clocks=0u;
    uint8_t new_counter,lo,hi;
    unsigned n;
    static const uint8_t mode2_offsets[2]={0u,0u};
    if(!i)return 0;
    if((i->cpu_io[0x0Cu]&0x04u)==0u)return 1;
    if(i->cpu_io[0x0Cu]!=0x04u || !civ_hdma2_config_is_proved(i))
        return civ_fail_frontier(i,"HDMA scanline reached outside the source-proved Civilization channel-2 configuration.",NULL);
    if(!i->hdma2.initialized)
        return civ_fail_frontier(i,"HDMA channel 2 became active before frame initialization.",NULL);
    if(i->hdma2.finished)return 1;
    if(!nested_manual_dma && !civ_hdma2_sync_start(i,&clocks))return 0;
    civ_timing_advance_master(i,8u); clocks+=8u; /* per-scanline HDMA overhead */
    if(i->hdma2.do_transfer) {
        /* Exact DMAP2=$42 is mode 2: two bytes to the same B-bus register.
           Indirect bank is source-proved $00; the live pointer is WRAM data. */
        for(n=0u;n<2u;n++) {
            uint32_t src=((uint32_t)i->dma2.indirect_bank<<16)|i->dma2.transfer_size;
            uint16_t dst=(uint16_t)(0x2100u+i->dma2.bbad+mode2_offsets[n]);
            if(!civ_hdma2_copy(i,src,dst))return 0;
            i->dma2.transfer_size++;
            clocks+=8u;
        }
    }
    i->hdma2.line_counter_repeat--;
    i->hdma2.do_transfer=(uint8_t)((i->hdma2.line_counter_repeat&0x80u)!=0u);
    /* Hardware always performs the speculative next-counter read. */
    if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&new_counter))return 0;
    clocks+=8u;
    if((i->hdma2.line_counter_repeat&0x7Fu)==0u) {
        i->hdma2.line_counter_repeat=new_counter;
        i->hdma2.table_address++;
        if(i->hdma2.line_counter_repeat==0u) {
            /* Channel 2 is the only source-proved active HDMA channel, so the
               SNES last-active-channel zero-counter quirk loads one byte as
               the high byte of the indirect pointer. */
            if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&hi))return 0;
            clocks+=8u; i->hdma2.table_address++;
            i->dma2.transfer_size=(uint16_t)((uint16_t)hi<<8);
            i->hdma2.finished=1u;
        } else {
            if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&lo))return 0;
            clocks+=8u; i->hdma2.table_address++;
            if(!civ_hdma2_read(i,((uint32_t)i->dma2.source_bank<<16)|i->hdma2.table_address,&hi))return 0;
            clocks+=8u; i->hdma2.table_address++;
            i->dma2.transfer_size=(uint16_t)(lo|((uint16_t)hi<<8));
        }
        i->hdma2.do_transfer=1u;
    }
    i->hdma_scanline_count++;
    if(!civ_hdma2_finish_timing(i,cpu_speed,clocks,nested_manual_dma,manual_dma_clock_counter))return 0;
    i->dma_run_count++;
    i->dma_channel_run_count[2]++;
    return 1;
}

static int civ_hdma2_scanline(CivRecomp *i,uint8_t cpu_speed)
{
    return civ_hdma2_scanline_impl(i,cpu_speed,0u,NULL);
}

/* While manual DMA is active the SNES checks the same pending HDMA/start-delay
   latch after DMA startup and after every transferred byte.  HDMA that runs at
   those checkpoints shares the already-active DMA clock domain: there is no
   second SyncStart/SyncEnd pair, and its clocks contribute to the outer DMA
   counter used for the final CPU-cycle resynchronization.  Civilization keeps
   this helper target-specific: only the already-proved channel-2 HDMA route can
   execute here. */
static int civ_dma_process_nested_hdma(CivRecomp *i,uint8_t cpu_speed,
                                       uint32_t *dma_clock_counter)
{
    if(!i||!dma_clock_counter||i->failed)return 0;
    if(!i->dma_start_delay && !i->hdma_init_pending && !i->hdma_transfer_pending)return 1;
    if(i->dma_start_delay) {
        i->dma_start_delay=0u;
        return 1;
    }
    if(i->hdma_transfer_pending) {
        i->hdma_transfer_pending=0u;
        if(i->cpu_io[0x0Cu]!=0u &&
           !civ_hdma2_scanline_impl(i,cpu_speed,1u,dma_clock_counter))return 0;
    } else if(i->hdma_init_pending) {
        i->hdma_init_pending=0u;
        if(i->cpu_io[0x0Cu]!=0u) {
            if(!civ_hdma2_init_impl(i,cpu_speed,1u,dma_clock_counter))return 0;
        } else {
            i->hdma2.finished=0u;
            i->hdma2.do_transfer=0u;
            i->hdma2.initialized=0u;
        }
    }
    return !i->failed;
}

void civ_hdma_schedule_init(CivRecomp *i)
{
    if(!i||!i->natural_timing_enabled||i->failed)return;
    if(i->hdma_init_pending)
        (void)civ_fail_frontier(i,"A second HDMA initialization event was scheduled before the first was serviced.",NULL);
    else {
        i->hdma_init_pending=1u;
        i->dma_start_delay=1u;
    }
}

void civ_hdma_schedule_scanline(CivRecomp *i)
{
    if(!i||!i->natural_timing_enabled||i->failed||i->cpu_io[0x0Cu]==0u)return;
    if(i->cpu_io[0x0Cu]!=0x04u) {
        (void)civ_fail_frontier(i,"HDMA scheduler observed an unproved enabled-channel mask.",NULL);
        return;
    }
    if(i->hdma_transfer_pending) {
        (void)civ_fail_frontier(i,"A second HDMA scanline event was scheduled before the first was serviced.",NULL);
        return;
    }
    i->hdma_transfer_pending=1u;
    i->dma_start_delay=1u;
}

/* Manual-DMA arbitration follows the certified SNES timing order: MDMAEN
   arms a transfer with a one-CPU-cycle start
   delay; the following CPU cycle synchronizes to an 8-master boundary, pays
   eight clocks of global DMA startup, eight clocks per active channel, eight
   clocks per byte, then synchronizes back to the interrupted CPU cycle speed.
   DMA remains target-restricted/fail-closed to the proved Civilization channel
   masks and transfer modes below. */
static int civ_dma_execute_pending(CivRecomp *i,uint8_t cpu_speed)
{
    uint8_t mask;
    uint32_t dma_clock_counter,start_sync,end_sync;
    CivDmaChannel0 *channel;
    unsigned channel_index;
    if(!i || i->dma_pending_mask==0u)return 1;
    mask=i->dma_pending_mask;
    i->dma_pending_mask=0u;

    if(mask==1u) { channel=&i->dma0; channel_index=0u; }
    else if(mask==2u) { channel=&i->dma1; channel_index=1u; }
    else if(mask==0x80u) { channel=&i->dma7; channel_index=7u; }
    else return civ_fail_frontier(i,"Simultaneous or unproved DMA-channel enable mask reached.",NULL);

    start_sync=(uint32_t)(8u-(uint32_t)(i->master_clock&7u));
    dma_clock_counter=start_sync;
    civ_timing_advance_master(i,start_sync);

    /* Global manual-DMA startup overhead, followed by the same pending-transfer
       arbitration point used by the pinned SNES DMA controller. */
    dma_clock_counter+=8u;
    civ_timing_advance_master(i,8u);
    if(!civ_dma_process_nested_hdma(i,cpu_speed,&dma_clock_counter))return 0;

    /* One channel is source-proved active for every Civilization MDMAEN write.
       Pay its eight-clock startup and check HDMA again before the first byte. */
    dma_clock_counter+=8u;
    civ_timing_advance_master(i,8u);
    if(!civ_dma_process_nested_hdma(i,cpu_speed,&dma_clock_counter))return 0;
    if(!civ_dma_channel_run(i,channel,channel_index,cpu_speed,&dma_clock_counter))return 0;

    if(cpu_speed!=6u && cpu_speed!=8u && cpu_speed!=12u)
        return civ_fail_frontier(i,"Manual DMA resumed at an unproved CPU bus speed.",NULL);
    end_sync=(uint32_t)cpu_speed-(dma_clock_counter%(uint32_t)cpu_speed);
    civ_timing_advance_master(i,end_sync);
    return 1;
}

uint8_t civ_dma_process_cpu_cycle(CivRecomp *i,uint8_t cpu_speed)
{
    int ran=0;
    if(!i || !i->natural_timing_enabled || i->cpu_bus_timing_suppressed || i->failed)return 0u;
    if(i->dma_pending_mask==0u && !i->hdma_init_pending && !i->hdma_transfer_pending)return 0u;
    /* MesenCE/65c816 arbitration uses one shared start-delay latch.  HDMA
       transfer has priority over HDMA init, which has priority over MDMA. */
    if(i->dma_start_delay) {
        i->dma_start_delay=0u;
        return 0u;
    }
    if(i->hdma_transfer_pending) {
        i->hdma_transfer_pending=0u;
        if(i->cpu_io[0x0Cu]!=0u) { if(!civ_hdma2_scanline(i,cpu_speed))return 0u; ran=1; }
    } else if(i->hdma_init_pending) {
        i->hdma_init_pending=0u;
        if(i->cpu_io[0x0Cu]!=0u) { if(!civ_hdma2_init(i,cpu_speed))return 0u; ran=1; }
        else { i->hdma2.finished=0u; i->hdma2.do_transfer=0u; i->hdma2.initialized=0u; }
    } else if(i->dma_pending_mask!=0u) {
        if(!civ_dma_execute_pending(i,cpu_speed))return 0u;
        ran=1;
    }
    /* The S-CPU interrupt lock is asserted for the CPU cycle in which an
       actual DMA/HDMA transfer ran. The NMI edge detector uses this to defer a
       just-matured request by one more CPU cycle. */
    return (uint8_t)(!i->failed && ran);
}
