-- Civilization Version 04 comparison-only session marker.
-- The patched MesenCE source logger is the authoritative export path for this oracle session.
local expectedRomSha = "de2d5a952096c5f50368b9270d342aa6e7a39007ffbec27117e182e30ef4cf32"
local output = "civilization-v04-oracle-session.log"
local function log(line)
  local f = io.open(output, "a")
  if f then f:write(line .. "\n"); f:close() end
end
log("Civilization V04 MesenCE 2.2.1 oracle-only comparison")
log("Expected ROM SHA-256: " .. expectedRomSha)
log("Compare reset 00:804A through instruction boundary D4:80C7 before CLI.")
log("Expected independent DMA total: 66,048 bytes in two channel-0 runs.")
log("Do not import emulator state, targets or timing constants into production generation.")
