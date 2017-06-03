#include "CActor.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "Collision/CMaterialList.hpp"
#include "Audio/CSfxManager.hpp"
#include "TCastTo.hpp"
#include "Character/IAnimReader.hpp"
#include "Character/CActorLights.hpp"
#include "Camera/CGameCamera.hpp"

namespace urde
{
static CMaterialList MakeActorMaterialList(const CMaterialList& materialList, const CActorParameters& params)
{
    CMaterialList ret = materialList;
    if (params.GetVisorParameters().x0_28_b3)
        ret.Add(EMaterialTypes::Unknown46);
    if (params.GetVisorParameters().x0_29_b4)
        ret.Add(EMaterialTypes::ScanPassthrough);
    return ret;
}

CActor::CActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform&,
               CModelData&& mData, const CMaterialList& list, const CActorParameters& params, TUniqueId otherUid)
: CEntity(uid, info, active, name)
, x68_material(MakeActorMaterialList(list, params))
, x70_materialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}))
, xc6_nextDrawNode(otherUid)
{
    if (mData.x10_animData || mData.x1c_normalModel)
        x64_modelData = std::make_unique<CModelData>(std::move(mData));
    xd8_nonLoopingSfxHandles.resize(2);
}

void CActor::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    switch (msg)
    {
    case EScriptObjectMessage::Activate:
    {
        if (!x30_24_active)
            xbc_time = CGraphics::GetSecondsMod900();
    }
    break;
    case EScriptObjectMessage::Decrement:
        RemoveEmitter();
        break;
    case EScriptObjectMessage::Registered: // 33
    {
        if (x98_scanObjectInfo)
            AddMaterial(EMaterialTypes::Scannable, mgr);
        else
            RemoveMaterial(EMaterialTypes::Scannable, mgr);

        if (HasModelData() && x64_modelData->AnimationData())
        {
            TAreaId aid = GetAreaId();
            // x64_modelData->AnimationData()->sub_8002AE6C(mgr, aid, x64_modelData->x0_particleScale);
        }
    }
    break;
    case EScriptObjectMessage::Deleted: // 34
    {
        RemoveEmitter();
#if 0
        if (HasModelData() && x64_modelData->AnimationData() && x64_modelData->GetNormalModel())
            x64_modelData->AnimationData()->GetParticleDB().GetActiveParticleLightIds(mgr);
#endif
    }
    break;
    case EScriptObjectMessage::InitializedInArea: // 35
    {
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::DFST)
                continue;

            const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
            if (act && xc6_nextDrawNode == kInvalidUniqueId)
                xc6_nextDrawNode = act->GetUniqueId();
        }
    }
    break;
    case EScriptObjectMessage::InternalMessage15: // 37
        SetInFluid(true, uid);
        break;
    case EScriptObjectMessage::InternalMessage17: // 39
        SetInFluid(false, kInvalidUniqueId);
        break;
    default:
        break;
    }
    CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CActor::CalculateRenderBounds()
{
    if (x64_modelData && (x64_modelData->AnimationData() || x64_modelData->GetNormalModel()))
        x9c_aabox = x64_modelData->GetBounds(x34_transform);
    else
        x9c_aabox = zeus::CAABox(x34_transform.origin, x34_transform.origin);
}

CHealthInfo* CActor::HealthInfo() { return nullptr; }

const CDamageVulnerability* CActor::GetDamageVulnerability() const { return nullptr; }

const CDamageVulnerability* CActor::GetDamageVulnerability(const zeus::CVector3f&, const zeus::CVector3f&,
                                                           const CDamageInfo&) const
{
    return nullptr;
}

rstl::optional_object<zeus::CAABox> CActor::GetTouchBounds() const { return {}; }

void CActor::Touch(CActor&, CStateManager&) {}

zeus::CVector3f CActor::GetOrbitPosition(const CStateManager&) const { return x34_transform.origin; }

zeus::CVector3f CActor::GetAimPosition(const CStateManager&, float) const { return x34_transform.origin; }

zeus::CVector3f CActor::GetHomingPosition(const CStateManager& mgr, float f) const { return GetAimPosition(mgr, f); }

zeus::CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager&) const { return {}; }

