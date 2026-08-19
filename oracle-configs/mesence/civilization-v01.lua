-- Civilization Version 01 comparison-only session marker.
-- This file emits no generated source, contexts, constants, or endpoint authority.
local expectedRomSha = "de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32"
local output = "civilization-v01-oracle-comparison.log"
local function log(line)
  local f = io.open(output, "a")
  if f then f:write(line .. "\n"); f:close() end
end
log("Civilization V01 MesenCE 2.2.1 oracle-only comparison")
log("Expected ROM SHA-256: " .. expectedRomSha)
log("Compare reset 00:804A, JML D4:8000, $4200=$00, $2100=$80, $420D=$01.")
log("Stop at independently unimplemented MVN D4:802E or D4:8050; do not import emulator state.")
