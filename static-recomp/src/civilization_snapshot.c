#include "civilization_internal.h"
#include "civilization_audio.h"
#include "civilization_diagnostics.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _WIN32
#include <windows.h>
#endif

#define CIV_SNAPSHOT_VERSION 36u
#define CIV_CORE_IDENTITY "6b60b742ae0e47a60b91cf8d083288ca5965f391571a307516c3cb9155b27874"

typedef struct CivSnapshotHeaderV36 {
    char magic[8];
    uint32_t version;
    uint32_t runtime_size;
    uint32_t audio_size;
    char rom_sha256[65];
    char core_identity[65];
    char payload_sha256[65];
} CivSnapshotHeaderV36;

static void snapshot_error(char *error, size_t capacity, const char *text) {
    if (!error || capacity == 0u) return;
    if (!text) text = "";
    (void)snprintf(error, capacity, "%s", text);
}

static int replace_file_atomically(const char *temporary, const char *target) {
#ifdef _WIN32
    return MoveFileExA(temporary,target,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)?1:0;
#else
    return rename(temporary,target)==0;
#endif
}

static void write_le32(uint8_t *out,uint32_t value)
{out[0]=(uint8_t)value;out[1]=(uint8_t)(value>>8);out[2]=(uint8_t)(value>>16);out[3]=(uint8_t)(value>>24);}
static uint32_t read_le32_snapshot(const uint8_t *in)
{return (uint32_t)in[0]|((uint32_t)in[1]<<8)|((uint32_t)in[2]<<16)|((uint32_t)in[3]<<24);}

static size_t runtime_payload_size(void)
{
    size_t size=0u;
#define CIV_STATE_FIELD(name) size+=4u+sizeof(((CivRecomp *)0)->name);
#include "civilization_snapshot_fields.inc"
#undef CIV_STATE_FIELD
    return size;
}

static int serialize_runtime(const CivRecomp *instance,uint8_t *out,size_t capacity)
{
    size_t used=0u;
    if(!instance||!out||capacity!=runtime_payload_size())return 0;
#define CIV_STATE_FIELD(name) do { \
    size_t field_size=sizeof(instance->name); \
    write_le32(out+used,(uint32_t)field_size);used+=4u; \
    memcpy(out+used,&instance->name,field_size);used+=field_size; \
} while(0);
#include "civilization_snapshot_fields.inc"
#undef CIV_STATE_FIELD
    return used==capacity;
}

static int deserialize_runtime(CivRecomp *instance,const uint8_t *in,size_t size)
{
    size_t used=0u;
    if(!instance||!in||size!=runtime_payload_size())return 0;
    memset(instance,0,sizeof(*instance));
#define CIV_STATE_FIELD(name) do { \
    size_t field_size=sizeof(instance->name); \
    if(used+4u>size||read_le32_snapshot(in+used)!=(uint32_t)field_size){return 0;}used+=4u; \
    if(used+field_size>size){return 0;}memcpy(&instance->name,in+used,field_size);used+=field_size; \
} while(0);
#include "civilization_snapshot_fields.inc"
#undef CIV_STATE_FIELD
    return used==size;
}