void CActor::RemoveEmitter()
{
    if (x8c_loopingSfxHandle)
    {
        CSfxManager::RemoveEmitter(x8c_loopingSfxHandle);
        x88_sfxId = -1;
        x8c_loopingSfxHandle.reset();
    }
}

const zeus::CTransform CActor::GetScaledLocatorTransform(const std::string& segName) const
{
    return x64_modelData->GetScaledLocatorTransform(segName);
}

const zeus::CTransform CActor::GetLocatorTransform(const std::string& segName) const
{
    return x64_modelData->GetLocatorTransform(segName);
}

EWeaponCollisionResponseTypes CActor::GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                               CWeaponMode&, s32)
{
    return EWeaponCollisionResponseTypes::Unknown13;
}

void CActor::FluidFXThink(CActor::EFluidState, CScriptWater&, CStateManager&) {}

void CActor::OnScanStateChanged(EScanState state, CStateManager& mgr)
{
    if (state == EScanState::Start)
        SendScriptMsgs(EScriptObjectState::ScanStart, mgr, EScriptObjectMessage::None);
    else if (state == EScanState::Processing)
        SendScriptMsgs(EScriptObjectState::ScanProcessing, mgr, EScriptObjectMessage::None);
    else if (state == EScanState::Done)
        SendScriptMsgs(EScriptObjectState::ScanDone, mgr, EScriptObjectMessage::None);
}

zeus::CAABox CActor::GetSortingBounds(const CStateManager&) const { return x9c_aabox; }

void CActor::DoUserAnimEvent(CStateManager&, CInt32POINode&, EUserEventType, float dt) {}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4,
                            CStateManager& mgr)
{
    x68_material.Remove(t1);
    RemoveMaterial(t2, t3, t4, mgr);
}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, CStateManager& mgr)
{
    x68_material.Remove(t1);
    RemoveMaterial(t2, t3, mgr);
}

void CActor::RemoveMaterial(EMaterialTypes t1, EMaterialTypes t2, CStateManager& mgr) { x68_material.Remove(t1); }

