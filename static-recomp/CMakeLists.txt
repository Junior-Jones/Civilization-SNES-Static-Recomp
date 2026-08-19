# Civilization Version 35 current Full Static core (Version 33 proof identity).
#
# Production CPU authority is exactly one generated compact closed graph.
# Every original PBR:PC:E:M:X context remains present, while its repeated C
# semantics are factored into frozen AOT templates.  There is no selectable
# legacy body-per-context core, runtime decoder, learning, or fallback.
set(CIVILIZATION_V35_COMPACT_AOT_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/generated/compact-aot")
file(GLOB CIVILIZATION_ACTIVE_CORE_SHARDS CONFIGURE_DEPENDS
    "${CIVILIZATION_V35_COMPACT_AOT_DIR}/civilization_compact_aot_shards/*.c")
list(LENGTH CIVILIZATION_ACTIVE_CORE_SHARDS
    CIVILIZATION_V35_COMPACT_AOT_SHARD_COUNT)
if(NOT CIVILIZATION_V35_COMPACT_AOT_SHARD_COUNT EQUAL 280)
    message(FATAL_ERROR
        "Compact AOT shard count mismatch: expected=280 "
        "actual=${CIVILIZATION_V35_COMPACT_AOT_SHARD_COUNT}")
endif()
set(CIVILIZATION_ACTIVE_CORE_SUPPORT
    "${CIVILIZATION_V35_COMPACT_AOT_DIR}/civilization_compact_aot_templates.c")

set(CIVILIZATION_CURRENT_CORE_SOURCES
    src/civilization_runtime.c
    src/civilization_machine.c
    src/civilization_diagnostics.c
    src/civilization_snapshot.c
    src/civilization_cpu_support.c
    src/civilization_scheduler.c
    src/civilization_input.c
    src/civilization_ppu_io.c
    src/civilization_dma.c
    src/civilization_cartridge.c
    src/civilization_bus.c
    src/civilization_cpu_timing.c
    src/civilization_hash.c
    src/civilization_mapper.c
    src/civilization_renderer.c
    src/civilization_audio.cpp
    generated/civilization_generated_core.c
    generated/civilization_generated_returns.c
    ${CIVILIZATION_ACTIVE_CORE_SUPPORT}
    ${CIVILIZATION_ACTIVE_CORE_SHARDS})

# Full Static S-SMP/S-DSP is the only production audio authority.
add_library(civilization-static-apu STATIC
    static-audio/civilization-bapu-aot/sc_static_apu.cpp
    static-audio/civilization-bapu-aot/smp/smp.cpp
    static-audio/civilization-bapu-aot/dsp/sdsp.cpp)
target_include_directories(civilization-static-apu PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/static-audio/civilization-bapu-aot")
target_compile_definitions(civilization-static-apu PRIVATE
    SC_SMP_AOT=1
    SC_STATIC_SDSP_PRIMITIVES=1
    SCStaticSPC_DSP_CUSTOM_RUN=1)
if(NOT MSVC)
    target_compile_options(civilization-static-apu PRIVATE
        -Wall -Wextra -Wpedantic
        -Wno-misleading-indentation -Wno-class-memaccess)
endif()

add_library(civilization-static-recomp STATIC ${CIVILIZATION_CURRENT_CORE_SOURCES})
target_include_directories(civilization-static-recomp
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/internal/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/generated"
        "${CIVILIZATION_V35_COMPACT_AOT_DIR}")
target_link_libraries(civilization-static-recomp PRIVATE civilization-static-apu)
civ_apply_warnings(civilization-static-recomp)

# Generated exact-PC dispatch is deterministic machine-produced authority.
# Keep handwritten code at the project's strict warning level; compile the
# large generated index/returns/shards quietly with the compiler's size
# optimization.  The V35 C0 experiment measured -Os 12.18% smaller than -O1
# without changing the generated static authority.
if(MSVC)
    set_source_files_properties(
        generated/civilization_generated_core.c
        generated/civilization_generated_returns.c
        ${CIVILIZATION_ACTIVE_CORE_SUPPORT}
        ${CIVILIZATION_ACTIVE_CORE_SHARDS}
        PROPERTIES COMPILE_OPTIONS "/W0;/O1;/wd4310;/wd4702")
else()
    set_source_files_properties(
        generated/civilization_generated_core.c
        generated/civilization_generated_returns.c
        ${CIVILIZATION_ACTIVE_CORE_SUPPORT}
        ${CIVILIZATION_ACTIVE_CORE_SHARDS}
        PROPERTIES COMPILE_OPTIONS "-w;-Os")
endif()
