#include "Kyoto/Animation/CPASAnimState.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/algorithm.hpp"
#include "rstl/math.hpp"

CPASAnimState::CPASAnimState(const int id) : mId(static_cast< pas::EAnimationState >(id)) {}

CPASAnimState::CPASAnimState(CInputStream& in) : mId(pas::kAS_Invalid) {
  mId = static_cast< pas::EAnimationState >(in.Get< int >());
  const uint parmCount = in.Get< uint >();
  const uint animCount = in.Get< uint >();

  mParms.reserve(parmCount);
  mAnims.reserve(animCount);
  mSelectionCache.reserve(animCount);

  for (uint i = 0; i < parmCount; i++) {
    mParms.push_back(CPASParmInfo(in));
  }

  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > parms;
  for (uint i = 0; i < animCount; i++) {
    int id = in.Get< int >();
    parms.clear();
    for (uint j = 0; j < parmCount; j++) {
      CPASAnimParm::UParmValue val;
      switch (mParms[j].GetParameterType()) {
      case CPASAnimParm::kPT_Int32:
        val.m_int = in.Get< int >();
        break;
      case CPASAnimParm::kPT_UInt32:
        val.m_uint = in.Get< uint >();
        break;
      case CPASAnimParm::kPT_Float:
        val.m_float = in.Get< float >();
        break;
      case CPASAnimParm::kPT_Bool:
        val.m_bool = in.Get< bool >();
        break;
      case CPASAnimParm::kPT_Enum:
        val.m_uint = in.Get< uint >();
        break;
      default:
        val.m_int = 0;
        break;
      }

      parms.push_back(val);
    }

    CPASAnimInfo animInfo(id, parms);
    rstl::vector< CPASAnimInfo >::iterator iter =
        rstl::lower_bound(mAnims.begin(), mAnims.end(), animInfo);
    mAnims.insert(iter, animInfo);
  }
}

CPASAnimParm CPASAnimState::GetAnimParmData(int animId, uint parmIdx) const {
  AUTO(it, rstl::binary_find(mAnims.begin(), mAnims.end(), CPASAnimInfo(animId)));
  if (it != mAnims.end()) {
    return it->GetAnimParmData(parmIdx, mParms[parmIdx].GetParameterType());
  }
  return CPASAnimParm::NoParameter();
}

// The linker strips this entry point and retains its vector insertion helpers.
void CPASAnimState::AddAnimParmData(
    int animId, const rstl::reserved_vector< CPASAnimParm, 8 >& parms) {
  rstl::reserved_vector< CPASAnimParm::UParmValue, 8 > values;
  for (int i = 0; i < parms.size(); ++i) {
    values.push_back(parms[i].GetParameterValue());
  }

  CPASAnimInfo animInfo(animId, values);
  AUTO(it, rstl::lower_bound(mAnims.begin(), mAnims.end(), animInfo));
  mAnims.insert(it, animInfo);
}

rstl::pair< float, int >
CPASAnimState::FindBestAnimation(const rstl::reserved_vector< CPASAnimParm, 8 >& parms,
                                 CRandom16& random, int ignoreAnim) const {
  float weight = -1.f;
  mSelectionCache.clear();
  if (HasAnims()) {
    for (AUTO(it, mAnims.begin()); it != mAnims.end(); ++it) {
      if (it->GetAnimId() == ignoreAnim) {
        continue;
      }

      float animWeight = mParms.size() > 0 ? 0.f : 1.f;
      int unweightedCount = 0;
      for (uint i = 0; i < mParms.size(); ++i) {
        const CPASAnimParm::UParmValue& value = it->GetAnimParmValue(i);
        const CPASParmInfo& info = mParms[i];
        const float parmWeight = info.GetParameterWeight();
        float computedWeight = 0.f;
        switch (info.GetWeightFunction()) {
        case CPASParmInfo::kWF_ExactMatch:
          computedWeight = ComputeExactMatchWeight(i, parms[i], value);
          break;
        case CPASParmInfo::kWF_PercentError:
          computedWeight = ComputePercentErrorWeight(i, parms[i], value);
          break;
        case CPASParmInfo::kWF_AngularPercent:
          computedWeight = ComputeAngularPercentErrorWeight(i, parms[i], value);
          break;
        case CPASParmInfo::kWF_NoWeight:
          ++unweightedCount;
          break;
        }
        animWeight += parmWeight * computedWeight;
      }

      if (unweightedCount == mParms.size()) {
        animWeight = 1.f;
      }
      if (animWeight > weight) {
        mSelectionCache.clear();
        mSelectionCache.push_back(it->GetAnimId());
        weight = animWeight;
      } else if (animWeight == weight) {
        mSelectionCache.push_back(it->GetAnimId());
        weight = animWeight;
      }
    }
  }
  return rstl::pair< float, int >(weight, PickRandomAnimation(random));
}