void CActor::RemoveMaterial(EMaterialTypes t, CStateManager& mgr)
{
    x68_material.Remove(t);
    mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, EMaterialTypes t5,
                         CStateManager& mgr)
{
    x68_material.Add(t1);
    AddMaterial(t2, t3, t4, t5, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, EMaterialTypes t4, CStateManager& mgr)
{
    x68_material.Add(t1);
    AddMaterial(t2, t3, t4, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, EMaterialTypes t3, CStateManager& mgr)
{
    x68_material.Add(t1);
    AddMaterial(t2, t3, mgr);
}

void CActor::AddMaterial(EMaterialTypes t1, EMaterialTypes t2, CStateManager& mgr)
{
    x68_material.Add(t1);
    AddMaterial(t2, mgr);
}

void CActor::AddMaterial(EMaterialTypes type, CStateManager& mgr)
{
    x68_material.Add(type);
    mgr.UpdateObjectInLists(*this);
}

void CActor::AddMaterial(const CMaterialList& l)
{
    x68_material.Add(l);
}

void CActor::CreateShadow(bool b)
{
    if (b)
    {
        _CreateShadow();
        if (!xe5_24_ && x94_simpleShadow)
            xe5_25_ = true;
    }
    xe5_24_ = b;
}

void CActor::_CreateShadow()
{
    if (!x94_simpleShadow && x64_modelData && (x64_modelData->HasAnimData() || x64_modelData->HasNormalModel()))
        x94_simpleShadow.reset(new CSimpleShadow(1.f, 1.f, 20.f, 0.05f));
}

void CActor::SetCallTouch(bool callTouch) { xe5_28_callTouch = callTouch; }

bool CActor::GetCallTouch() const { return xe5_28_callTouch; }

void CActor::SetUseInSortedLists(bool use) { xe5_27_useInSortedLists = use; }

bool CActor::GetUseInSortedLists() const { return xe5_27_useInSortedLists; }

void CActor::SetInFluid(bool in, TUniqueId uid)
{
    if (in)
    {
        xe6_26_inFluid = false;
        xc4_fluidId = uid;
    }
    else
    {
        if (!xe6_26_inFluid)
            return;

        xe6_26_inFluid = true;
        if (xe6_26_inFluid == 0)
            xc4_fluidId = kInvalidUniqueId;
    }
}

bool CActor::HasModelData() const { return bool(x64_modelData); }

void CActor::SetSoundEventPitchBend(s32 val)
{
    xe6_30_enablePitchBend = true;
    xc0_pitchBend = val / 8192.f;
    if (!x8c_loopingSfxHandle)
        return;

    CSfxManager::PitchBend(x8c_loopingSfxHandle, val);
}

void CActor::SetRotation(const zeus::CQuaternion &q)
{
    x34_transform = q.toTransform(x34_transform.origin);
    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;
}

void CActor::SetTranslation(const zeus::CVector3f& tr)
{
    x34_transform.origin = tr;
    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;
}

void CActor::SetTransform(const zeus::CTransform& tr)
{
    x34_transform = tr;
    xe4_27_ = true;
    xe4_28_ = true;
    xe4_29_ = true;
}

void CActor::SetAddedToken(u32 tok) { xcc_addedToken = tok; }

float CActor::GetPitch() const { return zeus::CQuaternion(x34_transform.buildMatrix3f()).pitch(); }

float CActor::GetYaw() const { return zeus::CQuaternion(x34_transform.buildMatrix3f()).yaw(); }

void CActor::EnsureRendered(const CStateManager& stateMgr, const zeus::CVector3f& pos,
                            const zeus::CAABox& aabb) const
{
    if (x64_modelData)
    {
        x64_modelData->RenderUnsortedParts(x64_modelData->GetRenderingModel(stateMgr),
                                           x34_transform, x90_actorLights.get(), xb4_drawFlags);
    }
    stateMgr.AddDrawableActor(*this, pos, aabb);
}

void CActor::UpdateSfxEmitters()
{
    for (CSfxHandle& sfx : xd8_nonLoopingSfxHandles)
        CSfxManager::UpdateEmitter(sfx, x34_transform.origin, zeus::CVector3f::skZero, xd4_maxVol);
}

void CActor::ProcessSoundEvent(u32 sfxId, float weight, u32 flags, float falloff, float maxDist,
                               float minVol, float maxVol, const zeus::CVector3f& toListener,
                               const zeus::CVector3f& position, TAreaId aid, CStateManager& mgr,
                               bool translateId)
{
    if (toListener.magSquared() >= maxDist * maxDist)
        return;
    u16 id = translateId ? CSfxManager::TranslateSFXID(sfxId) : sfxId;

    u32 musyxFlags = 0x1; // Continuous parameter update
    if (flags & 0x8)
        musyxFlags |= 0x8; // Doppler FX

    CAudioSys::C3DEmitterParmData parms;
    parms.x0_pos = position;
    parms.xc_dir = zeus::CVector3f::skZero;
    parms.x18_maxDist = maxDist;
    parms.x1c_distComp = falloff;
    parms.x20_flags = musyxFlags;
    parms.x24_sfxId = id;
    parms.x26_maxVol = maxVol;
    parms.x27_minVol = minVol;
    parms.x28_important = false;
    parms.x29_prio = 0x7f;

    bool useAcoustics = (flags & 0x80) == 0;
    bool looping = (sfxId & 0x80000000) != 0;
    bool nonEmitter = (sfxId & 0x40000000) != 0;
    bool continuousUpdate = (sfxId & 0x20000000) != 0;

    if (mgr.GetActiveRandom()->Float() > weight)
        return;

    if (looping)
    {
        u16 curId = x88_sfxId;
        if (!x8c_loopingSfxHandle)
        {
            CSfxHandle handle;
            if (nonEmitter)
                handle = CSfxManager::SfxStart(id, 1.f, 0.f, true, 0x7f, true, aid);
            else
                handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
            if (handle)
            {
                x88_sfxId = id;
                x8c_loopingSfxHandle = handle;
                if (xe6_30_enablePitchBend)
                    CSfxManager::PitchBend(handle, xc0_pitchBend);
            }
        }
        else if (curId == id)
        {
            CSfxManager::UpdateEmitter(x8c_loopingSfxHandle, position, zeus::CVector3f::skZero, maxVol);
        }
        else if (flags & 0x4)
        {
            CSfxManager::RemoveEmitter(x8c_loopingSfxHandle);
            CSfxHandle handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, true, aid);
            if (handle)
            {
                x88_sfxId = id;
                x8c_loopingSfxHandle = handle;
                if (xe6_30_enablePitchBend)
                    CSfxManager::PitchBend(handle, xc0_pitchBend);
            }
        }
    }
    else
    {
        CSfxHandle handle;
        if (nonEmitter)
            handle = CSfxManager::SfxStart(id, 1.f, 0.f, useAcoustics, 0x7f, false, aid);
        else
            handle = CSfxManager::AddEmitter(parms, useAcoustics, 0x7f, false, aid);

        if (continuousUpdate)
        {
            xd8_nonLoopingSfxHandles[xe4_24_nextNonLoopingSfxHandle] = handle;
            xe4_24_nextNonLoopingSfxHandle = (xe4_24_nextNonLoopingSfxHandle + 1) % xd8_nonLoopingSfxHandles.size();
        }

        if (xe6_30_enablePitchBend)
            CSfxManager::PitchBend(handle, xc0_pitchBend);
    }
}

SAdvancementDeltas CActor::UpdateAnimation(float dt, CStateManager& mgr, bool advTree)
{
    SAdvancementDeltas deltas = x64_modelData->AdvanceAnimation(dt, mgr, GetAreaId(), advTree);
    x64_modelData->AdvanceParticles(x34_transform, dt, mgr);
    UpdateSfxEmitters();
    if (x64_modelData && x64_modelData->HasAnimData())
    {
        zeus::CVector3f toCamera =
            mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetTranslation() - x34_transform.origin;

        for (int i=0 ; i<x64_modelData->GetAnimationData()->GetPassedSoundPOICount() ; ++i)
        {
            CSoundPOINode& poi = CAnimData::g_SoundPOINodes[i];
            if (poi.GetPoiType() != EPOIType::Sound)
                continue;
            if (xe5_26_muted)
                continue;
            if (poi.GetCharacterIndex() != -1 &&
                x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
                continue;
            ProcessSoundEvent(poi.GetSfxId(), poi.GetWeight(), poi.GetFlags(), poi.GetFalloff(),
                              poi.GetMaxDist(), 0.16f, xd4_maxVol, toCamera, x34_transform.origin, x4_areaId,
                              mgr, true);
        }

        for (int i=0 ; i<x64_modelData->GetAnimationData()->GetPassedIntPOICount() ; ++i)
        {
            CInt32POINode& poi = CAnimData::g_Int32POINodes[i];
            if (poi.GetPoiType() == EPOIType::SoundInt32)
            {
                if (xe5_26_muted)
                    continue;
                if (poi.GetCharacterIndex() != -1 &&
                    x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
                    continue;
                ProcessSoundEvent(poi.GetValue(), poi.GetWeight(), poi.GetFlags(), 0.1f,
                                  150.f, 0.16f, xd4_maxVol, toCamera, x34_transform.origin, x4_areaId,
                                  mgr, true);
            }
            else if (poi.GetPoiType() == EPOIType::UserEvent)
            {
                DoUserAnimEvent(mgr, poi, EUserEventType(poi.GetValue()), dt);
            }
        }

        for (int i=0 ; i<x64_modelData->GetAnimationData()->GetPassedParticlePOICount() ; ++i)
        {
            CParticlePOINode& poi = CAnimData::g_ParticlePOINodes[i];
            if (poi.GetCharacterIndex() != -1 &&
                x64_modelData->GetAnimationData()->GetCharacterIndex() != poi.GetCharacterIndex())
                continue;
            x64_modelData->AnimationData()->GetParticleDB().SetParticleEffectState(poi.GetString(), true, mgr);
        }
    }
    return deltas;
}

void CActor::SetActorLights(std::unique_ptr<CActorLights> lights)
{
    x90_actorLights = std::move(lights);
    xe4_31_lightsDirty = true;
}

bool CActor::CanDrawStatic() const
{
    if (!x30_24_active)
        return false;

    if (x64_modelData && x64_modelData->HasNormalModel())
        return xb4_drawFlags.x0_blendMode <= 4;

    return false;
}

const CScannableObjectInfo* CActor::GetScannableObjectInfo() const
{
    if (!x98_scanObjectInfo)
        return nullptr;

    TToken<CScannableObjectInfo>& info = *x98_scanObjectInfo;
    if (!info || !info.IsLoaded())
        return nullptr;

    return info.GetObj();
}
}
