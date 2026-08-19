/* AUTO-GENERATED compact exact-context AOT shard. */
#include "civilization_internal.h"
#include "civilization_generated_core.h"
#include "civilization_compact_aot.h"

typedef struct CivCompactRecord {uint32_t key;uint16_t parameter_offset;uint16_t template_id;} CivCompactRecord;
static const CivCompactRecord records[8]={
  {0x00D49001u,0x0000u,0x01Cu},
  {0x00D49004u,0x0003u,0x003u},
  {0x00D49008u,0x0009u,0x023u},
  {0x00D49009u,0x000Au,0x024u},
  {0x00D4900Cu,0x000Du,0x005u},
  {0x00D4900Fu,0x0010u,0x008u},
  {0x00D49012u,0x0013u,0x037u},
  {0x00D49015u,0x0016u,0x023u},
};
static const uint32_t parameters[23]={
  0x000000D4u,0x000000D4u,0x00009004u,0x00D482A2u,0x000000D4u,0x00000090u,0x00000007u,0x000000D4u,
  0x000082A2u,0x0000083Bu,0x00004854u,0x00004854u,0x0000900Cu,0x00000078u,0x00000078u,0x0000900Fu,
  0x000045FCu,0x000045FCu,0x00009012u,0x00001CF3u,0x00001CF3u,0x00009015u,0x0000083Cu,
};
int civilization_core_group_03524(CivRecomp *i,uint32_t key){
  unsigned lo=0u,hi=8u;
  while(lo<hi){unsigned mid=lo+(hi-lo)/2u;uint32_t found=records[mid].key;
    if(key<found)hi=mid;else if(key>found)lo=mid+1u;
    else return civ_compact_aot_execute(i,records[mid].template_id,parameters+records[mid].parameter_offset);
  }
  return -1;
}
