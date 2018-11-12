#include "CParasite.hpp"
#include "Character/CModelData.hpp"
#include "World/CActorParameters.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptWaypoint.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Graphics/CSkinnedModel.hpp"
#include "Collision/CGameCollision.hpp"

namespace urde::MP1
{

const float CParasite::flt_805A8FB0 = 2.f * std::sqrt(2.5f / 24.525002f);
const float CParasite::skAttackVelocity = 15.f / 2.f * (std::sqrt(2.5f / 24.525002f));
short CParasite::word_805A8FC0 = 0;
const float CParasite::flt_805A8FB8 = 2.f * std::sqrt(2.5f / 24.525002f);
const float CParasite::skRetreatVelocity = 3.f / 2.f * std::sqrt(2.5f / 24.525002f);

CParasite::CParasite(TUniqueId uid, std::string_view name, EFlavorType flavor, const CEntityInfo &info,
                     const zeus::CTransform &xf, CModelData &&mData, const CPatternedInfo &pInfo, EBodyType bodyType,
                     float f1, float f2, float f3, float f4, float f5, float f6, float f7, float f8, float f9,
                     float f10, float f11, float f12, float f13, float f14, float f15, float f16, float f17, float f18,
                     bool b1, EWalkerType wType, const CDamageVulnerability& dVuln,const CDamageInfo& parInfo, u16 sfxId1,
                     u16 sfxId2, u16 sfxId3, CAssetId modelRes, CAssetId skinRes, float f19, const CActorParameters &aParams)
: CWallWalker(ECharacter::Parasite, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
              EColliderType::Zero, bodyType, aParams, f7, f4, EKnockBackVariant::Small, f2, wType, f17, b1)
, x64c_(dVuln)
, x6b4_(parInfo)
, x6d0_(f1)
, x6d4_(f3)
, x6dc_(f5)
, x6e0_(f6)
, x6e4_(f8)
, x6e8_(f9)
, x6ec_(f10)
, x6f0_(f11)
, x6f4_(f12)
, x6f8_(f13)
, x6fc_(f14)
, x700_(f15)
, x704_(f16)
, x708_(pInfo.GetHeight() * 0.5f)
, x710_(f18)
, x714_(f19)
, x73c_haltSfx(CSfxManager::TranslateSFXID(sfxId1))
, x73e_getUpSfx(CSfxManager::TranslateSFXID(sfxId2))
, x740_crouchSfx(CSfxManager::TranslateSFXID(sfxId3))
{
    x742_28_onGround = true;
    x742_30_attackOver = true;

    switch (x5d0_walkerType)
    {
    case EWalkerType::Geemer:
        x460_knockBackController.SetEnableFreeze(false);
    case EWalkerType::Oculus:
        x460_knockBackController.SetAutoResetImpulse(false);
        break;
    case EWalkerType::IceZoomer:
    {
        TLockedToken<CModel> model = g_SimplePool->GetObj({FOURCC('CMDL'), modelRes});
        TLockedToken<CModel> skin = g_SimplePool->GetObj({FOURCC('CSKR'), skinRes});
        x624_extraModel = CToken(
        TObjOwnerDerivedFromIObj<CSkinnedModel>::GetNewDerivedObject(std::make_unique<CSkinnedModel>(
            model, skin, x64_modelData->AnimationData()->GetModelData()->GetLayoutInfo(), 1, 1)));
    }
    default:
        break;
    }
    if (x5d0_walkerType == EWalkerType::Oculus)
    {
        x460_knockBackController.SetEnableShock(false);
        x460_knockBackController.SetEnableBurn(false);
        x460_knockBackController.SetEnableBurnDeath(false);
        x460_knockBackController.SetEnableExplodeDeath(false);
        x460_knockBackController.SetX82_24(false);
    }
}

void CParasite::Accept(IVisitor &visitor) { visitor.Visit(this); }

void CParasite::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CPatterned::AcceptScriptMsg(msg, uid, mgr);
    if (msg == EScriptObjectMessage::Registered)
    {
        x450_bodyController->Activate(mgr);
        /* TODO: Finish 8015A0E8*/
    }
    else if (msg == EScriptObjectMessage::Deleted)
    {
        //mgr.xf54_.sub80125d88(GetUniqueId());
        if (x5d0_walkerType != EWalkerType::IceZoomer)
            DestroyActorManager(mgr);
    }
    else if (msg == EScriptObjectMessage::Jumped && x742_25_jumpVelDirty)
    {
        UpdateJumpVelocity();
        x742_25_jumpVelDirty = false;
    }
    else if (msg == EScriptObjectMessage::Activate)
    {
        x5d6_27_ = false;
        if (x5d0_walkerType != EWalkerType::Parasite)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    }
    else if (msg == EScriptObjectMessage::SuspendedMove)
    {
    }
}

