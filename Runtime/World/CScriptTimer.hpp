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
    virtual void Accept(IVisitor&);
};

}

#endif // __URDE_CSCRIPTIMER_HPP__
