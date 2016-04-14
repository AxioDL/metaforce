#include "CModelData.hpp"
#include "CAnimData.hpp"

namespace urde
{

CModelData::CModelData() {}

CModelData::CModelData(const CStaticRes& res)
{
}

CModelData::CModelData(const CAnimRes& res)
{
}

zeus::CVector3f CModelData::GetAdvancementDeltas(const CCharAnimTime& a,
                                                 const CCharAnimTime& b) const
{
}

void CModelData::Render(const CStateManager& stateMgr, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags) const
{
}

void CModelData::GetRenderingModel(const CStateManager& stateMgr)
{
}

void CModelData::PickAnimatedModel(EWhichModel) const
{
}

void CModelData::PickStaticModel(EWhichModel) const
{
}

void CModelData::SetXRayModel(const std::pair<TResId, TResId>& modelSkin)
{
}

void CModelData::SetInfraModel(const std::pair<TResId, TResId>& modelSkin)
{
}

bool CModelData::IsDefinitelyOpaque(EWhichModel) const
{
}

bool CModelData::GetIsLoop() const
{
}

float CModelData::GetAnimationDuration(int) const
{
}

void CModelData::EnableLooping(bool)
{
}

void CModelData::AdvanceParticles(const zeus::CTransform& xf, float,
                                  CStateManager& stateMgr)
{
}

zeus::CAABox CModelData::GetBounds() const
{
}

zeus::CAABox CModelData::GetBounds(const zeus::CTransform& xf) const
{
}

zeus::CTransform CModelData::GetScaledLocatorTransformDynamic(const std::string& name,
                                                              const CCharAnimTime* time) const
{
}

zeus::CTransform CModelData::GetScaledLocatorTransform(const std::string& name) const
{
}

zeus::CTransform CModelData::GetLocatorTransformDynamic(const std::string& name,
                                                        const CCharAnimTime* time) const
{
}

zeus::CTransform CModelData::GetLocatorTransform(const std::string& name) const
{
}

void CModelData::AdvanceAnimationIgnoreParticles(float dt, CRandom16&, bool)
{
}

void CModelData::AdvanceAnimation(float dt, CStateManager& stateMgr, bool)
{
}

bool CModelData::IsAnimating() const
{
}

bool CModelData::IsInFrustum(const zeus::CTransform& xf,
                             const CFrustumPlanes& frustum) const
{
}

void CModelData::RenderParticles(const CFrustumPlanes& frustum) const
{
}

void CModelData::Touch(EWhichModel, int) const
{
}

void CModelData::Touch(const CStateManager& stateMgr, int) const
{
}

void CModelData::RenderThermal(const zeus::CTransform& xf,
                               const zeus::CColor& a, const zeus::CColor& b) const
{
}

void CModelData::RenderUnsortedParts(EWhichModel, const zeus::CTransform& xf,
                                     const CActorLights* lights, const CModelFlags& drawFlags) const
{
}

void CModelData::Render(EWhichModel, const zeus::CTransform& xf,
                        const CActorLights* lights, const CModelFlags& drawFlags) const
{
}

}
