/* AUTO-GENERATED compact exact-context AOT shard. */
#include "civilization_internal.h"
#include "civilization_generated_core.h"
#include "civilization_compact_aot.h"

typedef struct CivCompactRecord {uint32_t key;uint16_t parameter_offset;uint16_t template_id;} CivCompactRecord;
static const CivCompactRecord records[4]={
  {0x06D40000u,0x0000u,0x03Bu},
  {0x06D40004u,0x0002u,0x001u},
  {0x06D40006u,0x0005u,0x03Bu},
  {0x06D4000Au,0x0007u,0x009u},
};
static const uint32_t parameters[11]={
  0x00002100u,0x00000004u,0x00000001u,0x00000001u,0x00000006u,0x0000420Du,0x0000000Au,0x0000004Cu,
  0x00000003u,0x0000800Eu,0x0000800Eu,
};
int civilization_core_group_03500(CivRecomp *i,uint32_t key){
  unsigned lo=0u,hi=4u;
  while(lo<hi){unsigned mid=lo+(hi-lo)/2u;uint32_t found=records[mid].key;
    if(key<found)hi=mid;else if(key>found)lo=mid+1u;
    else return civ_compact_aot_execute(i,records[mid].template_id,parameters+records[mid].parameter_offset);
  }
  return -1;
}
