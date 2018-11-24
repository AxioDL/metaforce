#pragma once

#include "World/CPatterned.hpp"
#include "Weapon/CProjectileInfo.hpp"
#include "Character/CBoneTracking.hpp"
#include "Character/CIkChain.hpp"
#include "Character/CRagDoll.hpp"
#include "World/CPathFindSearch.hpp"
#include "Weapon/CBurstFire.hpp"

namespace urde::MP1
{
class CSpacePirate;

class CPirateRagDoll : public CRagDoll
{
    CSpacePirate* x6c_spacePirate;
    u16 x70_s1;
    float x74_ = 0.f;
    zeus::CVector3f x78_;
    zeus::CVector3f x84_;
    rstl::reserved_vector<TUniqueId, 4> x90_waypoints;
    rstl::reserved_vector<u32, 4> x9c_wpParticleIdxs;
    bool xb0_24_ : 1;
public:
    CPirateRagDoll(CStateManager& mgr, CSpacePirate* sp, u16 s1, u32 flags);

    void PreRender(const zeus::CVector3f& v, CModelData& mData);
    void Update(CStateManager& mgr, float dt, float f2);
    void Prime(CStateManager& mgr, const zeus::CTransform& xf, CModelData& mData);
};

class CSpacePirate : public CPatterned
{
    friend class CPirateRagDoll;
public:
    DEFINE_PATTERNED(SpacePirate)
private:
    class CSpacePirateData
    {
        friend class CSpacePirate;
        float x0_;
        float x4_;
        float x8_;
        float xc_;
        float x10_;
        float x14_;
        u32 x18_;
        bool x1c_;
        CProjectileInfo x20_;
        u16 x48_;
        CDamageInfo x4c_;
        float x68_;
        CProjectileInfo x6c_;
        float x94_;
        u16 x98_;
        float x9c_;
        float xa0_;
        u16 xa4_;
        float xa8_;
        u32 xac_firstBurstCount;
        float xb0_;
        float xb4_;
        float xb8_;
        float xbc_;
        u16 xc0_;
        u16 xc2_;
        float xc4_;
        float xc8_;
    public:
        CSpacePirateData(CInputStream&, u32);
    };

    CSpacePirateData x568_pirateData;
    union
    {
        struct
        {
            bool x634_24_ : 1;
            bool x634_25_ : 1;
            bool x634_26_ : 1;
            bool x634_27_ : 1;
            bool x634_28_ : 1;
            bool x634_29_ : 1;
            bool x634_30_ : 1;
            bool x634_31_ : 1;
            bool x635_24_ : 1;
            bool x635_25_ : 1;
            bool x635_26_ : 1;
            bool x635_27_ : 1;
            bool x635_28_ : 1;
            bool x635_29_ : 1;
            bool x635_30_ : 1;
            bool x635_31_ : 1;
            bool x636_24_trooper : 1;
            bool x636_25_ : 1;
            bool x636_26_ : 1;
            bool x636_27_ : 1;
            bool x636_28_ : 1;
            bool x636_29_ : 1;
            bool x636_30_ : 1;
            bool x636_31_ : 1;
            bool x637_24_ : 1;
            bool x637_25_ : 1;
            bool x637_26_ : 1;
            bool x637_27_ : 1;
            bool x637_28_ : 1;
            bool x637_29_ : 1;
            bool x637_30_ : 1;
            bool x637_31_prevInCineCam : 1;
            bool x638_24_ : 1;
            bool x638_25_ : 1;
            bool x638_26_ : 1;
            bool x638_27_ : 1;
            bool x638_28_ : 1;
            bool x638_29_ : 1;
            bool x638_30_ : 1;
            bool x638_31_ : 1;
            bool x639_24_ : 1;
            bool x639_25_ : 1;
            bool x639_26_ : 1;
            bool x639_27_ : 1;
            bool x639_28_ : 1;
            bool x639_29_ : 1;
            bool x639_30_ : 1;
            bool x639_31_ : 1;
            bool x63a_24_ : 1;
        };

        u64 _dummy = 0;
    };

    u32 x63c_ = 0;
    TUniqueId x640_ = kInvalidUniqueId;
    TUniqueId x642_ = kInvalidUniqueId;
    float x644_ = 1.f;
    zeus::CVector3f x648_ = zeus::CVector3f::skForward;
    zeus::CVector3f x654_;
    CPathFindSearch x660_;
    float x744_ = 0.f;
    float x748_ = 0.f;
    u32 x74c_ = 0;
    float x750_;
    float x754_ = 0.f;
    CSegId x758_headSeg;
    u32 x75c_ = 0;
    s32 x760_ = -1;
    CBoneTracking x764_;
    s32 x79c_ = -1;
    float x7a4_intoJumpDist = 1.f;
    float x7a8_ = 2.f;
    float x7ac_ = 0.f;
    u32 x7b0_ = 0;
    TUniqueId x7b4_ = kInvalidUniqueId;
    CSegId x7b6_gunSeg;
    CSegId x7b7_elbowSeg;
    CSegId x7b8_wristSeg;
    CSegId x7b9_swooshSeg;
    float x7bc_ = 1.f;
    TUniqueId x7c0_ = kInvalidUniqueId;
    CBurstFire x7c4_;
    float x824_ = 3.f;
    zeus::CVector3f x828_;
    s32 x834_ = -1;
    float x838_ = 0.f;
    s32 x83c_ = -1;
    TUniqueId x840_ = kInvalidUniqueId;
    s32 x844_ = -1;
    float x848_dodgeDist = 3.f;
    float x84c_breakDodgeDist = 3.f;
    float x850_ = FLT_MAX;
    float x854_ = FLT_MAX;
    float x858_ = 0.f;
    std::unique_ptr<CPirateRagDoll> x85c_ragDoll;
    CIkChain x860_ikChain;
    float x8a8_ = 0.f;
    float x8ac_ = 0.f;
    float x8b0_ = 0.f;
    float x8b4_ = 0.5f;
    float x8b8_;
    float x8bc_;
    float x8c0_;
    float x8c4_;
    TUniqueId x8c8_ = kInvalidUniqueId;
    zeus::CColor x8cc_trooperColor = zeus::CColor::skWhite;
    zeus::CVector2f x8d0_;
    float x8d8_ = 0.f;
    float x8dc_ = 0.f;

    static const SBurst skBurstsSeatedOOV[];
    static const SBurst skBurstsInjuredOOV[];
    static const SBurst skBurstsJumpingOOV[];
    static const SBurst skBurstsFrenziedOOV[];
    static const SBurst skBurstsStandardOOV[];
    static const SBurst skBurstsQuickOOV[];
    static const SBurst skBurstsSeated[];
    static const SBurst skBurstsInjured[];
    static const SBurst skBurstsJumping[];
    static const SBurst skBurstsFrenzied[];
    static const SBurst skBurstsStandard[];
    static const SBurst skBurstsQuick[];
    static const SBurst* skBursts[];

public:
    CSpacePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

    void Accept(IVisitor &visitor);
    void Think(float dt, CStateManager&);
};
}
