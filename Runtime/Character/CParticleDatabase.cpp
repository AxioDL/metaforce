#include "CParticleDatabase.hpp"

namespace urde
{

void CParticleDatabase::CacheParticleDesc(const CCharacterInfo::CParticleResData& desc)
{
}

void CParticleDatabase::SetModulationColorAllActiveEffects(const zeus::CColor& color)
{
}

void CParticleDatabase::SuspendAllActiveEffects(CStateManager& stateMgr)
{
}

void CParticleDatabase::StartEffect(const std::string& name, u32 flags, const CParticleData& data,
                                    const zeus::CVector3f& scale, CStateManager& stateMgr, TAreaId aid, u32 unk1)
{
}

void CParticleDatabase::Update(float dt, const CPoseAsTransforms& pose, const CCharLayoutInfo& charInfo,
                               const zeus::CTransform& xf, const zeus::CVector3f& vec, CStateManager& stateMgr)
{
}

void CParticleDatabase::AddToRendererClipped(const zeus::CFrustum& frustum)
{
}

}
