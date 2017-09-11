#include "CHUDBillboardEffect.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "Camera/CGameCamera.hpp"

namespace urde
{

u32 CHUDBillboardEffect::g_IndirectTexturedBillboardCount = 0;
u32 CHUDBillboardEffect::g_BillboardCount = 0;

CHUDBillboardEffect::CHUDBillboardEffect(const std::experimental::optional<TToken<CGenDescription>>& particle,
                                         const std::experimental::optional<TToken<CElectricDescription>>& electric,
                                         TUniqueId uid, bool active, const std::string& name, float f,
                                         const zeus::CVector3f& v0, const zeus::CColor& color,
                                         const zeus::CVector3f& v1, const zeus::CVector3f& v2)
: CEffect(uid, CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList), active, name, zeus::CTransform::Identity())
{
    xec_v2 = v2;
    xec_v2.y += f;
    xf8_ = v1 * v0;
    x104_24_ = true;
    x104_25_ = false;
    x104_26_ = false;
    x104_27_ = false;

    if (particle)
    {
        xe8_generator = std::make_unique<CElementGen>(*particle);
        if (static_cast<CElementGen&>(*xe8_generator).IsIndirectTextured())
            ++g_IndirectTexturedBillboardCount;
    }
    else
    {
        xe8_generator = std::make_unique<CParticleElectric>(*electric);
    }
    ++g_BillboardCount;
    xe8_generator->SetModulationColor(color);
    xe8_generator->SetLocalScale(xf8_);
}

CHUDBillboardEffect::~CHUDBillboardEffect()
{
    --g_BillboardCount;
    if (xe8_generator->Get4CharId() == FOURCC('PART'))
        if (static_cast<CElementGen&>(*xe8_generator).IsIndirectTextured())
            --g_IndirectTexturedBillboardCount;
}

void CHUDBillboardEffect::Accept(IVisitor& visitor) { visitor.Visit(this); }

float CHUDBillboardEffect::GetNearClipDistance(CStateManager& mgr)
{
    return mgr.GetCameraManager()->GetCurrentCamera(mgr)->GetNearClipDistance() + 0.01f;
}

zeus::CVector3f CHUDBillboardEffect::GetScaleForPOV(CStateManager& mgr)
{
    return {0.155f, 1.f, 0.155f};
}

}