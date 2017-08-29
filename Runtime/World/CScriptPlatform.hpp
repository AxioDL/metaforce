#ifndef __URDE_CSCRIPTPLATFORM_HPP__
#define __URDE_CSCRIPTPLATFORM_HPP__

#include "CPhysicsActor.hpp"
#include "optional.hpp"
#include "CToken.hpp"
#include "CHealthInfo.hpp"
#include "CDamageVulnerability.hpp"

namespace urde
{
class CCollidableOBBTreeGroup;
class CCollidableOBBTreeGroupContainer;

struct SRiders
{
    TUniqueId x0_uid;
    u32 x4_;
    zeus::CTransform x8_transform;
};

class CScriptPlatform : public CPhysicsActor
{
    u32 x254_;
    TUniqueId x258_ = kInvalidUniqueId;
    TUniqueId x25a_ = kInvalidUniqueId;
    float x25c_;
    float x260_;
    float x264_;
    float x268_;
    float x26c_;
    zeus::CVector3f x270_;
    zeus::CQuaternion x27c_;
    CHealthInfo x28c_;
    CHealthInfo x294_;
    CDamageVulnerability x29c_;
    rstl::optional_object<TLockedToken<CCollidableOBBTreeGroupContainer>> x304_treeGroupContainer;
    std::unique_ptr<CCollidableOBBTreeGroup> x314_treeGroup;
    std::vector<SRiders> x318_riders;
    std::vector<SRiders> x328_;
    std::vector<SRiders> x338_;
    float x348_;
    u32 x34c_;
    u32 x350_;
    TUniqueId x354_;
    union
    {
        struct
        {
            bool x356_24_ : 1;
            bool x356_25_ : 1;
            bool x356_26_ : 1;
            bool x356_27_ : 1;
            bool x356_28_ : 1;
            bool x356_29_ : 1;
            bool x356_30_ : 1;
            bool x356_31_ : 1;
        };
        u32 x356_dummy = 0;
    };

public:
    CScriptPlatform(TUniqueId, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                    CModelData&& mData, const CActorParameters& actParms, const zeus::CAABox& aabb, float, bool, float,
                    bool, const CHealthInfo& hInfo, const CDamageVulnerability& dInfo,
                    const rstl::optional_object<TLockedToken<CCollidableOBBTreeGroupContainer>>& dcln, bool, u32, u32);

    void Accept(IVisitor& visitor);
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    bool IsRider(TUniqueId id) const;
    std::vector<SRiders>& GetX328() { return x328_; }
    const std::vector<SRiders>& GetX328() const { return x328_; }
    std::vector<SRiders>& GetX338() { return x338_; }
    const std::vector<SRiders>& GetX338() const { return x338_; }
    void AddSlave(TUniqueId, CStateManager&) {}
};
}

#endif // __URDE_CSCRIPTPLATFORM_HPP__
