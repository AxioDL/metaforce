#include "CActor.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "Collision/CMaterialList.hpp"
#include "Audio/CSfxManager.hpp"
#include "TCastTo.hpp"
#include "Character/IAnimReader.hpp"

namespace urde
{
static CMaterialList MakeActorMaterialList(const CMaterialList& materialList, const CActorParameters& params)
{
    CMaterialList ret = materialList;
    if (params.GetVisorParameters().x0_28_b3)
        ret.Add(EMaterialTypes::Mud);
    if (params.GetVisorParameters().x0_29_b4)
        ret.Add(EMaterialTypes::Glass);
    return ret;
}

CActor::CActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info, const zeus::CTransform&,
               CModelData&& mData, const CMaterialList& list, const CActorParameters& params, TUniqueId otherUid)
: CEntity(uid, info, active, name)
, x68_material(MakeActorMaterialList(list, params))
, x70_materialFilter(CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {0ull}))
, xc6_(otherUid)
{
    if (mData.x10_animData || mData.x1c_normalModel)
        x64_modelData = std::make_unique<CModelData>(std::move(mData));
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
    case EScriptObjectMessage::InternalMessage11: // 33
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
    case EScriptObjectMessage::InternalMessage13: // 35
    {
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::DFST)
                continue;

            const CActor* act = TCastToConstPtr<CActor>(mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)));
            if (act && xc6_ == kInvalidUniqueId)
                xc6_ = act->GetUniqueId();
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

zeus::CVector3f CActor::GetScanObjectIndicatorPosition(const CStateManager&) { return {}; }

void CActor::RemoveEmitter()
{
    if (x8c_sfxHandle)
    {
        CSfxManager::RemoveEmitter(*x8c_sfxHandle.get());
        x88_sfxId = -1;
        x8c_sfxHandle.reset();
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

void CActor::DoUserAnimEvent(CStateManager&, CInt32POINode&, EUserEventType) {}

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

const CSfxHandle* CActor::GetSfxHandle() const { return x8c_sfxHandle.get(); }

void CActor::SetSfxPitchBend(s32 val)
{
    xe6_30_enablePitchBend = true;
    xc0_ = val;
    if (x8c_sfxHandle == 0)
        return;

    CSfxManager::PitchBend(*x8c_sfxHandle.get(), val);
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

SAdvancementDeltas CActor::UpdateAnimation(float, CStateManager&, bool)
{
    return {};
}
}
