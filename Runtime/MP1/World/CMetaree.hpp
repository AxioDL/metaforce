#ifndef URDE_CMETAREE_HPP
#define URDE_CMETAREE_HPP

#include "World/CPatterned.hpp"
#include "World/CDamageInfo.hpp"

namespace urde::MP1
{
class CMetaree : public CPatterned
{
    float x568_;
    float x56c_;
    float x570_;
    zeus::CVector3f x574_;
    float x580_;
    zeus::CVector3f x584_;
    float x590_ = 0.f;
    float x594_ = 0.f;
    float x598_ = 0.f;
    zeus::CVector3f x59c_;
    u32 x5a8_ = 0;
    CDamageInfo x5ac_damgeInfo;
    u16 x5c8_ = 549;

    struct
    {
        struct
        {
            bool x5ca_24_ : 1;
            bool x5ca_25_ : 1;
            bool x5ca_26_ : 1;
        };
        u16 _dummy;
    };

    u32 x5cc_;
public:
    DEFINE_PATTERNED(Metaree)
    CMetaree(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
             const CPatternedInfo&, const CDamageInfo&, float, const zeus::CVector3f&, float, EBodyType, float, float,
             const CActorParameters&);

    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

    bool Delay(CStateManager&, float)
    {
        return x330_stateMachineState.GetTime() == x568_;
    }
};
}

#endif //URDE_CMETAREE_HPP
