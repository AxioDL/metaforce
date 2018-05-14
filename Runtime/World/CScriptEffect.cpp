#include "CScriptEffect.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"
#include "GameGlobalObjects.hpp"
#include "CSimplePool.hpp"
#include "Particle/CElementGen.hpp"
#include "Particle/CParticleElectric.hpp"
#include "CStateManager.hpp"
#include "World/CWorld.hpp"
#include "World/CGameLight.hpp"
#include "Camera/CGameCamera.hpp"
#include "CPlayerState.hpp"
#include "CScriptTrigger.hpp"
#include "TCastTo.hpp"

namespace urde
{

u32 CScriptEffect::g_NumParticlesUpdating = 0;
u32 CScriptEffect::g_NumParticlesRendered = 0;

CScriptEffect::CScriptEffect(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                             const zeus::CTransform& xf, const zeus::CVector3f& scale,
                             CAssetId partId, CAssetId elscId, bool hotInThermal, bool b2, bool b3, bool active,
                             bool b4, float f1, float f2, float f3, float f4, bool b5, float f5, float f6, float f7,
                             bool b6, bool b7, bool b8, const CLightParameters& lParms, bool b9)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None().HotInThermal(hotInThermal), kInvalidUniqueId)
, x10c_partId(partId)
, x110_24_(active)
, x110_25_(b2)
, x110_26_(b3)
, x110_27_(b4)
, x110_28_(b6)
, x110_29_(b7)
, x110_30_(b8)
, x110_31_(b8 && b7 && b6)
, x111_24_(b5)
, x111_25_(b9)
, x111_26_canRender(false)
, x114_(f1)
, x118_(f1 * f1)
, x11c_(f2)
, x120_(f5)
, x124_(f6)
, x128_(f7)
, x12c_(f3)
, x130_(f3)
, x134_(f4)
, x138_actorLights(lParms.MakeActorLights())
{
    if (partId.IsValid())
    {
        xf8_particleSystemToken = g_SimplePool->GetObj({FOURCC('PART'), partId});
        x104_particleSystem.reset(new CElementGen(xf8_particleSystemToken, CElementGen::EModelOrientationType::Normal, CElementGen::EOptionalSystemFlags::One));
        zeus::CTransform newXf = xf;
        newXf.origin = zeus::CVector3f::skZero;
        x104_particleSystem->SetOrientation(newXf);
        x104_particleSystem->SetTranslation(xf.origin);
        x104_particleSystem->SetLocalScale(scale);
        x104_particleSystem->SetParticleEmission(active);
        x104_particleSystem->SetModulationColor(lParms.GetNoLightsAmbient());
        x104_particleSystem->SetModelsUseLights(x138_actorLights != nullptr);
    }


    if (elscId.IsValid())
    {
        xe8_electricToken = g_SimplePool->GetObj({FOURCC('ELSC'), elscId});
        xf4_electric.reset(new CParticleElectric(xe8_electricToken));
        zeus::CTransform newXf = xf;
        newXf.origin = zeus::CVector3f::skZero;
        xf4_electric->SetOrientation(newXf);
        xf4_electric->SetTranslation(xf.origin);
        xf4_electric->SetLocalScale(scale);
        xf4_electric->SetParticleEmission(active);
    }
    xe7_29_actorActive = true;
}

