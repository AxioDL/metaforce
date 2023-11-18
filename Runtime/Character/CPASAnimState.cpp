#include "Runtime/Character/CPASAnimState.hpp"

#include "Runtime/CRandom16.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CharacterCommon.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"

#include <algorithm>
#include <cfloat>
#include <cmath>

#include <zeus/Math.hpp>

namespace metaforce {

CPASAnimState::CPASAnimState(CInputStream& in) {
  x0_id = static_cast<pas::EAnimationState>(in.ReadLong());
  u32 parmCount = in.ReadLong();
  u32 animCount = in.ReadLong();

  x4_parms.reserve(parmCount);
  x14_anims.reserve(animCount);
  x24_selectionCache.reserve(animCount);

  for (u32 i = 0; i < parmCount; ++i)
    x4_parms.emplace_back(in);

  for (u32 i = 0; i < animCount; ++i) {
    s32 id = in.ReadLong();
    rstl::reserved_vector<CPASAnimParm::UParmValue, 8> parms;
    for (const CPASParmInfo& parm : x4_parms) {
      CPASAnimParm::UParmValue val = {};
      switch (parm.GetParameterType()) {
      case CPASAnimParm::EParmType::Int32:
        val.m_int = in.ReadInt32();
        break;
      case CPASAnimParm::EParmType::UInt32:
        val.m_uint = in.ReadLong();
        break;
      case CPASAnimParm::EParmType::Float:
        val.m_float = in.ReadFloat();
        break;
      case CPASAnimParm::EParmType::Bool:
        val.m_bool = in.ReadBool();
        break;
      case CPASAnimParm::EParmType::Enum:
        val.m_int = in.ReadInt32();
        break;
      default:
        break;
      }
      parms.push_back(val);
    }

    auto search =
        std::lower_bound(x14_anims.begin(), x14_anims.end(), id,
                         [](const CPASAnimInfo& item, const u32& testId) -> bool { return item.GetAnimId() < testId; });
    x14_anims.emplace(search, id, std::move(parms));
  }
}

CPASAnimState::CPASAnimState(pas::EAnimationState stateId) : x0_id(stateId) {}

CPASAnimParm CPASAnimState::GetAnimParmData(s32 animId, size_t parmIdx) const {
  const auto search = rstl::binary_find(x14_anims.cbegin(), x14_anims.cend(), animId,
                                        [](const CPASAnimInfo& item) { return item.GetAnimId(); });
  if (search == x14_anims.cend()) {
    return CPASAnimParm::NoParameter();
  }

  const CPASParmInfo& parm = x4_parms.at(parmIdx);
  return search->GetAnimParmData(parmIdx, parm.GetParameterType());
}

s32 CPASAnimState::PickRandomAnimation(CRandom16& rand) const {
  if (x24_selectionCache.size() == 1)
    return x24_selectionCache[0];

  if (x24_selectionCache.size() > 1) {
    u32 idx = u32(floor(rand.Float()));
    if (idx == x24_selectionCache.size())
      idx--;

    return x24_selectionCache[idx];
  }

  return -1;
}

std::pair<float, s32> CPASAnimState::FindBestAnimation(const rstl::reserved_vector<CPASAnimParm, 8>& parms,
                                                       CRandom16& rand, s32 ignoreAnim) const {
  x24_selectionCache.clear();
  float weight = -1.f;

  for (const CPASAnimInfo& info : x14_anims) {
    if (info.GetAnimId() == ignoreAnim)
      continue;

    float calcWeight = 1.f;
    if (x4_parms.size() > 0)
      calcWeight = 0.f;

    u32 unweightedCount = 0;

    for (size_t i = 0; i < x4_parms.size(); ++i) {
      CPASAnimParm::UParmValue val = info.GetAnimParmValue(i);
      const CPASParmInfo& parmInfo = x4_parms[i];
      float parmWeight = parmInfo.GetParameterWeight();

      float computedWeight = 0.f;
      switch (parmInfo.GetWeightFunction()) {
      case CPASParmInfo::EWeightFunction::AngularPercent:
        computedWeight = ComputeAngularPercentErrorWeight(i, parms[i], val);
        break;
      case CPASParmInfo::EWeightFunction::ExactMatch:
        computedWeight = ComputeExactMatchWeight(i, parms[i], val);
        break;
      case CPASParmInfo::EWeightFunction::PercentError:
        computedWeight = ComputePercentErrorWeight(i, parms[i], val);
        break;
      case CPASParmInfo::EWeightFunction::NoWeight:
        unweightedCount++;
        break;
      default:
        break;
      }

      calcWeight += parmWeight * computedWeight;
    }

    if (unweightedCount == x4_parms.size())
      calcWeight = 1.0f;

    if (calcWeight > weight) {
      x24_selectionCache.clear();
      x24_selectionCache.push_back(info.GetAnimId());
      weight = calcWeight;
    } else if (weight == calcWeight) {
      x24_selectionCache.push_back(info.GetAnimId());
      weight = calcWeight;
    }
  }

  return {weight, PickRandomAnimation(rand)};
}

float CPASAnimState::ComputeExactMatchWeight(size_t, const CPASAnimParm& parm, CPASAnimParm::UParmValue parmVal) const {
  switch (parm.GetParameterType()) {
  case CPASAnimParm::EParmType::Int32:
    return (parm.GetInt32Value() == parmVal.m_int ? 1.f : 0.f);
  case CPASAnimParm::EParmType::UInt32:
    return (parm.GetUint32Value() == parmVal.m_uint ? 1.f : 0.f);
  case CPASAnimParm::EParmType::Float:
    return ((parmVal.m_float - parm.GetReal32Value()) < FLT_EPSILON ? 1.f : 0.f);
  case CPASAnimParm::EParmType::Bool:
    return (parm.GetBoolValue() == parmVal.m_bool ? 1.f : 0.f);
  case CPASAnimParm::EParmType::Enum:
    return (parm.GetEnumValue() == parmVal.m_int ? 1.f : 0.f);
  default:
    break;
  }

  return 0.f;
}

float CPASAnimState::ComputePercentErrorWeight(size_t idx, const CPASAnimParm& parm,
                                               CPASAnimParm::UParmValue parmVal) const {
  float range = 0.f;
  float val = 0.f;

  switch (parm.GetParameterType()) {
  case CPASAnimParm::EParmType::Int32: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_int - info.GetWeightMinValue().m_int;
    val = std::fabs(parm.GetInt32Value() - parmVal.m_int);
    break;
  }
  case CPASAnimParm::EParmType::UInt32: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_uint - info.GetWeightMinValue().m_uint;
    val = std::fabs(int(parmVal.m_uint) - int(parm.GetUint32Value()));
    break;
  }
  case CPASAnimParm::EParmType::Float: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_float - info.GetWeightMinValue().m_float;
    val = std::fabs(parm.GetReal32Value() - parmVal.m_float);
    break;
  }
  case CPASAnimParm::EParmType::Bool: {
    val = parm.GetBoolValue() == parmVal.m_bool ? 0.f : 1.f;
    break;
  }
  case CPASAnimParm::EParmType::Enum: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_int - info.GetWeightMinValue().m_int;
    val = std::fabs(parm.GetEnumValue() - parmVal.m_int);
    break;
  }
  default:
    break;
  }

  if (range > FLT_EPSILON)
    return 1.f - val / range;

  return (val < FLT_EPSILON ? 1.f : 0.f);
}

