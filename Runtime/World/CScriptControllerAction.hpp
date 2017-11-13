#ifndef __URDE_CSCRIPTCONTROLLERACTION_HPP__
#define __URDE_CSCRIPTCONTROLLERACTION_HPP__

#include "CEntity.hpp"
#include "Input/ControlMapper.hpp"

namespace urde
{

class CScriptControllerAction : public CEntity
{
    ControlMapper::ECommands x34_command;
    u32 x38_mapScreenSubaction;
    union
    {
        struct
        {
            bool x3c_24_mapScreenResponse : 1;
            bool x3c_25_deactivateOnClose : 1;
            bool x3c_26_pressed : 1;
        };
        u8 _dummy = 0;
    };
public:
    CScriptControllerAction(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                            bool active, ControlMapper::ECommands command, bool b1, u32 w1, bool b2);
    void Accept(IVisitor& visitor);
    void Think(float, CStateManager&);
};

}

#endif // __URDE_CSCRIPTCONTROLLERACTION_HPP__