void CScriptEffect::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptEffect::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    bool active = GetActive();
    if (msg == EScriptObjectMessage::Activate)
    {
        if (x110_26_)
        {
            if (x104_particleSystem)
            {
                zeus::CVector3f scale = x104_particleSystem->GetGlobalScale();
                zeus::CColor color = x104_particleSystem->GetModulationColor();
                x104_particleSystem.reset(new CElementGen(xf8_particleSystemToken));
                zeus::CTransform newXf = GetTransform();
                newXf.origin = zeus::CVector3f::skZero;
                x104_particleSystem->SetOrientation(newXf);
                x104_particleSystem->SetTranslation(GetTranslation());
                x104_particleSystem->SetLocalScale(scale);
                x104_particleSystem->SetParticleEmission(active);
                x104_particleSystem->SetModulationColor(color);
                x104_particleSystem->SetModelsUseLights(x138_actorLights != nullptr);
            }

            if (xf4_electric)
            {
                zeus::CVector3f scale = xf4_electric->GetGlobalScale();
                zeus::CColor color = xf4_electric->GetModulationColor();
                xf4_electric.reset(new CParticleElectric(xe8_electricToken));
                zeus::CTransform newXf = GetTransform();
                newXf.origin = zeus::CVector3f::skZero;
                xf4_electric->SetOrientation(newXf);
                xf4_electric->SetTranslation(GetTranslation());
                xf4_electric->SetLocalScale(scale);
                xf4_electric->SetParticleEmission(active);
                xf4_electric->SetModulationColor(color);
            }
        }
    }
    else if (msg == EScriptObjectMessage::Registered)
    {
        if (x104_particleSystem && x104_particleSystem->SystemHasLight())
        {
            x108_lightId = mgr.AllocateUniqueId();
            mgr.AddObject(new CGameLight(x108_lightId, GetAreaIdAlways(), true, std::string("EffectPLight_") + GetName().data(), x34_transform,  GetUniqueId(), x104_particleSystem->GetLight(), x10c_partId.Value(), 1, 0.f));
        }
    }
    else if (msg == EScriptObjectMessage::Deleted)
    {
        if (x108_lightId != kInvalidUniqueId)
        {
            mgr.FreeScriptObject(x108_lightId);
            x108_lightId = kInvalidUniqueId;
        }
    }
    else if (msg == EScriptObjectMessage::InitializedInArea)
    {
        for (const SConnection& conn : x20_conns)
        {
            if (!(conn.x0_state == EScriptObjectState::Active && conn.x4_msg == EScriptObjectMessage::Deactivate) ||
                    !(conn.x0_state == EScriptObjectState::Modify && conn.x4_msg == EScriptObjectMessage::Activate))
                continue;

            auto search = mgr.GetIdListForScript(conn.x8_objId);
            for (auto it = search.first; it != search.second; ++it)
            {
                if (TCastToConstPtr<CScriptTrigger>(mgr.GetObjectById(it->second)))
                    x13c_triggerId = it->second;
            }
        }
    }

    CActor::AcceptScriptMsg(msg, uid, mgr);

    TCastToPtr<CActor> act = mgr.ObjectById(x108_lightId);
    mgr.SendScriptMsg(act, uid, msg);

    if (active != GetActive())
    {
        std::vector<TUniqueId> playIds;
        for (const SConnection& conn : x20_conns)
        {
            if (conn.x0_state != EScriptObjectState::Play || conn.x4_msg != EScriptObjectMessage::Activate)
                continue;

            TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
            if (uid != kInvalidUniqueId)
                playIds.push_back(uid);
        }

        if (playIds.size() > 0)
        {
            TCastToConstPtr<CActor> otherAct = mgr.GetObjectById(playIds[u32(0.99f * playIds.size() * mgr.GetActiveRandom()->Float())]);
            if (otherAct)
                act->SetTransform(otherAct->GetTransform());
            else
                act->SetTransform(GetTransform());
        }
        x110_24_ = true;
        if (x104_particleSystem)
            x104_particleSystem->SetParticleEmission(GetActive());
        if (xf4_electric)
            xf4_electric->SetParticleEmission(GetActive());

        if (GetActive())
            x12c_ = zeus::max(x12c_, x130_);
    }
}

void CScriptEffect::PreRender(CStateManager& mgr, const zeus::CFrustum&)
{
    if (x110_27_ || x111_24_)
    {
        float f31 = 1.f;
        const CGameCamera* cam = mgr.GetCameraManager()->GetCurrentCamera(mgr);
        float f6 = cam->GetTranslation().y - GetTranslation().y;
        float f4 = cam->GetTranslation().x - GetTranslation().x;
        float f5 = cam->GetTranslation().z -  GetTranslation().z;
        float f1 = (f6 * f6);
        float f3 = (f5 * f5);
        f1 = (f4 * f4) + f1;
        f1 = f3 + f1;

        if (f1 <= 0.001f)
            f3 = 0.f;
        else if (f1 != 0.f)
            f3 = f1 * zeus::sqrtF(f1);
        if (x110_27_ && x118_ < f1)
        {
            float f0 = x114_;
            float f2 = x11c_;
            f0 = f3 / f0;
            f1 = 1.f - f2;
            f31 = f1 * f0 + f2;
        }
        if (x111_24_)
        {
            float f4 = x120_;
            float f2 = x124_;
            f1 = f3 - f4;
            f2 = f2 - f4;

            f1 = zeus::max(0.f, f1);
            f1 = f1 / f2;
            f2 = zeus::max(0.f, f1);
            f1 = 1.f - f2;
            float f0 = x128_;
            f0 = f2 * f0;
            f31 = f1 * f31 + f0;
        }

        x104_particleSystem->SetGeneratorRate(f31);
    }

    if (!mgr.GetObjectById(x13c_triggerId))
        x13c_triggerId = kInvalidUniqueId;
}

void CScriptEffect::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{
    if (!x111_26_canRender)
    {
        const_cast<CScriptEffect&>(*this).x12c_ = zeus::max(x12c_, x134_);
        return;
    }

    if (!frustum.aabbFrustumTest(x9c_renderBounds))
        return;
    const_cast<CScriptEffect&>(*this).x12c_ = zeus::max(x12c_, x134_);

    if (x110_31_)
    {
        bool visible = false;
        const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
        if (visor == CPlayerState::EPlayerVisor::Combat || visor == CPlayerState::EPlayerVisor::Scan)
            visible = x110_28_;
        else if (visor == CPlayerState::EPlayerVisor::XRay)
            visible = x110_30_;
        else if (visor == CPlayerState::EPlayerVisor::Thermal)
            visible = x110_29_;

        if (visible && x138_actorLights)
        {
            const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
            const_cast<CScriptEffect&>(*this).x138_actorLights->BuildAreaLightList(mgr, *area, zeus::CAABox{x9c_renderBounds.center(), x9c_renderBounds.center()});
            const_cast<CScriptEffect&>(*this).x138_actorLights->BuildDynamicLightList(mgr, x9c_renderBounds);
        }
        EnsureRendered(mgr);
    }
}

