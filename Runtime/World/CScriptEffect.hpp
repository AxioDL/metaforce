#ifndef __URDE_CSCRIPEFFECT_HPP__
#define __URDE_CSCRIPEFFECT_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptEffect : public CActor
{
public:
    CScriptEffect(TUniqueId, const std::string& name, const CEntityInfo& info,
                  const zeus::CTransform& xf, const zeus::CVector3f& scale,
                  ResId partId, ResId elscId, bool, bool, bool, bool active,
                  bool, float, float, float, float, bool, float, float, float,
                  bool, bool, bool, const CLightParameters& lParms, bool);

    void Accept(IVisitor& visitor);
};

}

#endif // __URDE_CSCRIPEFFECT_HPP__
