#ifndef __URDE_CSCRIPTCAMERASHAKER_HPP__
#define __URDE_CSCRIPTCAMERASHAKER_HPP__

#include "CEntity.hpp"
#include "Camera/CCameraShakeData.hpp"

namespace urde
{

class CScriptCameraShaker : public CEntity
{
    CCameraShakeData x34_shakeData;
public:
    CScriptCameraShaker(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                        bool active, const CCameraShakeData& shakeData);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
};

}

#endif // __URDE_CSCRIPTCAMERASHAKER_HPP__