void CScriptEffect::Render(const CStateManager& mgr) const
{
//    if (x138_actorLights)
//        x138_actorLights->ActivateLights();
    if (x104_particleSystem && x104_particleSystem->GetParticleCountAll() > 0)
    {
        g_NumParticlesRendered += x104_particleSystem->GetParticleCountAll();
        x104_particleSystem->Render();
    }

    if (xf4_electric && xf4_electric->GetParticleCount() > 0)
    {
        g_NumParticlesRendered += xf4_electric->GetParticleCount();
        xf4_electric->Render();
    }
}

void CScriptEffect::Think(float dt, CStateManager& mgr)
{
    if (xe4_28_transformDirty)
    {
        if (x104_particleSystem)
        {
            zeus::CTransform newXf = x34_transform;
            newXf.origin = zeus::CVector3f::skZero;
            x104_particleSystem->SetOrientation(newXf);
            x104_particleSystem->SetTranslation(x34_transform.origin);
        }
        if (xf4_electric)
        {
            zeus::CTransform newXf = x34_transform;
            newXf.origin = zeus::CVector3f::skZero;
            xf4_electric->SetOrientation(newXf);
            xf4_electric->SetTranslation(x34_transform.origin);
        }

        if (TCastToPtr<CActor> act = mgr.ObjectById(x108_lightId))
            act->SetTransform(GetTransform());

        xe4_28_transformDirty = false;
    }

    if (x110_25_)
    {
        const CGameArea* area = mgr.GetWorld()->GetAreaAlways(GetAreaIdAlways());
        bool visible = area->GetActive() ? bool(area->GetOcclusionState()) : false;

        if (!visible || x12c_ <= 0.f)
            return;
    }
    else if (x12c_ <= 0.f)
        return;

    x12c_ -= dt;

    if (x110_24_)
    {
        if (x104_particleSystem)
        {
            x104_particleSystem->Update(dt);
            g_NumParticlesUpdating += x104_particleSystem->GetParticleCountAll();
        }

        if (xf4_electric)
        {
            xf4_electric->Update(dt);
            g_NumParticlesUpdating += xf4_electric->GetParticleCount();
        }
    }

    if (x108_lightId != kInvalidUniqueId)
    {
        if (TCastToPtr<CGameLight> light = mgr.ObjectById(x108_lightId))
        {
            if (x30_24_active)
                light->SetLight(x104_particleSystem->GetLight());
        }
    }

    if (x111_25_)
    {
        x140_ += dt;
        if (x140_ > 15.f || AreBothSystemsDeleteable())
        {
            mgr.FreeScriptObject(GetUniqueId());
            return;
        }
    }

    if (x104_particleSystem)
    {
        if (xb4_drawFlags.x0_blendMode != 0)
            x104_particleSystem->SetModulationColor(xb4_drawFlags.x4_color);
        else
            x104_particleSystem->SetModulationColor(zeus::CColor::skWhite);
    }
}

void CScriptEffect::CalculateRenderBounds()
{
    std::experimental::optional<zeus::CAABox> particleBounds;
    if (x104_particleSystem)
        particleBounds = x104_particleSystem->GetBounds();

    std::experimental::optional<zeus::CAABox> electricBounds;
    if (xf4_electric)
        electricBounds = xf4_electric->GetBounds();

    if (particleBounds || electricBounds)
    {
        zeus::CAABox renderBounds = zeus::CAABox::skNullBox;
        if (particleBounds)
        {
            renderBounds.accumulateBounds(particleBounds->min);
            renderBounds.accumulateBounds(particleBounds->max);
        }
        if (electricBounds)
        {
            renderBounds.accumulateBounds(electricBounds->min);
            renderBounds.accumulateBounds(electricBounds->max);
        }
        x9c_renderBounds = renderBounds;
        x111_26_canRender = true;
    }
    else
    {
        x9c_renderBounds = {GetTranslation(), GetTranslation()};
        x111_26_canRender = false;
    }
}

zeus::CAABox CScriptEffect::GetSortingBounds(const CStateManager& mgr) const
{
    if (x13c_triggerId == kInvalidUniqueId)
        return x9c_renderBounds;
    else
        return static_cast<const CScriptTrigger*>(mgr.GetObjectById(x13c_triggerId))->GetTriggerBoundsWR();
}

bool CScriptEffect::AreBothSystemsDeleteable()
{
    return x104_particleSystem->IsSystemDeletable() && xf4_electric->IsSystemDeletable();
}
}