void CParasite::PreThink(float dt, CStateManager& mgr)
{
    CWallWalker::PreThink(dt, mgr);
    x743_26_oculusShotAt = false;
}

void CParasite::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    ++x5d4_;
    if (x5d0_walkerType == EWalkerType::IceZoomer)
        UpdateCollisionActors(mgr);

    x5d6_26_ = false;
    CGameArea* area = mgr.WorldNC()->GetArea(GetAreaIdAlways());

    CGameArea::EOcclusionState r6 = CGameArea::EOcclusionState::Occluded;
    if (area->IsPostConstructed())
        r6 = area->GetPostConstructed()->x10dc_occlusionState;
    if (r6 != CGameArea::EOcclusionState::Visible)
        x5d6_26_ = true;

    if (!x5d6_26_)
    {
        zeus::CVector3f plVec = mgr.GetPlayer().GetTranslation();
        float distance = (GetTranslation() - plVec).magnitude() ;

        if (distance > x5c4_)
        {
            CRayCastResult res = mgr.RayStaticIntersection(plVec, (GetTranslation() - plVec).normalized(), distance,
                                      CMaterialFilter::skPassEverything);
            if (res.IsValid())
                x5d6_26_ = true;
        }
    }

    if (x5d6_26_)
    {
        xf8_24_movable = x5d6_26_;
        return;
    }

    xf8_24_movable = !xf8_24_movable;

    if (!x5d6_27_)
    {
        if (x450_bodyController->IsFrozen())
        {
            if ((GetTranslation() - x614_).magSquared() < 0.3f /* <- Used to be a static variable */ * dt)
                x60c_ += dt;
            else
                x60c_ = 0.f;

            x614_ = GetTranslation();
            if (x608_ > 0.f)
                x608_ -= dt;
            else
                x608_ = 0.f;
        }
    }

    if (x400_25_alive)
    {
        CPlayer* pl = mgr.Player();
        float radius;
        if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed && !x742_30_attackOver)
            radius = x590_colSphere.GetSphere().radius;
        else
            radius = x708_;

        zeus::CAABox aabox{GetTranslation() - radius, GetTranslation() + radius};
        auto plBox = pl->GetTouchBounds();

        if (plBox && plBox->intersects(aabox))
        {
            if (!x742_30_attackOver)
            {
                x742_30_attackOver = true;
                x742_27_landed = false;
            }

            if (x420_curDamageRemTime <= 0.f)
            {
                mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                                CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
                x420_curDamageRemTime = x424_damageWaitTime;
            }
        }
    }

    CWallWalker::Think(dt, mgr);

    if (x5d6_27_)
        return;

    if (x450_bodyController->IsFrozen())
        return;

    x3b4_speed = x604_;
    if (x5d6_24_)
        AlignToFloor(mgr, x590_colSphere.GetSphere().radius, GetTranslation() + 2.f * dt * x138_velocity, dt);

    x742_27_landed = false;
}

void CParasite::Render(const CStateManager& mgr) const
{
    CWallWalker::Render(mgr);
}

