#ifndef __URDE_CSCRIPTPICKUPGENERATOR_HPP__
#define __URDE_CSCRIPTPICKUPGENERATOR_HPP__

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CScriptPickupGenerator : public CEntity
{
    zeus::CVector3f x34_position;
    float x40_frequency;
    float x44_ = 0.f;
    void sub8015E220();
public:
    CScriptPickupGenerator(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CVector3f&, float, bool);

    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager &stateMgr);
};
}
#endif // __URDE_CSCRIPTPICKUPGENERATOR_HPP__
