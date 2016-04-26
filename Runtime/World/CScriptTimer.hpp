#ifndef __URDE_CSCRIPTIMER_HPP__
#define __URDE_CSCRIPTIMER_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptTimer : public CEntity
{
public:
    CScriptTimer(TUniqueId, const std::string& name, const CEntityInfo& info,
                 float, float, bool, bool, bool);
};

}

#endif // __URDE_CSCRIPTIMER_HPP__