const CDamageVulnerability* CParasite::GetDamageVulnerability() const
{
    switch (x5d0_walkerType)
    {
    case EWalkerType::Oculus:
        if (x743_24_)
            return &x64c_;
        break;
    case EWalkerType::IceZoomer:
        if (!x743_25_)
            return &CDamageVulnerability::ImmuneVulnerabilty();
        break;
    default:
        break;
    }
    return CAi::GetDamageVulnerability();
}

void CParasite::Touch(CActor& actor, CStateManager& mgr)
{
    CPatterned::Touch(actor, mgr);
}

zeus::CVector3f CParasite::GetAimPosition(const CStateManager&, float) const
{
    return GetTranslation();
}

void CParasite::CollidedWith(TUniqueId uid, const CCollisionInfoList& list, CStateManager&)
{
    static CMaterialList testList(EMaterialTypes::Character, EMaterialTypes::Player);
    if (x743_27_inJump)
    {
        for (const auto& info : list)
        {
            if (!x5d6_24_ && info.GetMaterialLeft().Intersection(testList) == 0)
            {
                OrientToSurfaceNormal(info.GetNormalLeft(), 360.f);
                CPhysicsActor::Stop();
                SetVelocityWR(zeus::CVector3f::skZero);
                x742_27_landed = true;
                x742_28_onGround = true;
            }
        }
    }
}

void CParasite::Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state)
{
    CPhysicsActor::Stop();
    TelegraphAttack(mgr, EStateMsg::Activate, 0.f);
    SetMomentumWR({0.f, 0.f, -GetWeight()});
    CPatterned::Death(mgr, direction, state);
}

void CParasite::Patrol(CStateManager& mgr, EStateMsg msg, float dt)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x742_26_ = true;
        x5d6_24_ = true;
        if (!x5d6_27_ && x5d0_walkerType == EWalkerType::Parasite)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
        SetMomentumWR(zeus::CVector3f::skZero);
        x5d6_25_ = false;
        xf8_24_movable = false;
        break;
    case EStateMsg::Update:
        if (x5bc_ > 0.f)
        {
            x5bc_ -= dt;
            if (x5bc_ <= 0.f)
            {
                if (x5d0_walkerType == EWalkerType::Parasite)
                    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
                x5bc_ = 0.f;
            }
        }
        GotoNextWaypoint(mgr);
        if (x5bc_ <= 0.f && !x5d6_27_)
            DoFlockingBehavior(mgr);
        break;
    case EStateMsg::Deactivate:
        x5d6_24_ = false;
        xf8_24_movable = true;
        break;
    default:
        break;
    }
}

void CParasite::UpdatePFDestination(CStateManager& mgr)
{
    // Empty
}

