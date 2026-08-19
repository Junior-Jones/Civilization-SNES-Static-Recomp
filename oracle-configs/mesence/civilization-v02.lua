-- Civilization Version 02 comparison-only session marker.
-- The modified MesenCE source logger is the preferred deterministic export.
-- This script does not generate source, contexts, constants, or endpoint state.
local expectedRomSha = "de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32"
local output = "civilization-v02-oracle-session.log"
local function log(line)
  local f = io.open(output, "a")
  if f then f:write(line .. "\n"); f:close() end
end
log("Civilization V02 MesenCE 2.2.1 oracle-only comparison")
log("Expected ROM SHA-256: " .. expectedRomSha)
log("Compare reset 00:804A through static frontier D4:8073.")
log("Do not import emulator state into production generation.")
