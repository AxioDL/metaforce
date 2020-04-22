#include "Runtime/Character/CAnimData.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/Character/CAdditiveAnimPlayback.hpp"
#include "Runtime/Character/CAllFormatsAnimSource.hpp"
#include "Runtime/Character/CAnimPerSegmentData.hpp"
#include "Runtime/Character/CAnimPlaybackParms.hpp"
#include "Runtime/Character/CAnimTreeBlend.hpp"
#include "Runtime/Character/CAnimTreeNode.hpp"
#include "Runtime/Character/CAnimationManager.hpp"
#include "Runtime/Character/CBoolPOINode.hpp"
#include "Runtime/Character/CCharLayoutInfo.hpp"
#include "Runtime/Character/CCharacterFactory.hpp"
#include "Runtime/Character/CCharacterInfo.hpp"
#include "Runtime/Character/CInt32POINode.hpp"
#include "Runtime/Character/CParticleGenInfo.hpp"
#include "Runtime/Character/CParticlePOINode.hpp"
#include "Runtime/Character/CPrimitive.hpp"
#include "Runtime/Character/CSegStatementSet.hpp"
#include "Runtime/Character/CSoundPOINode.hpp"
#include "Runtime/Character/CTransitionManager.hpp"
#include "Runtime/Character/IAnimReader.hpp"
#include "Runtime/Graphics/CSkinnedModel.hpp"

#include <logvisor/logvisor.hpp>

