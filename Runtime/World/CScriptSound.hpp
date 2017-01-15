#ifndef __URDE_CSCRIPTSOUND_HPP__
#define __URDE_CSCRIPTSOUND_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptSound : public CActor
{
public:
    CScriptSound(TUniqueId, const std::string& name, const CEntityInfo& info,
                 const zeus::CTransform& xf, s16 soundId, bool, float, float, float,
                 u32, u32, u32, u32, u32, bool, bool, bool, bool, bool, bool, bool, bool, u32);

    void Accept(IVisitor& visitor);
};

}

#endif // __URDE_CSCRIPTSOUND_HPP__
