#include "CScriptPlatform.hpp"
#include "Collision/CMaterialList.hpp"
#include "Collision/CCollidableOBBTreeGroup.hpp"
#include "World/CScriptWaypoint.hpp"
#include "World/CScriptColorModulate.hpp"
#include "World/CWorld.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
namespace urde
{

static CMaterialList MakePlatformMaterialList()
{
    CMaterialList ret;
    ret.Add(EMaterialTypes::Solid);
    ret.Add(EMaterialTypes::Immovable);
    ret.Add(EMaterialTypes::Platform);
    ret.Add(EMaterialTypes::Occluder);
    return ret;
}

CScriptPlatform::CScriptPlatform(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                 const zeus::CTransform& xf, CModelData&& mData, const CActorParameters& actParms,
                                 const zeus::CAABox& aabb, float f1, bool b1, float f2, bool active,
                                 const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                                 const std::experimental::optional<TLockedToken<CCollidableOBBTreeGroupContainer>>& dcln,
                                 bool b2, u32 w1, u32 w2)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), MakePlatformMaterialList(), aabb, SMoverData(15000.f),
                actParms, 0.3f, 0.1f)
, x25c_currentMass(f1)
, x28c_initialHealth(hInfo)
, x294_health(hInfo)
, x29c_damageVuln(dVuln)
, x304_treeGroupContainer(dcln)
{
    x348_ = f2;
    x34c_ = w1;
    x350_ = w2;
    x356_24_dead = false;;
    x356_25_ = false;
    x356_26_ = b1;
    x356_27_ = false;
    x356_28_ = b2;
    x356_29_ = false;
    x356_30_ = false;
    x356_31_ = true;
    CActor::SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        CMaterialList(EMaterialTypes::Solid),
        CMaterialList(EMaterialTypes::NoStaticCollision, EMaterialTypes::NoPlatformCollision, EMaterialTypes::Platform)));
    if (x304_treeGroupContainer)
        x314_treeGroup = std::make_unique<CCollidableOBBTreeGroup>(x304_treeGroupContainer->GetObj(), x68_material);

}

void CScriptPlatform::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPlatform::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    switch(msg)
    {
    case EScriptObjectMessage::InitializedInArea:
        BuildSlaveList(mgr);
        break;
    case EScriptObjectMessage::AddPlatformRider:
        AddRider(x318_riders, uid, this, mgr);
        break;
    case EScriptObjectMessage::Stop:
    {
        x25c_currentMass = 0.f;
        Stop();
        break;
    }
    case EScriptObjectMessage::Next:
    {
        x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
        if (x25a_targetWaypoint == kInvalidUniqueId)
            mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Stop);
        else if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x25a_targetWaypoint))
        {
            x25c_currentMass = 0.f;
            Stop();
            x270_ = wp->GetTranslation() - GetTranslation();
            SetTranslation(wp->GetTranslation());

            x258_currentWaypoint = x25a_targetWaypoint;
            x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
            mgr.SendScriptMsg(wp, GetUniqueId(), EScriptObjectMessage::Arrived);
#if 0
            if (!x328_slaves1.empty() || !x338_slaves2.empty())
                DragSlaves(bitVector, x270_);

            /* TODO: Implement bitvector */
#endif
            x270_ = zeus::CVector3f::skZero;
        }
        break;
    }
    case EScriptObjectMessage::Start:
    {
        x25a_targetWaypoint = GetNext(x258_currentWaypoint, mgr);
        if (x25a_targetWaypoint == kInvalidUniqueId)
            mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Stop);
        else
            x25c_currentMass = 0.f;
        break;
    }
    case EScriptObjectMessage::Reset:
    {
        x356_24_dead = false;
        x294_health = x28c_initialHealth;
        break;
    }
    case EScriptObjectMessage::Increment:
    {
        if (GetActive())
            CScriptColorModulate::FadeInHelper(mgr, GetUniqueId(), x268_fadeInTime);
        else
            mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Activate);
        break;
    }
    case EScriptObjectMessage::Decrement:
        CScriptColorModulate::FadeOutHelper(mgr, GetUniqueId(), x26c_fadeOutTime);
        break;
    case EScriptObjectMessage::Deleted:
        DecayRiders(x318_riders, 0.f, mgr);
        break;
    default:
        break;
    }

    CPhysicsActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPlatform::PreThink(float dt, CStateManager& mgr)
{
    DecayRiders(x318_riders, dt, mgr);
    x264_ -= dt;
    x260_ -= dt;
    if (x260_ <= 0.f)
    {
        x270_ = zeus::CVector3f::skZero;
        zeus::CTransform xf = x34_transform;
        CMotionState mState = GetMotionState();
        if (GetActive())
        {
            for (SRiders& rider : x318_riders)
            {
                if (TCastToPtr<CPhysicsActor> act = mgr.ObjectById(rider.x0_uid))
                    rider.x8_transform.origin = x34_transform.transposeRotate(act->GetTranslation() - GetTranslation());
            }
        }
    }

    x27c_ = Move(dt, mgr);
}

