#ifndef __URDE_MP1_CSPACEPIRATE_HPP__
#define __URDE_MP1_CSPACEPIRATE_HPP__

#include "World/CPatterned.hpp"
#include "Weapon/CProjectileInfo.hpp"

namespace urde::MP1
{

class CSpacePirate : public CPatterned
{
public:
    DEFINE_PATTERNED(SpacePirate)

private:
    class CSpacePirateData
    {
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
        u32 xac_;
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

    CSpacePirateData x568_;
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
        };

        u32 _dummy = 0;
    };

public:
    CSpacePirate(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const CActorParameters&, const CPatternedInfo&, CInputStream&, u32);

    void Accept(IVisitor &visitor);
};
}
#endif // __URDE_MP1_CSPACEPIRATE_HPP__
