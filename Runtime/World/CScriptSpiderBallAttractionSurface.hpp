#ifndef __URDE_CSCRIPTSPIDERBALLATTRACTIONSURFACE_HPP__
#define __URDE_CSCRIPTSPIDERBALLATTRACTIONSURFACE_HPP__

#include "CActor.hpp"

namespace urde
{

class CScriptSpiderBallAttractionSurface : public CActor
{
    zeus::CVector3f xe8_scale;
    zeus::CAABox xf4_aabb;
public:
    CScriptSpiderBallAttractionSurface(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                       const zeus::CTransform& xf, const zeus::CVector3f& scale, bool active);
    void Accept(IVisitor& visitor);
    void Think(float dt, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr);
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor& actor, CStateManager& mgr);
    const zeus::CVector3f& GetScale() const { return xe8_scale; }
};

}

#endif // __URDE_CSCRIPTSPIDERBALLATTRACTIONSURFACE_HPP__
