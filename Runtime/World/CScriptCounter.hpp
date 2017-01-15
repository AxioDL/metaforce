#ifndef __URDE_CSCRIPTCOUNTER_HPP__
#define __URDE_CSCRIPTCOUNTER_HPP__

#include "CEntity.hpp"

namespace urde
{

class CScriptCounter : public CEntity
{
public:
    CScriptCounter(TUniqueId, const std::string& name, const CEntityInfo& info,
                   u32, u32, bool, bool);

    void Accept(IVisitor& visitor);
};

}

#endif // __URDE_CSCRIPTCOUNTER_HPP__
