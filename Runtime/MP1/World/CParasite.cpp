#include "CParasite.hpp"
#include "Character/CModelData.hpp"
#include "World/CActorParameters.hpp"
#include "World/CPatternedInfo.hpp"
#include "World/CWorld.hpp"
#include "World/CGameArea.hpp"
#include "World/CPlayer.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

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
                     bool b1, u32 w1, const CDamageVulnerability& dVuln,const CDamageInfo& parInfo, u16 sfxId1,
                     u16 sfxId2, u16 sfxId3, u32 w2, u32 w3, float f19, const CActorParameters &aParams)
: CWallWalker(ECharacter::Parasite, uid, name, flavor, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
              EColliderType::Zero, bodyType, aParams, f7, f4, EKnockBackVariant::Zero, f2, w1, f17, b1)
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
, x73c_(CSfxManager::TranslateSFXID(sfxId1))
, x73e_(CSfxManager::TranslateSFXID(sfxId2))
, x740_(CSfxManager::TranslateSFXID(sfxId3))
{
    x742_28_ = true;
    x742_30_ = true;

    switch(x5d0_)
    {
    case 2:
        x460_knockBackController.SetX81_25(false);
    case 1:
        x460_knockBackController.SetAutoResetImpulse(false);
        break;
    case 3:

    default:
        break;
    }
    if (x5d0_ == 1)
    {

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
        if (x5d0_ != 3)
            DestroyActorManager(mgr);
    }
    else if (msg == EScriptObjectMessage::Jumped && x742_25_)
    {
        UpdateJumpVelocity();
        x742_25_ = false;
    }
    else if (msg == EScriptObjectMessage::Activate)
    {
        x5d6_27_ = false;
        if (x5d0_ != 0)
            x450_bodyController->SetLocomotionType(pas::ELocomotionType::Lurk);
    }
    else if (msg == EScriptObjectMessage::SuspendedMove)
    {
    }
}

void CParasite::PreThink(float dt, CStateManager& mgr)
{
    CWallWalker::PreThink(dt, mgr);
    x743_26_ = false;
}

void CParasite::Think(float dt, CStateManager& mgr)
{
    if (!GetActive())
        return;

    ++x5d4_;
    if (x5d0_ == 3)
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
        if (pl->GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed && !x742_30_)
            radius = x590_colSphere.GetSphere().radius;
        else
            radius = x708_;

        zeus::CAABox aabox{GetTranslation() - radius, GetTranslation() + radius};
        auto plBox = pl->GetTouchBounds();

        if (plBox && plBox->intersects(aabox))
        {
            if (!x742_30_)
            {
                x742_30_ = true;
                x742_27_ = false;
            }

            if (x420_curDamageTime <= 0.f)
            {
                mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                                CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
                x420_curDamageTime = x424_damageWaitTime;
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

    x742_27_ = false;
}

void CParasite::DestroyActorManager(CStateManager& mgr)
{
    //x620_->Destroy(mgr);
}

void CParasite::UpdateJumpVelocity()
{
    x150_momentum.x = 0;
    x150_momentum.y = 0;
    x150_momentum.z = -GetWeight();
    zeus::CVector3f vec;

    if (!x742_30_)
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
    float f31 = x600_ - GetTranslation().z;
    zeus::CVector3f vec2;
    vec2.x = x5f8_ - GetTranslation().x;
    vec2.y = x5fc_ * GetTranslation().y;
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
