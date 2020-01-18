#include "Runtime/Character/CAnimTreeTimeScale.hpp"

namespace urde {

CAnimTreeTimeScale::CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node, float scale, std::string_view name)
: CAnimTreeSingleChild(node, name)
, x18_timeScale(new CConstantAnimationTimeScale(scale))
, x28_targetAccelTime(CCharAnimTime::Infinity()) {}

CAnimTreeTimeScale::CAnimTreeTimeScale(const std::weak_ptr<CAnimTreeNode>& node,
                                       std::unique_ptr<IVaryingAnimationTimeScale>&& timeScale,
                                       const CCharAnimTime& time, std::string_view name)
: CAnimTreeSingleChild(node, name), x18_timeScale(std::move(timeScale)), x28_targetAccelTime(time) {
  x30_initialTime = x14_child->VGetSteadyStateAnimInfo().GetDuration() - x14_child->VGetTimeRemaining();
}

std::string CAnimTreeTimeScale::CreatePrimitiveName(const std::weak_ptr<CAnimTreeNode>&, float, const CCharAnimTime&,
                                                    float) {
  return {};
}

CCharAnimTime CAnimTreeTimeScale::GetRealLifeTime(const CCharAnimTime& time) const {
  CCharAnimTime timeRem = x14_child->VGetTimeRemaining();

  CCharAnimTime ret = std::min(timeRem, time);
  if (x28_targetAccelTime > CCharAnimTime()) {
    if (ret < CCharAnimTime(x28_targetAccelTime - x20_curAccelTime))
      return x18_timeScale->VTimeScaleIntegral(x20_curAccelTime.GetSeconds(), (x20_curAccelTime + ret).GetSeconds());
    else {
      CCharAnimTime integral =
          x18_timeScale->VTimeScaleIntegral(x20_curAccelTime.GetSeconds(), x28_targetAccelTime.GetSeconds());

      if (integral > ret)
        return x18_timeScale->VFindUpperLimit(x20_curAccelTime.GetSeconds(), ret.GetSeconds()) -
               x20_curAccelTime.GetSeconds();
      else
        return integral + (ret - integral);
    }
  }

  return ret;
}

void CAnimTreeTimeScale::VSetPhase(float phase) { x14_child->VSetPhase(phase); }

std::optional<std::unique_ptr<IAnimReader>> CAnimTreeTimeScale::VSimplified() {
  if (auto simp = x14_child->Simplified()) {
    auto newNode = std::make_unique<CAnimTreeTimeScale>(CAnimTreeNode::Cast(std::move(*simp)), x18_timeScale->Clone(),
                                                        x28_targetAccelTime, x4_name);
    newNode->x20_curAccelTime = x20_curAccelTime;
    newNode->x30_initialTime = x30_initialTime;
    return {std::move(newNode)};
  }

  if (x20_curAccelTime == x28_targetAccelTime) {
    return {x14_child->Clone()};
  }

  return std::nullopt;
}

u32 CAnimTreeTimeScale::VGetBoolPOIList(const CCharAnimTime& time, CBoolPOINode* listOut, u32 capacity, u32 iterator,
                                        u32 unk) const {
  CCharAnimTime useTime = (time == CCharAnimTime::Infinity()) ? x14_child->VGetTimeRemaining() : GetRealLifeTime(time);
  u32 ret = x14_child->GetBoolPOIList(useTime, listOut, capacity, iterator, unk);
  if (x28_targetAccelTime > CCharAnimTime())
    for (u32 i = 0; i < ret; ++i)
      listOut[iterator + i].SetTime(GetRealLifeTime(listOut[i].GetTime()));
  return ret;
}

u32 CAnimTreeTimeScale::VGetInt32POIList(const CCharAnimTime& time, CInt32POINode* listOut, u32 capacity, u32 iterator,
                                         u32 unk) const {
  CCharAnimTime useTime = (time == CCharAnimTime::Infinity()) ? x14_child->VGetTimeRemaining() : GetRealLifeTime(time);
  u32 ret = x14_child->GetInt32POIList(useTime, listOut, capacity, iterator, unk);
  if (x28_targetAccelTime > CCharAnimTime())
    for (u32 i = 0; i < ret; ++i)
      listOut[iterator + i].SetTime(GetRealLifeTime(listOut[i].GetTime()));
  return ret;
}

u32 CAnimTreeTimeScale::VGetParticlePOIList(const CCharAnimTime& time, CParticlePOINode* listOut, u32 capacity,
                                            u32 iterator, u32 unk) const {
  CCharAnimTime useTime = (time == CCharAnimTime::Infinity()) ? x14_child->VGetTimeRemaining() : GetRealLifeTime(time);
  u32 ret = x14_child->GetParticlePOIList(useTime, listOut, capacity, iterator, unk);
  if (x28_targetAccelTime > CCharAnimTime())
    for (u32 i = 0; i < ret; ++i)
      listOut[iterator + i].SetTime(GetRealLifeTime(listOut[i].GetTime()));
  return ret;
}

