#include "Runtime/Weapon/CFlameInfo.hpp"

namespace metaforce {

CFlameInfo::CFlameInfo(s32 w1, s32 w2, CAssetId flameFxId, s32 w3, float f1, float f2, float f3)
: x0_propertyCount(w1), x4_attributes(w2), x8_flameFxId(flameFxId), xc_length(w3), x10_(f1), x18_(f2), x1c_(f3) {}

CFlameInfo::CFlameInfo(CInputStream& in)
: x0_propertyCount(in.ReadLong())
, x4_attributes(in.ReadLong())
, x8_flameFxId(in)
, xc_length(in.ReadLong())
, x10_(in.ReadFloat())
, x18_(in.ReadFloat())
, x1c_(in.ReadFloat()) {}
} // namespace metaforce