void CScriptPlatform::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    if (HasModelData() && GetModelData()->HasAnimData())
    {
        if (!x356_25_)
            UpdateAnimation(dt, mgr, true);
        if (x356_28_ && mgr.GetWorld()->GetNeededEnvFx() == EEnvFxType::Rain)
        {
            if (HasModelData() && (GetModelData()->HasAnimData() || GetModelData()->HasNormalModel())
                    && mgr.GetEnvFxManager()->GetRainMagnitude() != 0.f)
                mgr.GetEnvFxManager()->sub801e4f10(this, 0.f, mgr, x34c_, x350_);
        }
    }

#if 0
    if (!x328_slaves1.empty() || !x338_slaves2.empty())
        DragSlaves(bitVector, x270_);

    /* TODO: Implement bitvector */
#endif

    if (x356_24_dead)
        return;

    if (HealthInfo(mgr)->GetHP() <= 0.f)
    {
        x356_24_dead = true;
        SendScriptMsgs(EScriptObjectState::Dead, mgr, EScriptObjectMessage::None);
    }
}

std::experimental::optional<zeus::CAABox> CScriptPlatform::GetTouchBounds() const
{
    if (x314_treeGroup)
        return {x314_treeGroup->CalculateAABox(GetTransform())};

    return {CPhysicsActor::GetBoundingBox()};
}

bool CScriptPlatform::IsRider(TUniqueId id) const
{
    for (const SRiders& rider : x318_riders)
        if (rider.x0_uid == id)
            return true;
    return false;
}

bool CScriptPlatform::IsSlave(TUniqueId id) const
{
    auto search = std::find_if(x328_slaves1.begin(), x328_slaves1.end(),
                               [id](const SRiders& rider){ return rider.x0_uid == id; });
    if (search != x328_slaves1.end())
        return true;
    search = std::find_if(x338_slaves2.begin(), x338_slaves2.end(),
                          [id](const SRiders& rider){ return rider.x0_uid == id; });
    return search != x338_slaves2.end();
}

void CScriptPlatform::BuildSlaveList(CStateManager& mgr)
{
}

void CScriptPlatform::AddRider(std::vector<SRiders>&, TUniqueId, const CPhysicsActor*, CStateManager& )
{

}

TUniqueId CScriptPlatform::GetNext(TUniqueId uid, CStateManager& mgr)
{
    TCastToConstPtr<CScriptWaypoint> nextWp = mgr.GetObjectById(uid);
    if (!nextWp)
        return GetWaypoint(mgr);

    TUniqueId next = nextWp->NextWaypoint(mgr);
    if (TCastToConstPtr<CScriptWaypoint>(mgr.GetObjectById(next)))
        x25c_currentMass = xe8_mass;

    return next;
}

TUniqueId CScriptPlatform::GetWaypoint(CStateManager& mgr)
{
    for (const SConnection& conn : x20_conns)
    {
        if (conn.x4_msg == EScriptObjectMessage::Follow)
            return mgr.GetIdForScript(conn.x8_objId);
    }

    return kInvalidUniqueId;
}

void CScriptPlatform::SplashThink(const zeus::CAABox &, const CFluidPlane &, float, CStateManager &) const
{

}

zeus::CQuaternion CScriptPlatform::Move(float, CStateManager& mgr)
{
    TUniqueId nextWaypoint = x25a_targetWaypoint;
    if (x25a_targetWaypoint == kInvalidUniqueId)
        nextWaypoint = GetNext(x258_currentWaypoint, mgr);

    TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(nextWaypoint);
    if (x258_currentWaypoint != kInvalidUniqueId)
    {
        if (wp && !wp->GetActive())
            nextWaypoint = GetNext(x258_currentWaypoint, mgr);
        if (nextWaypoint == kInvalidUniqueId)
        {
            if (TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(x258_currentWaypoint))
                if (wp->GetActive())
                    nextWaypoint = wp->GetUniqueId();
        }
    }

    if (nextWaypoint == kInvalidUniqueId)
        return zeus::CQuaternion::skNoRotation;

    /* TODO: Finish */
    return {};
}
}
