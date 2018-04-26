#include "CExplosion.hpp"
#include "CStateManager.hpp"
#include "CGameLight.hpp"
#include "TCastTo.hpp"
#include "Graphics/CBooRenderer.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{

CExplosion::CExplosion(const TLockedToken<CGenDescription>& particle, TUniqueId uid, bool active,
                       const CEntityInfo& info, std::string_view name, const zeus::CTransform& xf,
                       u32 flags, const zeus::CVector3f& scale, const zeus::CColor& color)
: CEffect(uid, info, active, name, xf)
{
    xe8_particleGen = std::make_unique<CElementGen>(particle, CElementGen::EModelOrientationType::Normal,
                                                    flags & 0x2 ? CElementGen::EOptionalSystemFlags::Two :
                                                                  CElementGen::EOptionalSystemFlags::One);
    xf0_particleDesc = particle.GetObj();
    xf4_24_renderThermalHot = flags & 0x4;
    xf4_25_ = true;
    xf4_26_renderXray = flags & 0x8;
    xe6_27_thermalVisorFlags = flags & 0x1 ? 1 : 2;
    xe8_particleGen->SetGlobalTranslation(xf.origin);
    xe8_particleGen->SetOrientation(xf.getRotation());
    xe8_particleGen->SetGlobalScale(scale);
    xe8_particleGen->SetModulationColor(color);
}

void CExplosion::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CExplosion::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    if (msg == EScriptObjectMessage::Deleted)
    {
        if (xec_explosionLight != kInvalidUniqueId)
            mgr.FreeScriptObject(xec_explosionLight);
    }
    else if (msg == EScriptObjectMessage::Registered)
    {
        if (xe8_particleGen->SystemHasLight())
        {
            xec_explosionLight = mgr.AllocateUniqueId();
            mgr.AddObject(new CGameLight(xec_explosionLight, GetAreaIdAlways(), true, "ExplodePLight_" + x10_name, x34_transform,
                                         GetUniqueId(), xe8_particleGen->GetLight(), 1, /*xf0_particleDesc*/ 0, 0.f));
        }
    }

    CActor::AcceptScriptMsg(msg, sender, mgr);

    if (xec_explosionLight != kInvalidUniqueId)
        mgr.SendScriptMsgAlways(sender, xec_explosionLight, msg);
}

void CExplosion::Think(float dt, CStateManager& mgr)
{
    if (xe4_28_transformDirty)
    {
        xe8_particleGen->SetGlobalTranslation(GetTranslation());
        xe8_particleGen->SetGlobalOrientation(GetTransform().getRotation());
        xe4_28_transformDirty = false;
    }
    xe8_particleGen->Update(dt);

    if (xec_explosionLight != kInvalidUniqueId)
    {
        TCastToPtr<CGameLight> light = mgr.ObjectById(xec_explosionLight);
        if (light && x30_24_active)
            light->SetLight(xe8_particleGen->GetLight());
    }

    xf8_time += dt;

    if (xf8_time > 15.f || xe8_particleGen->IsSystemDeletable())
        mgr.FreeScriptObject(GetUniqueId());
}

void CExplosion::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum)
{
    CActor::PreRender(mgr, frustum);
    xe4_30_outOfFrustum = !xf4_25_ || !frustum.aabbFrustumTest(x9c_renderBounds);
}

void CExplosion::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const
{
    if (xe4_30_outOfFrustum)
        return;

    if (!(xf4_24_renderThermalHot && mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot)
         && !(xf4_26_renderXray && mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::EPlayerVisor::XRay))
    {
        g_Renderer->AddParticleGen(*xe8_particleGen.get());
        return;
    }

    EnsureRendered(mgr);
}

void CExplosion::Render(const CStateManager& mgr) const
{
    if (mgr.GetThermalDrawFlag() == EThermalDrawFlag::Hot && xf4_24_renderThermalHot)
    {
            CElementGen::SetSubtractBlend(true);
            CBooModel::SetRenderModelBlack(true);
            xe8_particleGen->Render();
            CBooModel::SetRenderModelBlack(false);
            CElementGen::SetSubtractBlend(false);
            return;
    }

    CElementGen::SetSubtractBlend(xf4_24_renderThermalHot);
    CGraphics::SetFog(ERglFogMode::PerspLin, 0.f, 74.f, zeus::CColor::skBlack);
    xe8_particleGen->Render();
    mgr.SetupFogForArea(GetAreaIdAlways());
    CElementGen::SetSubtractBlend(false);
}

bool CExplosion::CanRenderUnsorted(const CStateManager &) const
{
    return false;
}
}
