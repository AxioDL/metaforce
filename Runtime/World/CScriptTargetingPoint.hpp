#ifndef __URDE_CSCRIPTTARGETINGPOINT_HPP__
#define __URDE_CSCRIPTTARGETINGPOINT_HPP__

#include "World/CActor.hpp"

namespace urde
{
class CScriptTargetingPoint : public CActor
{
private:
    union
    {
        struct
        {
            bool xe8_e4_ : 1;
        };
        u8 xe8_dummy = 0;
    };
    TUniqueId xea_;
    float xec_time = 0.f;

public:
    CScriptTargetingPoint(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, bool);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &);
    void Think(float, CStateManager &);
    void Render(const CStateManager &) const {}

    bool GetLocked() const;
};
}

#endif // __URDE_CSCRIPTTARGETINGPOINT_HPP__