void CParasite::DoFlockingBehavior(CStateManager& mgr)
{
    zeus::CVector3f upVec = x34_transform.basis[2];
    rstl::reserved_vector<TUniqueId, 1024> _834;
    zeus::CAABox aabb(GetTranslation() - x6e4_, GetTranslation() + x6e4_);
    if (x5d4_ % 6 == 0)
    {
        rstl::reserved_vector<TUniqueId, 1024> nearList;
        static CMaterialFilter filter = CMaterialFilter::MakeInclude(EMaterialTypes::Character);
        CParasite* closestParasite = nullptr;
        float minDistSq = 2.f + x6e8_ * x6e8_;
        mgr.BuildNearList(nearList, aabb, filter, nullptr);
        for (TUniqueId id : nearList)
        {
            if (CParasite* parasite = CPatterned::CastTo<CParasite>(mgr.ObjectById(id)))
            {
                if (parasite != this && parasite->IsAlive())
                {
                    _834.push_back(parasite->GetUniqueId());
                    float distSq = (parasite->GetTranslation() - GetTranslation()).magSquared();
                    if (distSq < minDistSq)
                    {
                        minDistSq = distSq;
                        closestParasite = parasite;
                    }
                }
            }
        }
        if (closestParasite && x6ec_ > 0.f && x6e8_ > 0.f)
            x628_ = x45c_steeringBehaviors.Separation(*this, closestParasite->GetTranslation(), x6e8_) * x604_;
        else
            x628_ = zeus::CVector3f::skZero;
        x634_ = x45c_steeringBehaviors.Cohesion(*this, _834, 0.6f, mgr) * x604_;
        x640_ = x45c_steeringBehaviors.Alignment(*this, _834, mgr) * x604_;
    }

    if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() < x700_ * x700_)
    {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(ProjectVectorToPlane(
            x45c_steeringBehaviors.Separation(*this, mgr.GetPlayer().GetTranslation(), x700_), upVec) * x604_,
                zeus::CVector3f::skZero, x704_));
    }

    if (x628_ != zeus::CVector3f::skZero)
    {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(ProjectVectorToPlane(
            x628_, upVec), zeus::CVector3f::skZero, x6ec_));
    }

    for (const auto& r : x5d8_doorRepulsors)
    {
        if ((r.GetVector() - GetTranslation()).magSquared() < r.GetFloat() * r.GetFloat())
        {
            x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(ProjectVectorToPlane(
                x45c_steeringBehaviors.Separation(*this, r.GetVector(), r.GetFloat()) * x604_, upVec),
                    zeus::CVector3f::skZero, 1.f));
        }
    }

    if (x608_ <= 0.f)
    {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(ProjectVectorToPlane(
            x634_, upVec), zeus::CVector3f::skZero, x6f4_));
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(ProjectVectorToPlane(
            x640_, upVec), zeus::CVector3f::skZero, x6f0_));
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
            ProjectVectorToPlane(ProjectVectorToPlane(
            x45c_steeringBehaviors.Seek(*this, x2e0_destPos), upVec) * x604_, upVec),
                zeus::CVector3f::skZero, x6f8_));
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(
            x34_transform.basis[1] * x604_, zeus::CVector3f::skZero, x6fc_));
    }
}

void CParasite::PathFind(CStateManager& mgr, EStateMsg msg, float dt)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x742_26_ = true;
        x5d6_24_ = true;
        if (x5d0_walkerType == EWalkerType::Parasite)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
        SetMomentumWR(zeus::CVector3f::skZero);
        xf8_24_movable = false;
        break;
    case EStateMsg::Update:
        UpdatePFDestination(mgr);
        DoFlockingBehavior(mgr);
        break;
    case EStateMsg::Deactivate:
        xf8_24_movable = true;
        x5d6_24_ = false;
        x742_26_ = false;
        break;
    default:
        break;
    }
}

void CParasite::TargetPlayer(CStateManager& mgr, EStateMsg msg, float dt)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x5f8_ = mgr.GetPlayer().GetTranslation() + zeus::CVector3f(0.f, 0.f, 1.5f);
        break;
    case EStateMsg::Update:
        x450_bodyController->FaceDirection3D(
            ProjectVectorToPlane(x5f8_ - GetTranslation(), x34_transform.basis[2]),
            x34_transform.basis[1], 2.f);
        break;
    default:
        break;
    }
}

TUniqueId CParasite::RecursiveFindClosestWayPoint(CStateManager& mgr, TUniqueId id, float& dist)
{
    TUniqueId ret = id;
    TCastToPtr<CScriptWaypoint> wp = mgr.ObjectById(id);
    if (!wp)
        return ret;
    wp->SetActive(false);
    dist = (wp->GetTranslation() - GetTranslation()).magSquared();
    for (const auto& conn : wp->GetConnectionList())
    {
        if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
        {
            TUniqueId nextId = mgr.GetIdForScript(conn.x8_objId);
            if (nextId != kInvalidUniqueId)
            {
                if (TCastToConstPtr<CScriptWaypoint> wp2 = mgr.GetObjectById(nextId))
                {
                    if (wp2->GetActive())
                    {
                        float nextDist;
                        TUniqueId closestWp = RecursiveFindClosestWayPoint(mgr, nextId, nextDist);
                        if (nextDist < dist)
                        {
                            dist = nextDist;
                            ret = closestWp;
                        }
                    }
                }
            }
        }
    }
    wp->SetActive(true);
    return ret;
}