float CPASAnimState::ComputeAngularPercentErrorWeight(size_t idx, const CPASAnimParm& parm,
                                                      CPASAnimParm::UParmValue parmVal) const {
  float range = 0.f;
  float val = 0.f;

  switch (parm.GetParameterType()) {
  case CPASAnimParm::EParmType::Int32: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_int - info.GetWeightMinValue().m_int;
    val = std::fabs(parmVal.m_int - parm.GetInt32Value());
    break;
  }
  case CPASAnimParm::EParmType::UInt32: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_uint - info.GetWeightMinValue().m_uint;
    val = std::fabs(int(parmVal.m_uint) - int(parm.GetUint32Value()));
    break;
  }
  case CPASAnimParm::EParmType::Float: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_float - info.GetWeightMinValue().m_float;
    val = std::fabs(parm.GetReal32Value() - parmVal.m_float);
    break;
  }
  case CPASAnimParm::EParmType::Bool: {
    val = parm.GetBoolValue() == parmVal.m_bool ? 0.f : 1.f;
    break;
  }
  case CPASAnimParm::EParmType::Enum: {
    const CPASParmInfo& info = x4_parms[idx];
    range = info.GetWeightMaxValue().m_int - info.GetWeightMinValue().m_int + 1;
    val = std::fabs(parm.GetEnumValue() - parmVal.m_int);
    break;
  }
  default:
    break;
  }

  if (range > FLT_EPSILON) {
    float mid = 0.5f * range;
    float offset = 1.f - ((val > mid ? range - val : val) / mid);
    return zeus::clamp(0.f, offset, 1.f);
  }

  return (val >= FLT_EPSILON ? 0.f : 1.f);
}
} // namespace metaforce
