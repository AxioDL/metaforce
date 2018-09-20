#ifndef __URDE_CSCRIPTSTEAM_HPP__
#define __URDE_CSCRIPTSTEAM_HPP__

#include "CScriptTrigger.hpp"

namespace urde
{

class CScriptSteam : public CScriptTrigger
{
    bool x150_;
    CAssetId x154_texture;
    float x158_;
    float x15c_alphaInDur;
    float x160_alphaOutDur;
    float x164_ = 0.f;
    float x168_ = 0.f;
public:
    CScriptSteam(TUniqueId, std::string_view name, const CEntityInfo& info, const zeus::CVector3f& pos,
                 const zeus::CAABox&, const CDamageInfo& dInfo, const zeus::CVector3f& orientedForce,
                 ETriggerFlags flags, bool active, CAssetId, float, float, float, float, bool);

    void Accept(IVisitor&);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
};

}

#endif // __URDE_CSCRIPTSTEAM_HPP__