u32 CAnimTreeTimeScale::VGetSoundPOIList(const CCharAnimTime& time, CSoundPOINode* listOut, u32 capacity, u32 iterator,
                                         u32 unk) const {
  CCharAnimTime useTime = (time == CCharAnimTime::Infinity()) ? x14_child->VGetTimeRemaining() : GetRealLifeTime(time);
  u32 ret = x14_child->GetSoundPOIList(useTime, listOut, capacity, iterator, unk);
  if (x28_targetAccelTime > CCharAnimTime())
    for (u32 i = 0; i < ret; ++i)
      listOut[iterator + i].SetTime(GetRealLifeTime(listOut[i].GetTime()));
  return ret;
}

bool CAnimTreeTimeScale::VGetBoolPOIState(const char* name) const { return x14_child->VGetBoolPOIState(name); }

s32 CAnimTreeTimeScale::VGetInt32POIState(const char* name) const { return x14_child->VGetInt32POIState(name); }

CParticleData::EParentedMode CAnimTreeTimeScale::VGetParticlePOIState(const char* name) const {
  return x14_child->VGetParticlePOIState(name);
}

CAnimTreeEffectiveContribution CAnimTreeTimeScale::VGetContributionOfHighestInfluence() const {
  CAnimTreeEffectiveContribution c = x14_child->VGetContributionOfHighestInfluence();
  return {c.GetContributionWeight(), c.GetPrimitiveName(), VGetSteadyStateAnimInfo(), VGetTimeRemaining(),
          c.GetAnimDatabaseIndex()};
}

std::shared_ptr<IAnimReader> CAnimTreeTimeScale::VGetBestUnblendedChild() const {
  if (std::shared_ptr<IAnimReader> bestChild = x14_child->VGetBestUnblendedChild()) {
    auto newNode = std::make_shared<CAnimTreeTimeScale>(CAnimTreeNode::Cast(bestChild->Clone()),
                                                         x18_timeScale->Clone(), x28_targetAccelTime, x4_name);
    newNode->x20_curAccelTime = x20_curAccelTime;
    newNode->x30_initialTime = x30_initialTime;
    return {std::move(newNode)};
  }
  return nullptr;
}

std::unique_ptr<IAnimReader> CAnimTreeTimeScale::VClone() const {
  auto newNode = std::make_unique<CAnimTreeTimeScale>(CAnimTreeNode::Cast(x14_child->Clone()), x18_timeScale->Clone(),
                                                      x28_targetAccelTime, x4_name);
  newNode->x20_curAccelTime = x20_curAccelTime;
  newNode->x30_initialTime = x30_initialTime;
  return {std::move(newNode)};
}

CSteadyStateAnimInfo CAnimTreeTimeScale::VGetSteadyStateAnimInfo() const {
  CSteadyStateAnimInfo ssInfo = x14_child->VGetSteadyStateAnimInfo();
  if (x28_targetAccelTime == CCharAnimTime::Infinity()) {
    return {ssInfo.IsLooping(), x18_timeScale->VFindUpperLimit(0.f, ssInfo.GetDuration().GetSeconds()),
            ssInfo.GetOffset()};
  } else {
    CCharAnimTime time;
    if (x20_curAccelTime.GreaterThanZero())
      time = x18_timeScale->VTimeScaleIntegral(0.f, x20_curAccelTime.GetSeconds());
    return {ssInfo.IsLooping(), x30_initialTime + time + VGetTimeRemaining(), ssInfo.GetOffset()};
  }
}

CCharAnimTime CAnimTreeTimeScale::VGetTimeRemaining() const {
  CCharAnimTime timeRem = x14_child->VGetTimeRemaining();
  if (x28_targetAccelTime == CCharAnimTime::Infinity())
    return CCharAnimTime(x18_timeScale->VFindUpperLimit(x20_curAccelTime.GetSeconds(), timeRem.GetSeconds())) -
           x20_curAccelTime;
  else
    return GetRealLifeTime(timeRem);
}

SAdvancementResults CAnimTreeTimeScale::VAdvanceView(const CCharAnimTime& dt) {
  if (dt.EqualsZero() && dt > CCharAnimTime())
    return x14_child->VAdvanceView(dt);

  CCharAnimTime origAccelTime = x20_curAccelTime;
  CCharAnimTime newTime = x20_curAccelTime + dt;
  if (newTime < x28_targetAccelTime) {
    SAdvancementResults res =
        x14_child->VAdvanceView(x18_timeScale->VTimeScaleIntegral(origAccelTime.GetSeconds(), newTime.GetSeconds()));
    if (res.x0_remTime.EqualsZero()) {
      x20_curAccelTime = newTime;
      res.x0_remTime = CCharAnimTime();
      return res;
    } else {
      x20_curAccelTime =
          x18_timeScale->VFindUpperLimit(origAccelTime.GetSeconds(), (newTime - res.x0_remTime).GetSeconds());
      res.x0_remTime = dt - (x20_curAccelTime - origAccelTime);
      return res;
    }
  } else {
    CCharAnimTime newDt(
        x18_timeScale->VTimeScaleIntegral(origAccelTime.GetSeconds(), x28_targetAccelTime.GetSeconds()));
    SAdvancementResults res2;
    if (newDt.GreaterThanZero())
      res2 = x14_child->VAdvanceView(newDt);
    x20_curAccelTime = x28_targetAccelTime;
    res2.x0_remTime = res2.x0_remTime + (newTime - x28_targetAccelTime);
    return res2;
  }
}

} // namespace urde
