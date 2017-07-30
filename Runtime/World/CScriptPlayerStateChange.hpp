#ifndef __URDE_CSCRIPTPLAYERSTATECHANGE_HPP__
#define __URDE_CSCRIPTPLAYERSTATECHANGE_HPP__

#include "World/CEntity.hpp"

namespace urde
{
class CScriptPlayerStateChange : public CEntity
{
public:
    enum class EControl
    {
        Unfiltered,
        Filtered
    };
    enum class EControlCommandOption
    {
        Unfiltered,
        Filtered
    };
private:
    u32 x34_itemType;
    u32 x38_itemCount;
    u32 x3c_itemCapacity;
    EControl x40_ctrl;
    EControlCommandOption x44_ctrlCmdOpt;
public:
    CScriptPlayerStateChange(TUniqueId, const std::string&, const CEntityInfo&, bool, u32, u32, u32, EControl,
                             EControlCommandOption);
    void Accept(IVisitor& visit);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
};
}


#endif // __URDE_CSCRIPTPLAYERSTATECHANGE_HPP__
