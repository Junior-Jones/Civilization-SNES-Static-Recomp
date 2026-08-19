#include "civilization_internal.h"
#include "civilization_audio.h"
#include "civilization_generated_core.h"

/*
 * Version 33 production runtime.
 *
 * There is one S-CPU authority: the closed, exact-ROM Version 33 generated
 * graph.  It starts at the hardware reset vector and contains every CPU
 * context proved by the offline fixed-point analysis, including the finite
 * NMI/IRQ re-entry contexts.  Earlier bootstraps, continuation joins and
 * runtime indirect guards are historical evidence only and are not executable
 * production layers.
 *
 * The handwritten code below owns only machine scheduling: Full Static audio,
 * instruction-boundary interrupt sampling and the headless frame stop.  An
 * unexpected CPU context is rejected by civ_generated_core_step(); there is no
 * runtime decoder, learned edge, observed-target escape hatch or interpreter
 * fallback.
 */
static int civ_accept_sampled_interrupt(CivRecomp *i)
{
    /* NMI has priority.  Requests are accepted only between generated
       instructions so the interrupted PC is itself a statically reached
       instruction boundary. */
    if(i->nmi_pending && i->nmi_enable && i->nmi_depth==0u && i->irq_depth==0u)
        return civ_nmi_enter_native(i);
    if(i->irq_line && !(i->cpu.p&CIV_P_I) && i->nmi_depth==0u && i->irq_depth==0u)
        return civ_irq_enter_native(i);
    return 1;
}

int civ_run_static(CivRecomp *i,uint64_t max_instructions)
{
    uint64_t start;
    if(!i || i->failed)return 0;
    if(!i->rom || i->rom_size!=CIV_ROM_SIZE)
        return civ_fail_frontier(i,
            "Version 33 static runner requires the exact verified external Civilization ROM.",
            NULL);

    if(i->instruction_count==0u) {
        i->natural_timing_enabled=1u;
        if(!civ_v20_audio_begin(i))return 0;
        i->v20_full_static_audio_authoritative=1u;
    } else if(!i->v20_full_static_audio_enabled) {
        return civ_fail_frontier(i,
            "Version 33 static runner resumed without Full Static S-SMP/S-DSP ownership.",
            NULL);
    }

    start=i->instruction_count;
    while(!i->failed && i->instruction_count-start<max_instructions) {
        if(!civ_generated_core_step(i))return 0;
        if(!civ_accept_sampled_interrupt(i))return 0;
        if(civ_headless_frame_stop(i))break;
    }

    if(!i->failed && i->v20_full_static_audio_acquired &&
       !civ_v20_audio_sync_internal(i))return 0;
    return !i->failed;
}

/* Public compatibility name retained for existing frontends/tests.  It is an
   alias to the single Version 33 production runner, not a Version 20 path. */
int civ_run_v20(CivRecomp *i,uint64_t max_instructions)
{
    return civ_run_static(i,max_instructions);
}