int civ_snapshot_save(const CivRecomp *instance, const char *path,
                      char *error, size_t error_cap) {
    CivSnapshotHeaderV36 header;
    uint8_t *runtime = NULL;
    size_t runtime_size=runtime_payload_size();
    uint8_t *audio = NULL;
    size_t audio_size = 0u;
    CivRomInfo rom_info;
    FILE *file = NULL;
    uint8_t *payload = NULL;
    char *temporary = NULL;
    int ok = 0;

    if (!instance || !path || !*path || !instance->rom ||
        instance->rom_size != CIV_ROM_SIZE) {
        snapshot_error(error, error_cap,
                       "A loaded exact Civilization ROM and snapshot path are required.");
        return 0;
    }
    if (!civ_verify_rom(instance->rom, instance->rom_size, &rom_info,
                        error, error_cap)) return 0;

    runtime = (uint8_t *)malloc(runtime_size);
    if (!runtime) {
        snapshot_error(error, error_cap, "Unable to allocate snapshot runtime state.");
        return 0;
    }
    if(!serialize_runtime(instance,runtime,runtime_size)){
        snapshot_error(error,error_cap,"Unable to serialize canonical runtime fields.");goto done;
    }

    if (instance->v20_full_static_audio_acquired) {
        audio_size = civ_v20_audio_state_size();
        if (audio_size == 0u || audio_size > UINT32_MAX) {
            snapshot_error(error, error_cap, "Full Static audio snapshot size is invalid.");
            goto done;
        }
        audio = (uint8_t *)malloc(audio_size);
        if (!audio || !civ_v20_audio_state_save(instance, audio, audio_size)) {
            snapshot_error(error, error_cap, "Unable to capture Full Static audio state.");
            goto done;
        }
    }

    memset(&header, 0, sizeof(header));
    memcpy(header.magic, "CVSNAP36", 8u);
    header.version = CIV_SNAPSHOT_VERSION;
    header.runtime_size = (uint32_t)runtime_size;
    header.audio_size = (uint32_t)audio_size;
    memcpy(header.rom_sha256,rom_info.sha256,sizeof(header.rom_sha256));
    memcpy(header.core_identity,CIV_CORE_IDENTITY,sizeof(header.core_identity));
    payload=(uint8_t *)malloc(runtime_size+audio_size);
    if(!payload){snapshot_error(error,error_cap,"Unable to allocate canonical snapshot payload.");goto done;}
    memcpy(payload,runtime,runtime_size);
    if(audio_size)memcpy(payload+runtime_size,audio,audio_size);
    civ_sha256_hex(payload,runtime_size+audio_size,header.payload_sha256);

    temporary=(char *)malloc(strlen(path)+6u);
    if(!temporary){snapshot_error(error,error_cap,"Unable to allocate temporary snapshot path.");goto done;}
    (void)sprintf(temporary,"%s.tmp",path);
    file = fopen(temporary, "wb");
    if (!file) {
        snapshot_error(error, error_cap, "Unable to create snapshot file.");
        goto done;
    }
    if (fwrite(&header, 1u, sizeof(header), file) != sizeof(header) ||
        fwrite(runtime, 1u, runtime_size, file) != runtime_size ||
        (audio_size != 0u && fwrite(audio, 1u, audio_size, file) != audio_size) ||
        fclose(file) != 0) {
        file = NULL;
        snapshot_error(error, error_cap, "Unable to write complete snapshot file.");
        goto done;
    }
    file = NULL;
    if(!replace_file_atomically(temporary,path)){
        snapshot_error(error,error_cap,"Unable to atomically publish snapshot file.");
        goto done;
    }
    snapshot_error(error, error_cap, "");
    ok = 1;

done:
    if (file) (void)fclose(file);
    if(!ok&&temporary)(void)remove(temporary);
    free(temporary);
    free(payload);
    free(audio);
    free(runtime);
    return ok;
}

