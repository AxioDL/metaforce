#ifndef __URDE_CSCRIPTACTORKEYFRAME_HPP__
#define __URDE_CSCRIPTACTORKEYFRAME_HPP__

#include "CEntity.hpp"

namespace urde
{
class CScriptActorKeyframe : public CEntity
{
private:
    s32 x34_animationId;
    float x38_;
    float x3c_;
    float x40_;
    union {
        struct
        {
            bool x44_24_ : 1;
            bool x44_25_ : 1;
            bool x44_26_ : 1;
            bool x44_27_ : 1;
            bool x44_28_ : 1;
            bool x44_29_ : 1;
        };
        u8 _dummy = 0;
    };

public:
    CScriptActorKeyframe(TUniqueId uid, const std::string& name, const CEntityInfo& info, s32 w1, bool b1, float f1,
                         bool b2, u32 w2, bool active, float f2);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void Think(float, CStateManager&);
    void UpdateEntity(TUniqueId, CStateManager&);
};
}

#endif // __URDE_CSCRIPTACTORKEYFRAME_HPP__
