#pragma once

// Civilization project developer-only instrumentation for MesenCE 2.2.1.
// This logger observes the independent oracle. It is not linked into the
// static recomp and must never be used as source-generation authority.

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace CivilizationOracleTrace {

constexpr uint32_t FrontierPc = 0xC08088u;

inline bool &DoneFlag()
{
    static bool done = false;
    return done;
}

inline FILE *GetFile()
{
    static FILE *file = nullptr;
    static bool attempted = false;
    if(attempted) {
        return file;
    }
    attempted = true;

    const char *path = std::getenv("CIVILIZATION_ORACLE_TRACE");
    if(path == nullptr || path[0] == '\0') {
        return nullptr;
    }

    file = std::fopen(path, "wb");
    if(file != nullptr) {
        std::fputs("event,master_clock,cpu_cycle,pc,opcode,e,a,x,y,d,dbr,sp,ps,hclock,scanline,address,value,op_type\n", file);
        std::fflush(file);
    }
    return file;
}

inline bool RecordWramWrites()
{
    const char *value = std::getenv("CIVILIZATION_ORACLE_TRACE_WRAM");
    return value != nullptr && std::strcmp(value, "0") != 0;
}

inline bool IsIoBank(uint8_t bank)
{
    return bank <= 0x3Fu || (bank >= 0x80u && bank <= 0xBFu);
}

inline bool IsObservedRegister(uint32_t address)
{
    uint8_t bank = static_cast<uint8_t>(address >> 16);
    uint16_t local = static_cast<uint16_t>(address);
    return IsIoBank(bank) && ((local >= 0x2100u && local <= 0x21FFu) ||
        (local >= 0x4200u && local <= 0x43FFu));
}

inline void Instruction(uint64_t masterClock, uint64_t cpuCycle, uint32_t pc,
    uint8_t opcode, bool emulationMode, uint16_t a, uint16_t x, uint16_t y,
    uint16_t d, uint8_t dbr, uint16_t sp, uint8_t ps, uint16_t hclock,
    uint16_t scanline)
{
    if(DoneFlag()) {
        return;
    }
    FILE *file = GetFile();
    if(file == nullptr) {
        return;
    }

    bool frontier = (pc & 0xFFFFFFu) == FrontierPc;
    std::fprintf(file,
        "%c,%llu,%llu,%06X,%02X,%u,%04X,%04X,%04X,%04X,%02X,%04X,%02X,%u,%u,,,\n",
        frontier ? 'F' : 'I',
        static_cast<unsigned long long>(masterClock),
        static_cast<unsigned long long>(cpuCycle),
        pc & 0xFFFFFFu, opcode, emulationMode ? 1u : 0u,
        a, x, y, d, dbr, sp, ps, hclock, scanline);
    if(frontier) {
        DoneFlag() = true;
        std::fflush(file);
    }
}

inline void Read(uint64_t masterClock, uint64_t cpuCycle, uint32_t pc,
    uint32_t address, uint8_t value, uint32_t operationType, uint16_t hclock,
    uint16_t scanline)
{
    if(DoneFlag() || !IsObservedRegister(address)) {
        return;
    }
    FILE *file = GetFile();
    if(file == nullptr) {
        return;
    }
    const char *event = operationType == 4u ? "DR" : "R";
    std::fprintf(file, "%s,%llu,%llu,%06X,,,,,,,,,,%u,%u,%06X,%02X,%u\n",
        event,
        static_cast<unsigned long long>(masterClock),
        static_cast<unsigned long long>(cpuCycle),
        pc & 0xFFFFFFu, hclock, scanline, address & 0xFFFFFFu, value,
        operationType);
}

inline void Write(uint64_t masterClock, uint64_t cpuCycle, uint32_t pc,
    uint32_t address, uint8_t value, uint32_t operationType, uint16_t hclock,
    uint16_t scanline)
{
    uint8_t bank = static_cast<uint8_t>(address >> 16);
    bool registerWrite = IsObservedRegister(address);
    bool wramWrite = (bank == 0x7Eu || bank == 0x7Fu);
    if(DoneFlag() || (!registerWrite && !(wramWrite && RecordWramWrites()))) {
        return;
    }

    FILE *file = GetFile();
    if(file == nullptr) {
        return;
    }
    const char *event = operationType == 5u ? "DW" : "W";
    std::fprintf(file, "%s,%llu,%llu,%06X,,,,,,,,,,%u,%u,%06X,%02X,%u\n",
        event,
        static_cast<unsigned long long>(masterClock),
        static_cast<unsigned long long>(cpuCycle),
        pc & 0xFFFFFFu, hclock, scanline, address & 0xFFFFFFu, value,
        operationType);
}

inline void Flush()
{
    FILE *file = GetFile();
    if(file != nullptr) {
        std::fflush(file);
    }
}

} // namespace CivilizationOracleTrace