int civ_snapshot_load(CivRecomp *instance, const uint8_t *rom, size_t rom_size,
                      const char *path, char *error, size_t error_cap) {
    CivSnapshotHeaderV36 header;
    CivRecomp *loaded = NULL;
    uint8_t *runtime = NULL;
    uint8_t *audio = NULL;
    CivRomInfo rom_info;
    FILE *file = NULL;
    uint8_t *payload = NULL;
    char payload_sha256[65];
    CivHostHooks preserved_hooks;
    int trailing;
    int ok = 0;

    if (!instance || !rom || !path || !*path) {
        snapshot_error(error, error_cap,
                       "A loaded exact Civilization ROM and snapshot path are required.");
        return 0;
    }
    if (!civ_verify_rom(rom, rom_size, &rom_info, error, error_cap)) return 0;
    preserved_hooks=instance->host_hooks;
    file = fopen(path, "rb");
    if (!file) {
        snapshot_error(error, error_cap, "Unable to open snapshot file.");
        return 0;
    }
    if (fread(&header, 1u, sizeof(header), file) != sizeof(header) ||
        memcmp(header.magic, "CVSNAP36", 8u) != 0 ||
        header.version != CIV_SNAPSHOT_VERSION ||
        header.runtime_size != runtime_payload_size() ||
        strcmp(header.rom_sha256,rom_info.sha256)!=0 ||
        strcmp(header.core_identity,CIV_CORE_IDENTITY)!=0 ||
        (header.audio_size != 0u && header.audio_size != civ_v20_audio_state_size())) {
        snapshot_error(error, error_cap,
                       "Snapshot is invalid, for another ROM, or from another build.");
        goto done;
    }

    loaded = (CivRecomp *)malloc(sizeof(*loaded));
    if (!loaded) {
        snapshot_error(error, error_cap, "Unable to allocate snapshot runtime state.");
        goto done;
    }
    runtime=(uint8_t *)malloc(header.runtime_size);
    if(!runtime){snapshot_error(error,error_cap,"Unable to allocate canonical runtime payload.");goto done;}
    if (header.audio_size != 0u) {
        audio = (uint8_t *)malloc(header.audio_size);
        if (!audio) {
            snapshot_error(error, error_cap, "Unable to allocate snapshot audio state.");
            goto done;
        }
    }
    if (fread(runtime, 1u, header.runtime_size, file) != header.runtime_size ||
        (header.audio_size != 0u &&
         fread(audio, 1u, header.audio_size, file) != header.audio_size) ||
        (trailing = fgetc(file)) != EOF || ferror(file)) {
        snapshot_error(error, error_cap, "Snapshot data is truncated or has trailing bytes.");
        goto done;
    }

    payload=(uint8_t *)malloc(header.runtime_size+header.audio_size);
    if(!payload){snapshot_error(error,error_cap,"Unable to allocate snapshot verification payload.");goto done;}
    memcpy(payload,runtime,header.runtime_size);
    if(header.audio_size)memcpy(payload+header.runtime_size,audio,header.audio_size);
    civ_sha256_hex(payload,header.runtime_size+header.audio_size,payload_sha256);
    if (strcmp(payload_sha256,header.payload_sha256)!=0) {
        snapshot_error(error, error_cap, "Snapshot SHA-256 integrity check failed.");
        goto done;
    }
    if(!deserialize_runtime(loaded,runtime,header.runtime_size)){
        snapshot_error(error,error_cap,"Snapshot canonical field schema is invalid.");goto done;
    }

    memcpy(instance, loaded, sizeof(*loaded));
    instance->rom = rom;
    instance->rom_size = rom_size;
    instance->headless_frame_stop_enabled = 0u;
    instance->headless_frame_stop_reached = 0u;
    instance->headless_frame_stop_target = 0u;
    instance->host_hooks=preserved_hooks;

    if (header.audio_size != 0u) {
        /* A fresh process has no static-APU owner.  Acquire the one production
           Full Static audio lane, then replace its state with the snapshot. */
        if (!civ_v20_audio_begin(instance) ||
            !civ_v20_audio_state_load(instance, audio, header.audio_size)) {
            snapshot_error(error, error_cap, "Unable to restore Full Static audio state.");
            goto done;
        }
    }
    /* Snapshot restoration can rewind guest SRAM relative to the on-disk
       battery image.  Mark the host persistence image dirty so the frontend
       can atomically flush the restored exact SRAM later. */
    instance->sram_dirty = 1u;
    snapshot_error(error, error_cap, "");
    ok = 1;

done:
    if (file) (void)fclose(file);
    free(audio);
    free(runtime);
    free(payload);
    free(loaded);
    return ok;
}