float CPASAnimState::ComputeExactMatchWeight(uint idx, const CPASAnimParm& parm,
                                             CPASAnimParm::UParmValue value) const {
  float weight = 0.f;
  switch (parm.GetParameterType()) {
  case CPASAnimParm::kPT_Int32:
    weight = value.m_int == parm.GetInt32Value() ? 1.f : 0.f;
    break;
  case CPASAnimParm::kPT_UInt32:
    weight = value.m_uint == parm.GetUint32Value() ? 1.f : 0.f;
    break;
  case CPASAnimParm::kPT_Float:
    weight = CMath::AbsF(parm.GetReal32Value() - value.m_float) < FLT_EPSILON ? 1.f : 0.f;
    break;
  case CPASAnimParm::kPT_Bool:
    weight = value.m_bool == parm.GetBoolValue() ? 1.f : 0.f;
    break;
  case CPASAnimParm::kPT_Enum:
    weight = value.m_int == parm.GetEnumValue() ? 1.f : 0.f;
    break;
  }
  return weight;
}

float CPASAnimState::ComputePercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                               CPASAnimParm::UParmValue value) const {
  float range = 0.f;
  float error = 0.f;
  switch (parm.GetParameterType()) {
  case CPASAnimParm::kPT_Int32:
    range = mParms[idx].GetParameterMaxValue().m_int - mParms[idx].GetParameterMinValue().m_int;
    error = CMath::AbsF(parm.GetInt32Value() - value.m_int);
    break;
  case CPASAnimParm::kPT_UInt32:
    range =
        mParms[idx].GetParameterMaxValue().m_uint - mParms[idx].GetParameterMinValue().m_uint;
    if (parm.GetUint32Value() > value.m_uint) {
      error = parm.GetUint32Value() - value.m_uint;
    } else {
      error = value.m_uint - parm.GetUint32Value();
    }
    break;
  case CPASAnimParm::kPT_Float:
    range =
        mParms[idx].GetParameterMaxValue().m_float - mParms[idx].GetParameterMinValue().m_float;
    error = CMath::AbsF(parm.GetReal32Value() - value.m_float);
    break;
  case CPASAnimParm::kPT_Bool:
    error = value.m_bool == parm.GetBoolValue() ? 0.f : 1.f;
    break;
  case CPASAnimParm::kPT_Enum:
    range = mParms[idx].GetParameterMaxValue().m_int - mParms[idx].GetParameterMinValue().m_int;
    error = CMath::AbsF(parm.GetEnumValue() - value.m_int);
    break;
  }
  if (range > FLT_EPSILON) {
    return 1.f - error / range;
  }
  return error < FLT_EPSILON ? 1.f : 0.f;
}

float CPASAnimState::ComputeAngularPercentErrorWeight(uint idx, const CPASAnimParm& parm,
                                                      CPASAnimParm::UParmValue value) const {
  float range = 0.f;
  float error = 0.f;
  switch (parm.GetParameterType()) {
  case CPASAnimParm::kPT_Int32:
    range = mParms[idx].GetParameterMaxValue().m_int - mParms[idx].GetParameterMinValue().m_int;
    error = CMath::AbsF(parm.GetInt32Value() - value.m_int);
    break;
  case CPASAnimParm::kPT_UInt32: {
    range =
        mParms[idx].GetParameterMaxValue().m_uint - mParms[idx].GetParameterMinValue().m_uint;
    const uint parmValue = parm.GetUint32Value();
    error = parmValue > value.m_uint ? parmValue - value.m_uint : value.m_uint - parmValue;
    break;
  }
  case CPASAnimParm::kPT_Float:
    range =
        mParms[idx].GetParameterMaxValue().m_float - mParms[idx].GetParameterMinValue().m_float;
    error = CMath::AbsF(parm.GetReal32Value() - value.m_float);
    break;
  case CPASAnimParm::kPT_Bool:
    error = value.m_bool == parm.GetBoolValue() ? 0.f : 1.f;
    break;
  case CPASAnimParm::kPT_Enum:
    range =
        mParms[idx].GetParameterMaxValue().m_int - mParms[idx].GetParameterMinValue().m_int + 1;
    error = CMath::AbsF(parm.GetEnumValue() - value.m_int);
    break;
  }
  if (range > FLT_EPSILON) {
    return rstl::min_val(
        rstl::max_val(1.f - (error > 0.5f * range ? range - error : error) / (0.5f * range), 0.f),
        1.f);
  }
  return error < FLT_EPSILON ? 1.f : 0.f;
}

int CPASAnimState::PickRandomAnimation(CRandom16& random) const {
  int anim = -1;
  switch (mSelectionCache.size()) {
  case 0:
    break;
  case 1:
    anim = mSelectionCache[0];
    break;
  default: {
    int idx = static_cast< int >(random.Float() * mSelectionCache.size());
    if (idx == mSelectionCache.size()) {
      --idx;
    }
    anim = mSelectionCache[idx];
    break;
  }
  }
  return anim;
}
