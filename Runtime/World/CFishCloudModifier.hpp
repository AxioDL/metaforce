#ifndef __URDE_CFISHCLOUDMODIFIER_HPP__
#define __URDE_CFISHCLOUDMODIFIER_HPP__

#include "CActor.hpp"

namespace urde
{
class CFishCloudModifier : public CActor
{
    float xe8_;
    float xec_;
    bool xf0_isRepulsor;
    bool xf1_;
public:
    CFishCloudModifier(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, bool, bool, float, float);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

    void AddSelf(CStateManager&);
    void RemoveSelf(CStateManager&);
};
}

#endif