TUniqueId CParasite::GetClosestWaypointForState(EScriptObjectState state, CStateManager& mgr)
{
    float minDist = FLT_MAX;
    TUniqueId ret = kInvalidUniqueId;
    for (const auto& conn : GetConnectionList())
    {
        if (conn.x0_state == state && conn.x4_msg == EScriptObjectMessage::Follow)
        {
            TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
            float dist;
            TUniqueId closestWp = RecursiveFindClosestWayPoint(mgr, id, dist);
            if (dist < minDist)
            {
                minDist = dist;
                ret = closestWp;
            }
        }
    }
    return ret;
}

void CParasite::TargetPatrol(CStateManager& mgr, EStateMsg msg, float dt)
{
    if (msg == EStateMsg::Activate)
    {
        SetMomentumWR(zeus::CVector3f::skZero);
        TUniqueId wpId = GetClosestWaypointForState(EScriptObjectState::Patrol, mgr);
        if (wpId != kInvalidUniqueId)
            x2dc_destObj = wpId;
    }
}

void CParasite::Halt(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x330_stateMachineState.SetDelay(x710_);
        x32c_animState = EAnimState::One;
        x743_24_ = true;
        x5d6_24_ = true;
        if (x5d0_walkerType == EWalkerType::Geemer)
            CSfxManager::AddEmitter(x73c_haltSfx, GetTranslation(), zeus::CVector3f::skZero,
                                    true, false, 0x7f, kInvalidAreaId);
        break;
    case EStateMsg::Update:
        TryCommand(mgr, pas::EAnimationState::LoopReaction, &CPatterned::TryLoopReaction, 1);
        x400_24_hitByPlayerProjectile = false;
        break;
    case EStateMsg::Deactivate:
        x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::ExitState));
        x32c_animState = EAnimState::Zero;
        x743_24_ = false;
        x5d6_24_ = false;
        break;
    default:
        break;
    }
}

void CParasite::Run(CStateManager&, EStateMsg, float)
{
    // Empty
}

void CParasite::Generate(CStateManager&, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x5e8_ = 0;
        break;
    case EStateMsg::Update:
        switch (x5e8_)
        {
        case 0:
            if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate)
                x5e8_ = 1;
            else
                x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::Zero));
            break;
        case 1:
            if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate)
                x5e8_ = 2;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

void CParasite::Deactivate(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x5e8_ = 0;
        SendScriptMsgs(EScriptObjectState::UNKS2, mgr, EScriptObjectMessage::None);
        mgr.FreeScriptObject(GetUniqueId());
        break;
    case EStateMsg::Update:
        if (x5e8_ == 0)
        {
            if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Generate)
                x5e8_ = 1;
            else
                x450_bodyController->GetCommandMgr().DeliverCmd(CBCGenerateCmd(pas::EGenerateType::One));
        }
        break;
    default:
        break;
    }
}

void CParasite::Attack(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        x608_ = 0.f;
        if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
        {
            float rz = mgr.GetActiveRandom()->Float();
            float ry = mgr.GetActiveRandom()->Float();
            float rx = mgr.GetActiveRandom()->Float();
            x5f8_ = (zeus::CVector3f(rx, ry, rz) - 0.5f) * 0.5f + mgr.GetPlayer().GetTranslation();
        }
        else
        {
            float rz = mgr.GetActiveRandom()->Float();
            float ry = mgr.GetActiveRandom()->Float();
            float rx = mgr.GetActiveRandom()->Float();
            x5f8_ = (zeus::CVector3f(rx, ry, rz) + mgr.GetPlayer().GetTranslation() - GetTranslation())
            .normalized() * 15.f + GetTranslation();
        }
        FaceTarget(x5f8_);
        x5e8_ = 0;
        x742_30_attackOver = false;
        x742_24_ = false;
        x742_28_onGround = false;
        break;
    case EStateMsg::Update:
        switch (x5e8_)
        {
        case 0:
            if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Jump)
            {
                x5e8_ = 1;
            }
            else
            {
                x742_25_jumpVelDirty = true;
                FaceTarget(x5f8_);
                x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x5f8_, pas::EJumpType::Normal));
            }
            break;
        default:
            break;
        }
        break;
    case EStateMsg::Deactivate:
        x742_28_onGround = true;
        x742_30_attackOver = true;
        break;
    default:
        break;
    }
}

