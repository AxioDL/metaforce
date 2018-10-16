#pragma once

#include "CEntity.hpp"
#include "Camera/CCameraShakeData.hpp"

namespace urde
{

class CScriptCameraShaker : public CEntity
{
    CCameraShakeData x34_shakeData;
public:
    CScriptCameraShaker(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                        bool active, const CCameraShakeData& shakeData);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
};

}

