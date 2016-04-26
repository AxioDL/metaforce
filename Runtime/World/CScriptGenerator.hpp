#ifndef __URDE_CSCRIPTGENERATOR_HPP__
#define __URDE_CSCRIPTGENERATOR_HPP__

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CScriptGenerator : public CEntity
{
public:
    CScriptGenerator(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                     u32, bool, const zeus::CVector3f&, bool, bool, float, float);
};

}

#endif // __URDE_CSCRIPTGENERATOR_HPP__