void CParasite::Crouch(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Crouch);
        if (x5d0_walkerType == EWalkerType::Geemer)
            CSfxManager::AddEmitter(x740_crouchSfx, GetTranslation(), zeus::CVector3f::skZero,
                                    true, false, 0x7f, kInvalidAreaId);
    }
}

void CParasite::GetUp(CStateManager&, EStateMsg msg, float)
{
    if (msg == EStateMsg::Activate)
    {
        x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
        if (x5d0_walkerType == EWalkerType::Geemer)
            CSfxManager::AddEmitter(x73e_getUpSfx, GetTranslation(), zeus::CVector3f::skZero,
                                    true, false, 0x7f, kInvalidAreaId);
    }
}

void CParasite::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        for (auto it = mgr.GetParasiteTelegraphs().begin(); it != mgr.GetParasiteTelegraphs().end();)
        {
            CParasite* other = CPatterned::CastTo<CParasite>(mgr.ObjectById(*it));
            if (!other)
            {
                it = mgr.GetParasiteTelegraphs().erase(it);
                continue;
            }
            if (other != this && other->IsAlive() &&
                (other->GetTranslation() - GetTranslation()).magSquared() < x6d0_ * x6d0_)
            {
                other->x742_24_ = true;
                other->x608_ = mgr.GetActiveRandom()->Float() * 0.5f + 0.5f;
                other->x5f8_ = GetTranslation();
            }
            ++it;
        }
        x400_24_hitByPlayerProjectile = false;
        break;
    default:
        break;
    }
}

void CParasite::Jump(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
        AddMaterial(EMaterialTypes::GroundCollider, mgr);
        SetMomentumWR({0.f, 0.f, -GetWeight()});
        x742_28_onGround = false;
        x5d6_24_ = false;
        x742_27_landed = false;
        x743_27_inJump = true;
        break;
    case EStateMsg::Update:
        SetMomentumWR({0.f, 0.f, -GetWeight()});
        break;
    case EStateMsg::Deactivate:
        RemoveMaterial(EMaterialTypes::GroundCollider, mgr);
        SetMomentumWR(zeus::CVector3f::skZero);
        x742_28_onGround = true;
        x742_27_landed = false;
        x743_27_inJump = false;
        break;
    }
}

void CParasite::FaceTarget(const zeus::CVector3f& target)
{
    zeus::CQuaternion q =
    zeus::CQuaternion::lookAt(zeus::CTransform::Identity().basis[1],
                              target - GetTranslation(), zeus::degToRad(360.f));
    SetTransform(q.toTransform(GetTranslation()));
}

void CParasite::Retreat(CStateManager& mgr, EStateMsg msg, float)
{
    switch (msg)
    {
    case EStateMsg::Activate:
    {
        zeus::CVector3f dir = mgr.GetPlayer().GetTranslation() - GetTranslation();
        dir.z = 0.f;
        if (dir.canBeNormalized())
            dir.normalize();
        else
            dir = mgr.GetPlayer().GetTransform().basis[1];
        x5f8_ = GetTranslation() - dir * 3.f;
        FaceTarget(x5f8_);
        x5e8_ = 0;
        x742_27_landed = false;
        x742_28_onGround = false;
        x742_25_jumpVelDirty = true;
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCJumpCmd(x5f8_, pas::EJumpType::One));
        break;
    }
    case EStateMsg::Update:
        x3b4_speed = 1.f;
        break;
    case EStateMsg::Deactivate:
        x742_28_onGround = true;
        break;
    }
}

bool CParasite::AnimOver(CStateManager&, float)
{
    return x5e8_ == 2;
}

