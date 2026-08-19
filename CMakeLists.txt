cmake_minimum_required(VERSION 3.20)
project(civilization_snes_static_recomp_v1_1_0 C CXX)

option(CIVILIZATION_BUILD_WINDOWS_LAUNCHER
    "Build the native Windows Launcher and its SDL dependency" ON)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    add_compile_options(/MP /experimental:deterministic)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    add_link_options(/STACK:16777216)
endif()
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(civ_apply_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
    endif()
endfunction()

add_subdirectory(static-recomp)

add_library(civilization-frontend-common STATIC
    frontend/common/civilization_frontend.c
    frontend/common/civilization_input_latch.c)
target_include_directories(civilization-frontend-common PUBLIC frontend/common static-recomp/include)
target_link_libraries(civilization-frontend-common PUBLIC civilization-static-recomp)
civ_apply_warnings(civilization-frontend-common)

add_executable(civilization-headless frontend/linux/civilization_headless.c)
target_link_libraries(civilization-headless PRIVATE civilization-frontend-common)
civ_apply_warnings(civilization-headless)

if(WIN32 AND CIVILIZATION_BUILD_WINDOWS_LAUNCHER)
    include(FetchContent)
    set(SDL_SHARED OFF CACHE BOOL "" FORCE)
    set(SDL_STATIC ON CACHE BOOL "" FORCE)
    set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
    set(SDL_TESTS OFF CACHE BOOL "" FORCE)
    set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(civilization_sdl
        URL "https://github.com/libsdl-org/SDL/archive/refs/tags/release-3.4.10.zip"
        URL_HASH "SHA256=30DCE54890B17544A35E4521274762016AF6ED59C2183B896EF5E6693E97FB6C"
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    FetchContent_MakeAvailable(civilization_sdl)
    FetchContent_GetProperties(civilization_sdl SOURCE_DIR CIVILIZATION_SDL_SOURCE_DIR)

    add_executable(civilization-launcher WIN32
        frontend/windows/civilization_app_win32.c
        frontend/windows/civilization_app_core.c
        frontend/windows/launcher_version.rc
        frontend/windows/audio_output_winmm.c
        frontend/windows/audio_recorder_win32.c
        frontend/windows/frontend_settings_win32.c
        third_party/civilization-gamepad/gamepad_input_win32.c)
    target_include_directories(civilization-launcher PRIVATE
        frontend/common
        frontend/windows
        static-recomp/include
        third_party/civilization-gamepad)
    target_compile_definitions(civilization-launcher PRIVATE
        UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX _CRT_SECURE_NO_WARNINGS)
    target_link_libraries(civilization-launcher PRIVATE
        civilization-frontend-common SDL3::SDL3-static
        winmm user32 gdi32 comdlg32 comctl32 shell32)
    if(MINGW)
        target_link_options(civilization-launcher PRIVATE -municode)
    endif()
    set_target_properties(civilization-launcher PROPERTIES OUTPUT_NAME "Launcher")
    civ_apply_warnings(civilization-launcher)
    add_custom_command(TARGET civilization-launcher POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/gamecontrollerdb.txt"
            "$<TARGET_FILE_DIR:civilization-launcher>/gamecontrollerdb.txt"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
            "${CIVILIZATION_SDL_SOURCE_DIR}/LICENSE.txt"
            "$<TARGET_FILE_DIR:civilization-launcher>/SDL-LICENSE.txt"
        VERBATIM)
endif()

include(CTest)
if(BUILD_TESTING)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME arch-v36-completion
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_v36_architecture_completion.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME arch-v36-source-generated-provenance
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/verify_v36_provenance.py"
            --project "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(civilization-v35-input-latch-selftest
    tests/test_v35_input_latch.c
    frontend/common/civilization_input_latch.c)
target_include_directories(civilization-v35-input-latch-selftest PRIVATE
    frontend/common)
civ_apply_warnings(civilization-v35-input-latch-selftest)
add_test(NAME v35-input-latch-regression
    COMMAND civilization-v35-input-latch-selftest)

# Version 36 architecture characterization. These gates are intentionally
# small and separate from the historical milestone tests so later subsystem
# refactors must preserve the current externally observable contracts.
add_executable(civilization-arch-api-surface tests/test_arch_api_surface.c)
target_link_libraries(civilization-arch-api-surface PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-arch-api-surface)
add_test(NAME arch-public-api-surface COMMAND civilization-arch-api-surface)
add_test(NAME arch-opaque-core-lifecycle
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_opaque_core_api.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(civilization-arch-diagnostics-api tests/test_arch_diagnostics_api.c)
target_link_libraries(civilization-arch-diagnostics-api PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-arch-diagnostics-api)
add_test(NAME arch-diagnostics-api COMMAND civilization-arch-diagnostics-api)
add_test(NAME arch-diagnostics-api-separation
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_diagnostics_api_separation.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(civilization-arch-renderer-instances tests/test_arch_renderer_instances.cpp)
target_include_directories(civilization-arch-renderer-instances PRIVATE static-recomp/internal/include)
target_link_libraries(civilization-arch-renderer-instances PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-arch-renderer-instances)
add_test(NAME arch-parallel-renderer-instances COMMAND civilization-arch-renderer-instances)

add_executable(civilization-arch-audio-ownership tests/test_arch_audio_ownership.cpp)
target_include_directories(civilization-arch-audio-ownership PRIVATE static-recomp/internal/include)
target_link_libraries(civilization-arch-audio-ownership PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-arch-audio-ownership)
add_test(NAME arch-audio-ownership-and-pcm-hook COMMAND civilization-arch-audio-ownership)

add_executable(civilization-arch-generated-dispatch tests/test_arch_generated_dispatch.c)
target_include_directories(civilization-arch-generated-dispatch PRIVATE
    static-recomp/internal/include static-recomp/generated)
target_link_libraries(civilization-arch-generated-dispatch PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-arch-generated-dispatch)
add_test(NAME arch-unknown-dispatch-fails-closed COMMAND civilization-arch-generated-dispatch)

# Static machine-layer regression tests.  They do not run a natural game route.
add_executable(civilization-cartridge-mapping-selftest tests/test_cartridge_mapping.c)
target_link_libraries(civilization-cartridge-mapping-selftest PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-cartridge-mapping-selftest)
add_test(NAME v33-cartridge-mapping COMMAND civilization-cartridge-mapping-selftest)

add_executable(civilization-natural-timing-selftest tests/test_v33_natural_timing.c)
target_link_libraries(civilization-natural-timing-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-natural-timing-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-natural-timing-selftest)
add_test(NAME v33-master-clock-model COMMAND civilization-natural-timing-selftest)

add_executable(civilization-ppu-config-selftest tests/test_v33_ppu_config.c)
target_link_libraries(civilization-ppu-config-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-ppu-config-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-ppu-config-selftest)
add_test(NAME v33-ppu-config-regression COMMAND civilization-ppu-config-selftest)

add_executable(civilization-v33-ppu-multiply-selftest tests/test_v33_ppu_multiply.c)
target_link_libraries(civilization-v33-ppu-multiply-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-ppu-multiply-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-v33-ppu-multiply-selftest)
add_test(NAME v33-ppu-multiply-regression COMMAND civilization-v33-ppu-multiply-selftest)

add_executable(civilization-v33-rom-ppu-multiply-route-selftest tests/test_v33_rom_ppu_multiply_route.c)
target_link_libraries(civilization-v33-rom-ppu-multiply-route-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-rom-ppu-multiply-route-selftest PRIVATE
    static-recomp/internal/include static-recomp/generated)
civ_apply_warnings(civilization-v33-rom-ppu-multiply-route-selftest)

add_executable(civilization-v33-prebiased-indirect-selftest tests/test_v33_prebiased_indirect.c)
target_link_libraries(civilization-v33-prebiased-indirect-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-prebiased-indirect-selftest PRIVATE
    static-recomp/internal/include static-recomp/generated)
civ_apply_warnings(civilization-v33-prebiased-indirect-selftest)

add_executable(civilization-dma-oam-selftest tests/test_v33_dma_oam.c)
target_link_libraries(civilization-dma-oam-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-dma-oam-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-dma-oam-selftest)
add_test(NAME v33-dma-oam-regression COMMAND civilization-dma-oam-selftest)

add_executable(civilization-v33-hdma-selftest tests/test_v33_hdma_channel2.c)
target_link_libraries(civilization-v33-hdma-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-hdma-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-v33-hdma-selftest)


add_executable(civilization-v33-dma-hdma-arbitration-selftest tests/test_v33_dma_hdma_arbitration.c)
target_link_libraries(civilization-v33-dma-hdma-arbitration-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-dma-hdma-arbitration-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-v33-dma-hdma-arbitration-selftest)

add_executable(civilization-v33-mode3-renderer-selftest tests/test_v33_mode3_renderer.c)
target_include_directories(civilization-v33-mode3-renderer-selftest PRIVATE static-recomp/internal/include)
target_link_libraries(civilization-v33-mode3-renderer-selftest PRIVATE civilization-static-recomp)
civ_apply_warnings(civilization-v33-mode3-renderer-selftest)
add_test(NAME v33-mode3-renderer COMMAND civilization-v33-mode3-renderer-selftest)

add_executable(civilization-controller-serial-selftest tests/test_v33_controller_serial.c)
target_link_libraries(civilization-controller-serial-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-controller-serial-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-controller-serial-selftest)
add_test(NAME v33-controller-serial-regression COMMAND civilization-controller-serial-selftest)

add_executable(civilization-v33-sram-persistence-selftest tests/test_v33_sram_persistence.c)
target_link_libraries(civilization-v33-sram-persistence-selftest PRIVATE civilization-static-recomp)
target_include_directories(civilization-v33-sram-persistence-selftest PRIVATE static-recomp/internal/include)
civ_apply_warnings(civilization-v33-sram-persistence-selftest)
add_test(NAME v33-sram-persistence COMMAND civilization-v33-sram-persistence-selftest)

add_test(NAME v33-closed-authority
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/verify_v33_closed_authority.py"
            --project "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME v35-compact-aot-authority
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/verify_v35_compact_aot.py"
            --project "${CMAKE_CURRENT_SOURCE_DIR}")
add_test(NAME v33-static-io-read-surface
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_io_read_surface.py"
            --project "${CMAKE_CURRENT_SOURCE_DIR}"
            --out "${CMAKE_CURRENT_BINARY_DIR}/v33-io-read-surface.json")

add_test(NAME v33-static-audio-authority
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/audit_v33_static_audio_authority.py"
            --project "${CMAKE_CURRENT_SOURCE_DIR}"
            --out "${CMAKE_CURRENT_BINARY_DIR}/v33-static-audio-authority.json")
add_test(NAME arch-no-legacy-hybrid-audio-state
    COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/verify_no_legacy_audio_state.py"
            "${CMAKE_CURRENT_SOURCE_DIR}")

if(DEFINED CIVILIZATION_TEST_ROM AND EXISTS "${CIVILIZATION_TEST_ROM}")
    add_executable(civilization-arch-frame-hooks tests/test_arch_frame_hooks.c)
    target_link_libraries(civilization-arch-frame-hooks PRIVATE civilization-static-recomp)
    civ_apply_warnings(civilization-arch-frame-hooks)
    add_test(NAME arch-authoritative-frame-hooks
        COMMAND civilization-arch-frame-hooks "${CIVILIZATION_TEST_ROM}")

    add_executable(civilization-arch-snapshot-transaction
        tests/test_arch_snapshot_transaction.c)
    target_include_directories(civilization-arch-snapshot-transaction PRIVATE
        static-recomp/internal/include)
    target_link_libraries(civilization-arch-snapshot-transaction PRIVATE
        civilization-static-recomp)
    civ_apply_warnings(civilization-arch-snapshot-transaction)
    add_test(NAME arch-snapshot-corruption-rollback
        COMMAND civilization-arch-snapshot-transaction
            "${CIVILIZATION_TEST_ROM}"
            "${CMAKE_CURRENT_BINARY_DIR}/arch-valid.civsnap"
            "${CMAKE_CURRENT_BINARY_DIR}/arch-corrupt.civsnap")

    add_executable(civilization-v35-screenshot-diagnostic-selftest
        tests/test_v35_screenshot_diagnostic.c
        frontend/windows/civilization_app_core.c)
    target_include_directories(civilization-v35-screenshot-diagnostic-selftest PRIVATE
        frontend/common frontend/windows static-recomp/include)
    target_link_libraries(civilization-v35-screenshot-diagnostic-selftest PRIVATE
        civilization-frontend-common)
    civ_apply_warnings(civilization-v35-screenshot-diagnostic-selftest)
    add_test(NAME v35-screenshot-static-log
        COMMAND civilization-v35-screenshot-diagnostic-selftest
                "${CIVILIZATION_TEST_ROM}"
                "${CMAKE_CURRENT_BINARY_DIR}/v35-screenshot-static-log.txt")
    add_test(NAME v33-rom-inspection
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/rom/inspect_civilization_rom.py"
                "${CIVILIZATION_TEST_ROM}" --expect)
    add_test(NAME v33-wrong-rom-rejected
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/wrong_rom_headless_test.py"
                "$<TARGET_FILE:civilization-headless>" "${CIVILIZATION_TEST_ROM}")
    add_test(NAME v33-hdma-channel2
        COMMAND civilization-v33-hdma-selftest "${CIVILIZATION_TEST_ROM}")
    add_test(NAME v33-dma-hdma-arbitration
        COMMAND civilization-v33-dma-hdma-arbitration-selftest "${CIVILIZATION_TEST_ROM}")
    add_test(NAME v33-hdma-source-proof
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/prove_v33_hdma_channel2.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-hdma-channel2-proof.json")
    add_test(NAME v33-ppu-domain-proof
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_ppu_domains.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-ppu-domain-proof.json")
    add_test(NAME v33-rom-ppu-multiply-route
        COMMAND civilization-v33-rom-ppu-multiply-route-selftest "${CIVILIZATION_TEST_ROM}")
    add_test(NAME v33-prebiased-indirect-generated
        COMMAND civilization-v33-prebiased-indirect-selftest "${CIVILIZATION_TEST_ROM}")
    add_test(NAME v33-indirect-effective-address
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_indirect_effective_address.py"
                "${CIVILIZATION_TEST_ROM}"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-indirect-effective-address.json")
    add_test(NAME v33-ppu-read-port-audit
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_ppu_read_ports.py"
                "${CIVILIZATION_TEST_ROM}"
                --project "${CMAKE_CURRENT_SOURCE_DIR}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-ppu-read-port-audit.json")
    add_test(NAME v33-lifecycle-partition
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_lifecycle_partition.py"
                "${CIVILIZATION_TEST_ROM}"
                --ownership "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_ownership.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-lifecycle-partition.json")
    add_test(NAME v33-indirect-recertification
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_indirect_recertification.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-indirect-recertification.json")
    add_test(NAME v33-machine-guard-audit
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_machine_guards.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-machine-guard-audit.json")
    add_test(NAME v33-name-state-machine-proof
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/prove_v33_name_state_machine.py"
                "${CIVILIZATION_TEST_ROM}"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-name-state-machine-proof.json")
    add_test(NAME v33-indirect-transition-consistency
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_indirect_transition_consistency.py"
                "${CIVILIZATION_TEST_ROM}"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-indirect-transition-consistency.json")
    add_test(NAME v33-static-io-write-surface
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_io_write_surface.py"
                "${CIVILIZATION_TEST_ROM}"
                --project "${CMAKE_CURRENT_SOURCE_DIR}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-io-write-surface.json")
    add_test(NAME v33-indirect-domain-completeness
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_indirect_domain_completeness.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-indirect-domain-completeness.json")
    add_test(NAME v33-new-context-route
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_new_context_route.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --ownership "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_ownership.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-new-context-route.json")
    add_test(NAME v33-call-return-consistency
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_call_return_consistency.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --ownership "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_ownership.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-call-return-consistency.json")
    add_test(NAME v33-setup-city-route-coverage
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_setup_city_route_coverage.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-setup-city-route-coverage.json")
    add_test(NAME v33-legacy-indirect-second-method
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_legacy_indirect_second_method.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-legacy-indirect-second-method.json")
    add_test(NAME v33-generalized-indirect-domains
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_generalized_indirect_domains.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-generalized-indirect-domains.json")
    add_test(NAME v35-indirect-producer-completeness
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v35_indirect_producer_completeness.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --generalized "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_generalized_indirect_domains.json"
                --legacy "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_legacy_indirect_second_method.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v35-indirect-producer-completeness.json")
    add_test(NAME v33-special-return-reentry
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_special_return_reentry.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-special-return-reentry.json")
    add_test(NAME v33-earth-city-lifecycle-clusters
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/scpu/audit_v33_earth_city_lifecycle_clusters.py"
                "${CIVILIZATION_TEST_ROM}"
                --analysis "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_fixedpoint.manifest.json"
                --ownership "${CMAKE_CURRENT_SOURCE_DIR}/docs/research/civilization_v33_ownership.manifest.json"
                --indirect-proof "${CMAKE_CURRENT_SOURCE_DIR}/config/static-indirect-control-flow-v33.json"
                --out "${CMAKE_CURRENT_BINARY_DIR}/v33-earth-city-lifecycle-clusters.json")
    add_test(NAME v33-persistent-snapshot
        COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tools/testing/test_v33_persistent_snapshot.py"
                --headless "$<TARGET_FILE:civilization-headless>"
                --rom "${CIVILIZATION_TEST_ROM}")
else()
    message(STATUS "CIVILIZATION_TEST_ROM not supplied; ROM identity tests are not registered")
endif()
endif() # BUILD_TESTING