namespace urde {
static logvisor::Module Log("CAnimData");

rstl::reserved_vector<CBoolPOINode, 8> CAnimData::g_BoolPOINodes;
rstl::reserved_vector<CInt32POINode, 16> CAnimData::g_Int32POINodes;
rstl::reserved_vector<CParticlePOINode, 20> CAnimData::g_ParticlePOINodes;
rstl::reserved_vector<CSoundPOINode, 20> CAnimData::g_SoundPOINodes;
rstl::reserved_vector<CInt32POINode, 16> CAnimData::g_TransientInt32POINodes;

void CAnimData::FreeCache() {}

void CAnimData::InitializeCache() {}

CAnimData::CAnimData(CAssetId id, const CCharacterInfo& character, int defaultAnim, int charIdx, bool loop,
                     TLockedToken<CCharLayoutInfo> layout, TToken<CSkinnedModel> model,
                     const std::optional<TToken<CMorphableSkinnedModel>>& iceModel,
                     const std::weak_ptr<CAnimSysContext>& ctx, std::shared_ptr<CAnimationManager> animMgr,
                     std::shared_ptr<CTransitionManager> transMgr, TLockedToken<CCharacterFactory> charFactory,
                     int drawInstCount)
: x0_charFactory(charFactory)
, xc_charInfo(character)
, xcc_layoutData(layout)
, xd8_modelData(std::move(model))
, xfc_animCtx(ctx.lock())
, x100_animMgr(std::move(animMgr))
, x1d8_selfId(id)
, x1fc_transMgr(std::move(transMgr))
, x204_charIdx(charIdx)
, x208_defaultAnim(defaultAnim)
, x224_pose(layout->GetSegIdList().GetList().size())
, x2fc_poseBuilder(CLayoutDescription{layout})
, m_drawInstCount(drawInstCount) {
  x220_25_loop = loop;

  if (iceModel)
    xe4_iceModelData = *iceModel;

  g_BoolPOINodes.resize(8);
  g_Int32POINodes.resize(16);
  g_ParticlePOINodes.resize(20);
  g_SoundPOINodes.resize(20);
  g_TransientInt32POINodes.resize(16);

  x108_aabb = xd8_modelData->GetModel()->GetAABB();
  x120_particleDB.CacheParticleDesc(xc_charInfo.GetParticleResData());

  CHierarchyPoseBuilder pb(CLayoutDescription{xcc_layoutData});
  pb.BuildNoScale(x224_pose);
  x220_30_poseBuilt = true;

  if (defaultAnim == -1) {
    defaultAnim = 0;
    Log.report(logvisor::Warning, FMT_STRING("Character {} has invalid initial animation, so defaulting to first."),
               character.GetCharacterName());
  }

  auto treeNode = GetAnimationManager()->GetAnimationTree(character.GetAnimationIndex(defaultAnim),
                                                          CMetaAnimTreeBuildOrders::NoSpecialOrders());
  if (treeNode != x1f8_animRoot) {
    x1f8_animRoot = std::move(treeNode);
  }
}

void CAnimData::SetParticleEffectState(std::string_view effectName, bool active, CStateManager& mgr) {
  auto search = std::find_if(xc_charInfo.x98_effects.begin(), xc_charInfo.x98_effects.end(),
                             [effectName](const auto& v) { return v.first == effectName; });
  if (search != xc_charInfo.x98_effects.end())
    for (const auto& p : search->second)
      x120_particleDB.SetParticleEffectState(p.GetComponentName(), active, mgr);
}

void CAnimData::InitializeEffects(CStateManager& mgr, TAreaId aId, const zeus::CVector3f& scale) {
  for (const auto& effects : xc_charInfo.GetEffectList()) {
    for (const auto& effect : effects.second) {
      x120_particleDB.CacheParticleDesc(effect.GetParticleTag());
      x120_particleDB.AddParticleEffect(effect.GetSegmentName(), effect.GetFlags(), CParticleData(), scale, mgr, aId,
                                        true, x21c_particleLightIdx);
      x120_particleDB.SetParticleEffectState(effect.GetComponentName(), false, mgr);
    }
  }
}

CAssetId CAnimData::GetEventResourceIdForAnimResourceId(CAssetId id) const {
  return x0_charFactory->GetEventResourceIdForAnimResourceId(id);
}

void CAnimData::AddAdditiveSegData(const CSegIdList& list, CSegStatementSet& stSet) {
  for (auto& additive : x434_additiveAnims)
    if (additive.second.GetTargetWeight() > 0.00001f)
      additive.second.AddToSegStatementSet(list, *xcc_layoutData.GetObj(), stSet);
}

SAdvancementResults CAnimData::AdvanceAdditiveAnim(std::shared_ptr<CAnimTreeNode>& anim, const CCharAnimTime& time) {
  SAdvancementResults ret = anim->VAdvanceView(time);
  auto simplified = anim->Simplified();
  if (simplified)
    anim = CAnimTreeNode::Cast(std::move(*simplified));
  return ret;
}

SAdvancementDeltas CAnimData::AdvanceAdditiveAnims(float dt) {
  CCharAnimTime time(dt);

  SAdvancementDeltas deltas = {};

  for (auto& additive : x434_additiveAnims) {
    std::shared_ptr<CAnimTreeNode>& anim = additive.second.GetAnim();
    if (additive.second.IsActive()) {
      while (time.GreaterThanZero() && std::fabs(time.GetSeconds()) >= 0.00001f) {
        x210_passedIntCount +=
            u32(anim->GetInt32POIList(time, g_Int32POINodes.data(), g_Int32POINodes.size(), x210_passedIntCount, 0));
        x20c_passedBoolCount +=
            u32(anim->GetBoolPOIList(time, g_BoolPOINodes.data(), g_BoolPOINodes.size(), x20c_passedBoolCount, 0));
        x214_passedParticleCount +=
            u32(anim->GetParticlePOIList(time, g_ParticlePOINodes.data(), 8, x214_passedParticleCount, 0));
        x218_passedSoundCount += u32(anim->GetSoundPOIList(time, g_SoundPOINodes.data(), 8, x218_passedSoundCount, 0));

        SAdvancementResults results = AdvanceAdditiveAnim(anim, time);
        deltas.x0_posDelta += results.x8_deltas.x0_posDelta;
        deltas.xc_rotDelta *= results.x8_deltas.xc_rotDelta;
        time = results.x0_remTime;
      }
    } else {
      CCharAnimTime remTime = anim->VGetTimeRemaining();
      while (remTime.GreaterThanZero() && std::fabs(remTime.GetSeconds()) >= 0.00001f) {
        x210_passedIntCount +=
            u32(anim->GetInt32POIList(time, g_Int32POINodes.data(), g_Int32POINodes.size(), x210_passedIntCount, 0));
        x20c_passedBoolCount +=
            u32(anim->GetBoolPOIList(time, g_BoolPOINodes.data(), g_BoolPOINodes.size(), x20c_passedBoolCount, 0));
        x214_passedParticleCount +=
            u32(anim->GetParticlePOIList(time, g_ParticlePOINodes.data(), 8, x214_passedParticleCount, 0));
        x218_passedSoundCount += u32(anim->GetSoundPOIList(time, g_SoundPOINodes.data(), 8, x218_passedSoundCount, 0));

        SAdvancementResults results = AdvanceAdditiveAnim(anim, time);
        deltas.x0_posDelta += results.x8_deltas.x0_posDelta;
        deltas.xc_rotDelta *= results.x8_deltas.xc_rotDelta;
        CCharAnimTime tmpTime = anim->VGetTimeRemaining();
        if (tmpTime < results.x0_remTime)
          remTime = tmpTime;
        else
          remTime = results.x0_remTime;
      }
    }
  }

  return deltas;
}

SAdvancementDeltas CAnimData::UpdateAdditiveAnims(float dt) {
  for (auto it = x434_additiveAnims.begin(); it != x434_additiveAnims.end();) {
    it->second.Update(dt);
    CCharAnimTime timeRem = it->second.GetAnim()->VGetTimeRemaining();
    if (timeRem.EpsilonZero() && it->second.NeedsFadeOut())
      it->second.FadeOut();
    if (it->second.GetPhase() == EAdditivePlaybackPhase::FadedOut) {
      it = x434_additiveAnims.erase(it);
      continue;
    }
    ++it;
  }

  return AdvanceAdditiveAnims(dt);
}

bool CAnimData::IsAdditiveAnimation(s32 idx) const {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  return x0_charFactory->HasAdditiveInfo(animIdx);
}

bool CAnimData::IsAdditiveAnimationAdded(s32 idx) const {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  return search != x434_additiveAnims.cend();
}

const std::shared_ptr<CAnimTreeNode>& CAnimData::GetAdditiveAnimationTree(s32 idx) const {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  return search->second.GetAnim();
}

bool CAnimData::IsAdditiveAnimationActive(s32 idx) const {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  if (search == x434_additiveAnims.cend())
    return false;
  return search->second.IsActive();
}

void CAnimData::DelAdditiveAnimation(s32 idx) {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.begin(), x434_additiveAnims.end(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  if (search != x434_additiveAnims.cend() && search->second.GetPhase() != EAdditivePlaybackPhase::FadingOut &&
      search->second.GetPhase() != EAdditivePlaybackPhase::FadedOut) {
    search->second.FadeOut();
  }
}

void CAnimData::AddAdditiveAnimation(s32 idx, float weight, bool active, bool fadeOut) {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.begin(), x434_additiveAnims.end(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  if (search != x434_additiveAnims.cend()) {
    search->second.SetActive(active);
    search->second.SetWeight(weight);
    search->second.SetNeedsFadeOut(!search->second.IsActive() && fadeOut);
  } else {
    std::shared_ptr<CAnimTreeNode> node =
        GetAnimationManager()->GetAnimationTree(animIdx, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    const CAdditiveAnimationInfo& info = x0_charFactory->FindAdditiveInfo(animIdx);
    x434_additiveAnims.emplace_back(
        std::make_pair(animIdx, CAdditiveAnimPlayback(node, weight, active, info, fadeOut)));
  }
}

float CAnimData::GetAdditiveAnimationWeight(s32 idx) const {
  s32 animIdx = xc_charInfo.GetAnimationIndex(idx);
  auto search = std::find_if(x434_additiveAnims.cbegin(), x434_additiveAnims.cend(),
                             [animIdx](const auto& pair) { return pair.first == animIdx; });
  if (search != x434_additiveAnims.cend())
    return search->second.GetTargetWeight();
  return 0.f;
}

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager() { return x100_animMgr; }

void CAnimData::SetPhase(float ph) { x1f8_animRoot->VSetPhase(ph); }

void CAnimData::Touch(const CSkinnedModel& model, int shadIdx) const {
  model.GetModelInst()->Touch(shadIdx);
}

SAdvancementDeltas CAnimData::GetAdvancementDeltas(const CCharAnimTime& a, const CCharAnimTime& b) const {
  return x1f8_animRoot->VGetAdvancementResults(a, b).x8_deltas;
}

CCharAnimTime CAnimData::GetTimeOfUserEvent(EUserEventType type, const CCharAnimTime& time) const {
  const size_t count =
      x1f8_animRoot->GetInt32POIList(time, g_TransientInt32POINodes.data(), g_TransientInt32POINodes.size(), 0, 64);
  for (size_t i = 0; i < count; ++i) {
    CInt32POINode& poi = g_TransientInt32POINodes[i];
    if (poi.GetPoiType() == EPOIType::UserEvent && EUserEventType(poi.GetValue()) == type) {
      CCharAnimTime ret = poi.GetTime();
      for (; i < count; ++i)
        g_TransientInt32POINodes[i] = CInt32POINode();
      return ret;
    } else {
      poi = CInt32POINode();
    }
  }
  return CCharAnimTime::Infinity();
}

void CAnimData::MultiplyPlaybackRate(float mul) { x200_speedScale *= mul; }

void CAnimData::SetPlaybackRate(float set) { x200_speedScale = set; }

void CAnimData::SetRandomPlaybackRate(CRandom16& r) {
  for (size_t i = 0; i < x210_passedIntCount; ++i) {
    const CInt32POINode& poi = g_Int32POINodes[i];
    if (poi.GetPoiType() == EPOIType::RandRate) {
      float tmp = (r.Next() % poi.GetValue()) / 100.f;
      if ((r.Next() % 100) < 50)
        x200_speedScale = 1.f + tmp;
      else
        x200_speedScale = 1.f - tmp;
      break;
    }
  }
}

void CAnimData::CalcPlaybackAlignmentParms(const CAnimPlaybackParms& parms,
                                           const std::shared_ptr<CAnimTreeNode>& node) {
  zeus::CQuaternion orient;
  x1e8_alignRot = zeus::CQuaternion();

  x220_27_ = false;
  if (parms.GetDeltaOrient() && parms.GetObjectXform()) {
    ResetPOILists();
    x210_passedIntCount += u32(node->GetInt32POIList(CCharAnimTime::Infinity(), g_Int32POINodes.data(),
                                                     g_Int32POINodes.size(), x210_passedIntCount, 64));
    for (size_t i = 0; i < x210_passedIntCount; ++i) {
      const CInt32POINode& poi = g_Int32POINodes[i];
      if (poi.GetPoiType() == EPOIType::UserEvent && EUserEventType(poi.GetValue()) == EUserEventType::AlignTargetRot) {
        SAdvancementResults res = node->VGetAdvancementResults(poi.GetTime(), 0.f);
        orient = zeus::CQuaternion::slerp(zeus::CQuaternion(),
                                          *parms.GetDeltaOrient() *
                                              zeus::CQuaternion(parms.GetObjectXform()->buildMatrix3f().inverted()) *
                                              res.x8_deltas.xc_rotDelta.inverse(),
                                          1.f / (60.f * poi.GetTime().GetSeconds()));
        x1e8_alignRot = orient;
        x220_27_ = true;
      }
    }
  }

  if (!x220_27_) {
    bool didAlign = false;
    bool didStart = false;
    zeus::CVector3f posStart, posAlign;
    CCharAnimTime timeStart, timeAlign;
    if (parms.GetTargetPos() && parms.GetObjectXform()) {
      ResetPOILists();
      x210_passedIntCount += u32(node->GetInt32POIList(CCharAnimTime::Infinity(), g_Int32POINodes.data(),
                                                       g_Int32POINodes.size(), x210_passedIntCount, 64));
      for (size_t i = 0; i < x210_passedIntCount; ++i) {
        const CInt32POINode& poi = g_Int32POINodes[i];
        if (poi.GetPoiType() == EPOIType::UserEvent) {
          if (EUserEventType(poi.GetValue()) == EUserEventType::AlignTargetPosStart) {
            didStart = true;
            SAdvancementResults res = node->VGetAdvancementResults(poi.GetTime(), 0.f);
            posStart = res.x8_deltas.x0_posDelta;
            timeStart = poi.GetTime();

            if (parms.GetIsUseLocator())
              posStart += GetLocatorTransform(poi.GetLocatorName(), &poi.GetTime()).origin;

            if (didAlign)
              break;
          } else if (EUserEventType(poi.GetValue()) == EUserEventType::AlignTargetPos) {
            didAlign = true;
            SAdvancementResults res = node->VGetAdvancementResults(poi.GetTime(), 0.f);
            posAlign = res.x8_deltas.x0_posDelta;
            timeAlign = poi.GetTime();

            if (parms.GetIsUseLocator())
              posAlign += GetLocatorTransform(poi.GetLocatorName(), &poi.GetTime()).origin;

            if (didStart)
              break;
          }
        }
      }

      if (didAlign && didStart) {
        zeus::CVector3f scaleStart = *parms.GetObjectScale() * posStart;
        zeus::CVector3f scaleAlign = *parms.GetObjectScale() * posAlign;
        x1dc_alignPos =
            (parms.GetObjectXform()->inverse() * *parms.GetTargetPos() - scaleStart - (scaleAlign - scaleStart)) /
            *parms.GetObjectScale() * (1.f / (timeAlign.GetSeconds() - timeStart.GetSeconds()));
        x220_28_ = true;
        x220_26_aligningPos = false;
      } else {
        x1dc_alignPos = zeus::skZero3f;
        x220_28_ = false;
        x220_26_aligningPos = false;
      }
    }
  } else {
    bool didStart = false;
    bool didAlign = false;
    CCharAnimTime timeStart, timeAlign;
    zeus::CVector3f startPos;
    if (parms.GetTargetPos() && parms.GetObjectXform()) {
      ResetPOILists();
      x210_passedIntCount += u32(node->GetInt32POIList(CCharAnimTime::Infinity(), g_Int32POINodes.data(),
                                                       g_Int32POINodes.size(), x210_passedIntCount, 64));
      for (size_t i = 0; i < x210_passedIntCount; ++i) {
        CInt32POINode& poi = g_Int32POINodes[i];
        if (poi.GetPoiType() == EPOIType::UserEvent) {
          if (EUserEventType(poi.GetValue()) == EUserEventType::AlignTargetPosStart) {
            didStart = true;
            timeStart = poi.GetTime();
            if (didAlign)
              break;
          } else if (EUserEventType(poi.GetValue()) == EUserEventType::AlignTargetPos) {
            didAlign = true;
            timeAlign = poi.GetTime();
            if (didStart)
              break;
          }
        }
      }

      if (didAlign && didStart) {
        CCharAnimTime frameInterval(1.f / 60.f);
        orient = zeus::CQuaternion();
        x1e8_alignRot = zeus::CQuaternion();
        x220_27_ = true;
        CCharAnimTime time;
        zeus::CVector3f pos;
        zeus::CQuaternion quat;
        bool foundStartPos = false;
        while (time < timeAlign) {
          SAdvancementResults res = node->VGetAdvancementResults(frameInterval, time);
          pos += quat.toTransform() * res.x8_deltas.x0_posDelta;
          quat *= (res.x8_deltas.xc_rotDelta * orient);
          if (!foundStartPos && time >= timeStart) {
            startPos = pos;
            foundStartPos = true;
          }
          time += frameInterval;
        }
        zeus::CVector3f scaleStart = startPos * *parms.GetObjectScale();
        zeus::CVector3f scaleAlign = pos * *parms.GetObjectScale();
        x1dc_alignPos =
            (parms.GetObjectXform()->inverse() * *parms.GetTargetPos() - scaleStart - (scaleAlign - scaleStart)) /
            *parms.GetObjectScale() * (1.f / (timeAlign.GetSeconds() - timeStart.GetSeconds()));
        x220_28_ = true;
        x220_26_aligningPos = false;
      } else {
        x1dc_alignPos = zeus::skZero3f;
        x220_28_ = false;
        x220_26_aligningPos = false;
      }
    } else {
      x1dc_alignPos = zeus::skZero3f;
      x220_28_ = false;
      x220_26_aligningPos = false;
    }
  }
}

zeus::CTransform CAnimData::GetLocatorTransform(CSegId id, const CCharAnimTime* time) const {
  if (id.IsInvalid()) {
    return {};
  }

  zeus::CTransform ret;
  if (time || !x220_31_poseCached) {
    const_cast<CAnimData*>(this)->RecalcPoseBuilder(time);
    const_cast<CAnimData*>(this)->x220_31_poseCached = time == nullptr;
  }

  if (!x220_30_poseBuilt)
    x2fc_poseBuilder.BuildTransform(id, ret);
  else {
    ret.setRotation(x224_pose.GetTransformMinusOffset(id));
    ret.origin = x224_pose.GetOffset(id);
  }
  return ret;
}

zeus::CTransform CAnimData::GetLocatorTransform(std::string_view name, const CCharAnimTime* time) const {
  return GetLocatorTransform(xcc_layoutData->GetSegIdFromString(name), time);
}

bool CAnimData::IsAnimTimeRemaining(float rem, std::string_view name) const {
  if (!x1f8_animRoot)
    return false;
  return x1f8_animRoot->VGetTimeRemaining().GetSeconds() >= rem;
}

float CAnimData::GetAnimTimeRemaining(std::string_view name) const {
  float rem = x1f8_animRoot->VGetTimeRemaining().GetSeconds();
  if (x200_speedScale)
    return rem / x200_speedScale;
  return rem;
}

float CAnimData::GetAnimationDuration(int animIn) const {
  std::shared_ptr<IMetaAnim> anim = x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(animIn));
  std::set<CPrimitive> prims;
  anim->GetUniquePrimitives(prims);

  SObjectTag tag{FOURCC('ANIM'), 0};
  float durAccum = 0.f;
  for (const CPrimitive& prim : prims) {
    tag.id = prim.GetAnimResId();
    TLockedToken<CAllFormatsAnimSource> animRes = xfc_animCtx->xc_store.GetObj(tag);

    CCharAnimTime dur;
    switch (animRes->GetFormat()) {
    case EAnimFormat::Uncompressed:
    default: {
      const CAnimSource& src = animRes->GetAsCAnimSource();
      dur = src.GetDuration();
      break;
    }
    case EAnimFormat::BitstreamCompressed:
    case EAnimFormat::BitstreamCompressed24: {
      const CFBStreamedCompression& src = animRes->GetAsCFBStreamedCompression();
      dur = src.GetAnimationDuration();
      break;
    }
    }

    durAccum += dur.GetSeconds();
  }

  if (anim->GetType() == EMetaAnimType::Random)
    return durAccum / float(prims.size());
  return durAccum;
}

std::shared_ptr<CAnimSysContext> CAnimData::GetAnimSysContext() const { return xfc_animCtx; }

std::shared_ptr<CAnimationManager> CAnimData::GetAnimationManager() const { return x100_animMgr; }

void CAnimData::RecalcPoseBuilder(const CCharAnimTime* time) {
  if (!x1f8_animRoot)
    return;

  const CSegIdList& segIdList = GetCharLayoutInfo().GetSegIdList();
  CSegStatementSet segSet;
  if (time)
    x1f8_animRoot->VGetSegStatementSet(segIdList, segSet, *time);
  else
    x1f8_animRoot->VGetSegStatementSet(segIdList, segSet);

  AddAdditiveSegData(segIdList, segSet);

  for (const CSegId& id : segIdList.GetList()) {
    if (id == 3)
      continue;
    CAnimPerSegmentData& segData = segSet[id];
    if (segData.x1c_hasOffset)
      x2fc_poseBuilder.Insert(id, segData.x0_rotation, segData.x10_offset);
    else
      x2fc_poseBuilder.Insert(id, segData.x0_rotation);
  }
}

void CAnimData::RenderAuxiliary(const zeus::CFrustum& frustum) const { x120_particleDB.AddToRendererClipped(frustum); }

void CAnimData::Render(CSkinnedModel& model, const CModelFlags& drawFlags,
                       const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes) {
  SetupRender(model, drawFlags, morphEffect, morphMagnitudes);
  DrawSkinnedModel(model, drawFlags);
}

void CAnimData::SetupRender(CSkinnedModel& model, const CModelFlags& drawFlags,
                            const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes) {
  if (!x220_30_poseBuilt) {
    x2fc_poseBuilder.BuildNoScale(x224_pose);
    x220_30_poseBuilt = true;
  }
  PoseSkinnedModel(model, x224_pose, drawFlags, morphEffect, morphMagnitudes);
}

void CAnimData::DrawSkinnedModel(CSkinnedModel& model, const CModelFlags& flags) { model.Draw(flags); }

void CAnimData::PreRender() {
  if (!x220_31_poseCached) {
    RecalcPoseBuilder(nullptr);
    x220_31_poseCached = true;
    x220_30_poseBuilt = false;
  }
}

void CAnimData::BuildPose() {
  if (!x220_31_poseCached) {
    RecalcPoseBuilder(nullptr);
    x220_31_poseCached = true;
    x220_30_poseBuilt = false;
  }

  if (!x220_30_poseBuilt) {
    x2fc_poseBuilder.BuildNoScale(x224_pose);
    x220_30_poseBuilt = true;
  }
}

void CAnimData::PrimitiveSetToTokenVector(const std::set<CPrimitive>& primSet, std::vector<CToken>& tokensOut,
                                          bool preLock) {
  tokensOut.reserve(primSet.size());

  SObjectTag tag{FOURCC('ANIM'), 0};
  for (const CPrimitive& prim : primSet) {
    tag.id = prim.GetAnimResId();
    tokensOut.push_back(g_SimplePool->GetObj(tag));
    if (preLock)
      tokensOut.back().Lock();
  }
}

void CAnimData::GetAnimationPrimitives(const CAnimPlaybackParms& parms, std::set<CPrimitive>& primsOut) const {
  std::shared_ptr<IMetaAnim> animA =
      x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(parms.GetAnimationId()));
  animA->GetUniquePrimitives(primsOut);

  if (parms.GetSecondAnimationId() != -1) {
    std::shared_ptr<IMetaAnim> animB =
        x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(parms.GetSecondAnimationId()));
    animB->GetUniquePrimitives(primsOut);
  }
}

void CAnimData::SetAnimation(const CAnimPlaybackParms& parms, bool noTrans) {
  if (parms.GetAnimationId() == x40c_playbackParms.GetAnimationId() ||
      (parms.GetSecondAnimationId() == x40c_playbackParms.GetSecondAnimationId() &&
       parms.GetSecondAnimationId() != -1) ||
      (parms.GetBlendFactor() == x40c_playbackParms.GetBlendFactor() && parms.GetBlendFactor() != 1.f)) {
    if (x220_29_animationJustStarted)
      return;
  }

  x40c_playbackParms.SetAnimationId(parms.GetAnimationId());
  x40c_playbackParms.SetSecondAnimationId(parms.GetSecondAnimationId());
  x40c_playbackParms.SetBlendFactor(parms.GetBlendFactor());
  x200_speedScale = 1.f;
  x208_defaultAnim = parms.GetAnimationId();

  s32 animIdxA = xc_charInfo.GetAnimationIndex(parms.GetAnimationId());

  ResetPOILists();

  std::shared_ptr<CAnimTreeNode> blendNode;
  if (parms.GetSecondAnimationId() != -1) {
    s32 animIdxB = xc_charInfo.GetAnimationIndex(parms.GetSecondAnimationId());

    std::shared_ptr<CAnimTreeNode> treeA =
        x100_animMgr->GetAnimationTree(animIdxA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
    std::shared_ptr<CAnimTreeNode> treeB =
        x100_animMgr->GetAnimationTree(animIdxB, CMetaAnimTreeBuildOrders::NoSpecialOrders());

    blendNode =
        std::make_shared<CAnimTreeBlend>(false, treeA, treeB, parms.GetBlendFactor(),
                                         CAnimTreeBlend::CreatePrimitiveName(treeA, treeB, parms.GetBlendFactor()));
  } else {
    blendNode = x100_animMgr->GetAnimationTree(animIdxA, CMetaAnimTreeBuildOrders::NoSpecialOrders());
  }

  if (!noTrans && x1f8_animRoot)
    x1f8_animRoot = x1fc_transMgr->GetTransitionTree(x1f8_animRoot, blendNode);
  else
    x1f8_animRoot = blendNode;

  x220_24_animating = parms.GetIsPlayAnimation();
  CalcPlaybackAlignmentParms(parms, blendNode);
  ResetPOILists();
  x220_29_animationJustStarted = true;
}

SAdvancementDeltas CAnimData::DoAdvance(float dt, bool& suspendParticles, CRandom16& random, bool advTree) {
  suspendParticles = false;

  zeus::CVector3f offsetPre, offsetPost;
  zeus::CQuaternion quatPre, quatPost;

  ResetPOILists();
  float scaleDt = dt * x200_speedScale;
  if (x2fc_poseBuilder.HasRoot()) {
    SAdvancementDeltas deltas = UpdateAdditiveAnims(scaleDt);
    offsetPre = deltas.x0_posDelta;
    quatPre = deltas.xc_rotDelta;
  }

  if (!x220_24_animating) {
    suspendParticles = true;
    return {};
  }

  if (x220_29_animationJustStarted) {
    x220_29_animationJustStarted = false;
    suspendParticles = true;
  }

  if (advTree && x1f8_animRoot) {
    SetRandomPlaybackRate(random);
    CCharAnimTime time(scaleDt);
    if (x220_25_loop) {
      while (time.GreaterThanZero() && !time.EpsilonZero()) {
        x210_passedIntCount += u32(x1f8_animRoot->GetInt32POIList(time, g_Int32POINodes.data(), g_Int32POINodes.size(),
                                                                  x210_passedIntCount, 0));
        x20c_passedBoolCount += u32(
            x1f8_animRoot->GetBoolPOIList(time, g_BoolPOINodes.data(), g_BoolPOINodes.size(), x20c_passedBoolCount, 0));
        x214_passedParticleCount +=
            u32(x1f8_animRoot->GetParticlePOIList(time, g_ParticlePOINodes.data(), 16, x214_passedParticleCount, 0));
        x218_passedSoundCount +=
            u32(x1f8_animRoot->GetSoundPOIList(time, g_SoundPOINodes.data(), 16, x218_passedSoundCount, 0));
        AdvanceAnim(time, offsetPost, quatPost);
      }
    } else {
      CCharAnimTime remTime = x1f8_animRoot->VGetTimeRemaining();
      while (!remTime.EpsilonZero() && !time.EpsilonZero()) {
        x210_passedIntCount += u32(x1f8_animRoot->GetInt32POIList(time, g_Int32POINodes.data(), g_Int32POINodes.size(),
                                                                  x210_passedIntCount, 0));
        x20c_passedBoolCount += u32(
            x1f8_animRoot->GetBoolPOIList(time, g_BoolPOINodes.data(), g_BoolPOINodes.size(), x20c_passedBoolCount, 0));
        x214_passedParticleCount +=
            u32(x1f8_animRoot->GetParticlePOIList(time, g_ParticlePOINodes.data(), 16, x214_passedParticleCount, 0));
        x218_passedSoundCount +=
            u32(x1f8_animRoot->GetSoundPOIList(time, g_SoundPOINodes.data(), 16, x218_passedSoundCount, 0));
        AdvanceAnim(time, offsetPost, quatPost);
        remTime = x1f8_animRoot->VGetTimeRemaining();
        time = std::max(0.f, std::min(remTime.GetSeconds(), time.GetSeconds()));
        if (remTime.EpsilonZero()) {
          x220_24_animating = false;
          x1dc_alignPos = zeus::skZero3f;
          x220_28_ = false;
          x220_26_aligningPos = false;
        }
      }
    }

    x220_31_poseCached = false;
    x220_30_poseBuilt = false;
  }

  return {offsetPost + offsetPre, quatPost * quatPre};
}

SAdvancementDeltas CAnimData::Advance(float dt, const zeus::CVector3f& scale, CStateManager& stateMgr, TAreaId aid,
                                      bool advTree) {
  bool suspendParticles;
  SAdvancementDeltas deltas = DoAdvance(dt, suspendParticles, *stateMgr.GetActiveRandom(), advTree);
  if (suspendParticles)
    x120_particleDB.SuspendAllActiveEffects(stateMgr);

  for (size_t i = 0; i < x214_passedParticleCount; ++i) {
    const CParticlePOINode& node = g_ParticlePOINodes[i];
    if (node.GetCharacterIndex() == -1 || node.GetCharacterIndex() == x204_charIdx) {
      x120_particleDB.AddParticleEffect(node.GetString(), node.GetFlags(), node.GetParticleData(), scale, stateMgr, aid,
                                        false, x21c_particleLightIdx);
    }
  }

  return deltas;
}

SAdvancementDeltas CAnimData::AdvanceIgnoreParticles(float dt, CRandom16& random, bool advTree) {
  bool suspendParticles;
  return DoAdvance(dt, suspendParticles, random, advTree);
}

void CAnimData::AdvanceAnim(CCharAnimTime& time, zeus::CVector3f& offset, zeus::CQuaternion& quat) {
  SAdvancementResults results;
  std::optional<std::unique_ptr<IAnimReader>> simplified;

  if (x104_animDir == EAnimDir::Forward) {
    results = x1f8_animRoot->VAdvanceView(time);
    simplified = x1f8_animRoot->Simplified();
  }

  if (simplified)
    x1f8_animRoot = CAnimTreeNode::Cast(std::move(*simplified));

  if ((x220_28_ || x220_27_) && x210_passedIntCount > 0) {
    for (size_t i = 0; i < x210_passedIntCount; ++i) {
      const CInt32POINode& node = g_Int32POINodes[i];
      if (node.GetPoiType() == EPOIType::UserEvent) {
        switch (EUserEventType(node.GetValue())) {
        case EUserEventType::AlignTargetPosStart: {
          x220_26_aligningPos = true;
          break;
        }
        case EUserEventType::AlignTargetPos: {
          x1dc_alignPos = zeus::skZero3f;
          x220_28_ = false;
          x220_26_aligningPos = false;
          break;
        }
        case EUserEventType::AlignTargetRot: {
          x1e8_alignRot = zeus::CQuaternion();
          x220_27_ = false;
          break;
        }
        default:
          break;
        }
      }
    }
  }

  offset += results.x8_deltas.x0_posDelta;
  if (x220_26_aligningPos)
    offset += x1dc_alignPos * time.GetSeconds();

  zeus::CQuaternion rot = results.x8_deltas.xc_rotDelta * x1e8_alignRot;
  quat = quat * rot;
  x1dc_alignPos = rot.transform(x1dc_alignPos);
  time = results.x0_remTime;
}

void CAnimData::SetXRayModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules) {
  xf4_xrayModel = std::make_shared<CSkinnedModel>(model, skinRules, xd8_modelData->GetLayoutInfo(), 0, m_drawInstCount);
}

void CAnimData::SetInfraModel(const TLockedToken<CModel>& model, const TLockedToken<CSkinRules>& skinRules) {
  xf8_infraModel =
      std::make_shared<CSkinnedModel>(model, skinRules, xd8_modelData->GetLayoutInfo(), 0, m_drawInstCount);
}

void CAnimData::PoseSkinnedModel(CSkinnedModel& model, const CPoseAsTransforms& pose, const CModelFlags& drawFlags,
                                 const std::optional<CVertexMorphEffect>& morphEffect, const float* morphMagnitudes) {
  model.Calculate(pose, drawFlags, morphEffect, morphMagnitudes);
}

void CAnimData::AdvanceParticles(const zeus::CTransform& xf, float dt, const zeus::CVector3f& vec,
                                 CStateManager& stateMgr) {
  x120_particleDB.Update(dt, x224_pose, *xcc_layoutData, xf, vec, stateMgr);
}

float CAnimData::GetAverageVelocity(int animIn) const {
  std::shared_ptr<IMetaAnim> anim = x100_animMgr->GetMetaAnimation(xc_charInfo.GetAnimationIndex(animIn));
  std::set<CPrimitive> prims;
  anim->GetUniquePrimitives(prims);

  SObjectTag tag{FOURCC('ANIM'), 0};
  float velAccum = 0.f;
  float durAccum = 0.f;
  for (const CPrimitive& prim : prims) {
    tag.id = prim.GetAnimResId();
    TLockedToken<CAllFormatsAnimSource> animRes = xfc_animCtx->xc_store.GetObj(tag);

    CCharAnimTime dur;
    float avgVel;
    switch (animRes->GetFormat()) {
    case EAnimFormat::Uncompressed:
    default: {
      const CAnimSource& src = animRes->GetAsCAnimSource();
      dur = src.GetDuration();
      avgVel = src.GetAverageVelocity();
      break;
    }
    case EAnimFormat::BitstreamCompressed:
    case EAnimFormat::BitstreamCompressed24: {
      const CFBStreamedCompression& src = animRes->GetAsCFBStreamedCompression();
      dur = src.GetAnimationDuration();
      avgVel = src.GetAverageVelocity();
      break;
    }
    }

    velAccum += dur.GetSeconds() * avgVel;
    durAccum += dur.GetSeconds();
  }

  if (durAccum > 0.f)
    return velAccum / durAccum;
  return 0.f;
}

void CAnimData::ResetPOILists() {
  x20c_passedBoolCount = 0;
  x210_passedIntCount = 0;
  x214_passedParticleCount = 0;
  x218_passedSoundCount = 0;
}

CSegId CAnimData::GetLocatorSegId(std::string_view name) const { return xcc_layoutData->GetSegIdFromString(name); }

zeus::CAABox CAnimData::GetBoundingBox(const zeus::CTransform& xf) const {
  return GetBoundingBox().getTransformedAABox(xf);
}

zeus::CAABox CAnimData::GetBoundingBox() const {
  auto aabbList = xc_charInfo.GetAnimBBoxList();
  if (aabbList.empty())
    return x108_aabb;

  CAnimTreeEffectiveContribution contrib = x1f8_animRoot->GetContributionOfHighestInfluence();
  auto search = rstl::binary_find(
      aabbList.cbegin(), aabbList.cend(), contrib.x4_name,
      [](const std::pair<std::string, zeus::CAABox>& other) -> const std::string& { return other.first; });
  if (search == aabbList.cend())
    return x108_aabb;

  return search->second;
}

void CAnimData::SubstituteModelData(const TCachedToken<CSkinnedModel>& model) {
  xd8_modelData = model;
  x108_aabb = xd8_modelData->GetModel()->GetAABB();
}

void CAnimData::SetParticleCEXTValue(std::string_view name, int idx, float value) {
  auto search = std::find_if(xc_charInfo.x98_effects.begin(), xc_charInfo.x98_effects.end(),
                             [&name](const auto& v) { return v.first == name; });
  if (search != xc_charInfo.x98_effects.end() && search->second.size())
    x120_particleDB.SetCEXTValue(search->second.front().GetComponentName(), idx, value);
}

} // namespace urde
