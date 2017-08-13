#ifndef __URDE_CSCRIPEFFECT_HPP__
#define __URDE_CSCRIPEFFECT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptEffect : public CActor
{
    static u32 g_NumParticlesUpdating;
    static u32 g_NumParticlesRendered;
public:
    CScriptEffect(TUniqueId, const std::string& name, const CEntityInfo& info,
                  const zeus::CTransform& xf, const zeus::CVector3f& scale,
                  CAssetId partId, CAssetId elscId, bool, bool, bool, bool active,
                  bool, float, float, float, float, bool, float, float, float,
                  bool, bool, bool, const CLightParameters& lParms, bool);

    void Accept(IVisitor& visitor);

    static void ResetParticleCounts()
    {
        g_NumParticlesUpdating = 0;
        g_NumParticlesRendered = 0;
    }
};

}

#endif // __URDE_CSCRIPEFFECT_HPP__
