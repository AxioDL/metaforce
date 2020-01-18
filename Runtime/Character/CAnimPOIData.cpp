#include "Runtime/Character/CAnimPOIData.hpp"

#include "Runtime/CToken.hpp"

namespace urde {

CAnimPOIData::CAnimPOIData(CInputStream& in) : x0_version(in.readUint32Big()) {
  u32 boolCount = in.readUint32Big();
  x4_boolNodes.reserve(boolCount);
  for (u32 i = 0; i < boolCount; ++i)
    x4_boolNodes.emplace_back(in);

  u32 int32Count = in.readUint32Big();
  x14_int32Nodes.reserve(int32Count);
  for (u32 i = 0; i < int32Count; ++i)
    x14_int32Nodes.emplace_back(in);

  u32 particleCount = in.readUint32Big();
  x24_particleNodes.reserve(particleCount);
  for (u32 i = 0; i < particleCount; ++i)
    x24_particleNodes.emplace_back(in);

  if (x0_version >= 2) {
    u32 soundCount = in.readUint32Big();
    x34_soundNodes.reserve(soundCount);
    for (u32 i = 0; i < soundCount; ++i)
      x34_soundNodes.emplace_back(in);
  }
}

CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& parms,
                                    CObjectReference* selfRef) {
  return TToken<CAnimPOIData>::GetIObjObjectFor(std::make_unique<CAnimPOIData>(in));
}

} // namespace urde
