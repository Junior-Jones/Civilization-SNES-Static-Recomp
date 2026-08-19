/* AUTO-GENERATED compact exact-context AOT shard. */
#include "civilization_internal.h"
#include "civilization_generated_core.h"
#include "civilization_compact_aot.h"

typedef struct CivCompactRecord {uint32_t key;uint16_t parameter_offset;uint16_t template_id;} CivCompactRecord;
static const CivCompactRecord records[10]={
  {0x00C8F418u,0x0000u,0x023u},
  {0x02C8F401u,0x0001u,0x006u},
  {0x02C8F404u,0x0004u,0x00Cu},
  {0x02C8F407u,0x0007u,0x001u},
  {0x02C8F409u,0x000Au,0x002u},
  {0x02C8F40Cu,0x000Du,0x006u},
  {0x02C8F40Fu,0x0010u,0x00Cu},
  {0x02C8F412u,0x0013u,0x001u},
  {0x02C8F414u,0x0016u,0x002u},
  {0x02C8F417u,0x0019u,0x00Du},
};
static const uint32_t parameters[26]={
  0x0000076Bu,0x00002220u,0x00002220u,0x0000F404u,0x00004302u,0x00004302u,0x0000F407u,0x0000007Eu,
  0x0000007Eu,0x0000F409u,0x00004304u,0x00004304u,0x0000F40Cu,0x00000100u,0x00000100u,0x0000F40Fu,
  0x00004305u,0x00004305u,0x0000F412u,0x00000001u,0x00000001u,0x0000F414u,0x0000420Bu,0x0000420Bu,
  0x0000F417u,0x0000F418u,
};
int civilization_core_group_0323D(CivRecomp *i,uint32_t key){
  unsigned lo=0u,hi=10u;
  while(lo<hi){unsigned mid=lo+(hi-lo)/2u;uint32_t found=records[mid].key;
    if(key<found)hi=mid;else if(key>found)lo=mid+1u;
    else return civ_compact_aot_execute(i,records[mid].template_id,parameters+records[mid].parameter_offset);
  }
  return -1;
}