bool CParasite::ShouldAttack(CStateManager& mgr, float arg)
{
    bool shouldAttack = false;
    if (x742_24_ && x608_ > 0.1f)
        shouldAttack = true;
    if (!TooClose(mgr, arg) && InMaxRange(mgr, arg))
        return shouldAttack || InDetectionRange(mgr, 0.f);
    return false;
}

bool CParasite::CloseToWall(CStateManager& mgr)
{
    static CMaterialFilter filter = CMaterialFilter::MakeInclude(EMaterialTypes::Solid);
    zeus::CAABox aabb = CPhysicsActor::GetBoundingBox();
    float margin = x590_colSphere.GetSphere().radius + x5b0_collisionCloseMargin;
    aabb.min -= zeus::CVector3f(margin);
    aabb.max += zeus::CVector3f(margin);
    CCollidableAABox cAABB(aabb, x68_material);
    return CGameCollision::DetectStaticCollisionBoolean(mgr, cAABB, {}, filter);
}

bool CParasite::HitSomething(CStateManager& mgr, float)
{
    if (x5d4_ & 0x1)
        return true;
    return x5b8_ < 270.f && CloseToWall(mgr);
}

bool CParasite::Stuck(CStateManager&, float)
{
    return x60c_ > x6e0_;
}

bool CParasite::Landed(CStateManager&, float)
{
    return x742_27_landed;
}

bool CParasite::AttackOver(CStateManager&, float)
{
    return x742_30_attackOver;
}

bool CParasite::ShotAt(CStateManager&, float)
{
    if (x5d0_walkerType != EWalkerType::Oculus)
        return x400_24_hitByPlayerProjectile;
    return x743_26_oculusShotAt;
}

void CParasite::MassiveDeath(CStateManager& mgr)
{
    CPatterned::MassiveDeath(mgr);
}

void CParasite::MassiveFrozenDeath(CStateManager& mgr)
{
    CPatterned::MassiveFrozenDeath(mgr);
}

void CParasite::ThinkAboutMove(float dt)
{
    if (!x68_material.HasMaterial(EMaterialTypes::GroundCollider))
        CPatterned::ThinkAboutMove(dt);
}

bool CParasite::IsOnGround() const
{
    return x742_28_onGround;
}

void CParasite::UpdateWalkerAnimation(CStateManager& mgr, float dt)
{
    CActor::UpdateAnimation(dt, mgr, true);
}

void CParasite::DestroyActorManager(CStateManager& mgr)
{
    //x620_->Destroy(mgr);
}

void CParasite::UpdateJumpVelocity()
{
    SetMomentumWR({0.f, 0.f, -GetWeight()});
    zeus::CVector3f vec;

    if (!x742_30_attackOver)
    {
        vec = skAttackVelocity * GetTransform().frontVector();
        vec.z = 0.5f * skRetreatVelocity;
    }
    else
    {
        vec = skRetreatVelocity * GetTransform().frontVector();
        vec.z = 0.5f * skAttackVelocity;
    }

    float f30 = x150_momentum.z / xe8_mass;
    float f31 = x5f8_.z - GetTranslation().z;
    zeus::CVector3f vec2 = x5f8_ - GetTranslation();
    vec2.z = 0.f;
    float f29 = vec2.magnitude();

    if (f29 > FLT_EPSILON)
    {
        vec2 *= zeus::CVector3f{1.f / f29};
        float f28 = vec2.dot(vec);
        if (f28 > FLT_EPSILON)
        {
            float f27 = 0.f;
            bool isNeg = f31 < 0.f;
            float out1, out2;
            if (CSteeringBehaviors::SolveQuadratic(f30, vec.z, -f31, vec2.y, out1, out2))
                f27 = isNeg ? out1 : out2;

            if (!isNeg)
                f27 = f27 * f29 / f28;

            if (f27 < 10.f)
            {
                vec = f29 / f27 * vec2;
                vec.z = (0.5f * f30 * f27 + f31 / f27);
            }
        }
    }
    SetVelocityWR(vec);
}

void CParasite::AlignToFloor(CStateManager&, float, const zeus::CVector3f&, float)
{
}

} // namespace urde::MP1
