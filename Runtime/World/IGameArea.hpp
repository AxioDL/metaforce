#ifndef __URDE_IGAMEAREA_HPP__
#define __URDE_IGAMEAREA_HPP__

#include "RetroTypes.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class IGameArea
{
public:
    class Dock
    {
    public:
        struct SDockReference
        {
            u32 x0_area;
            s16 x4_dock;
            union
            {
                struct
                {
                    bool x6_16_ : 1;
                };
                u16 x6_ = 0;
            };
            SDockReference() = default;
        };
    private:
        u32 x0_ = 0;
        std::vector<SDockReference> x4_dockReferences;
        rstl::reserved_vector<zeus::CVector3f, 4> x14_planeVertices;
        bool x48_;
    public:

        u32 GetReferenceCount() const { return x0_; }
        Dock(CInputStream& in, const zeus::CTransform& xf);
        TAreaId GetConnectedAreaId(s32 other) const;
        s16 GetOtherDockNumber(s32 other) const;
        bool GetShouldLoadOther(s32 other) const;
        void SetShouldLoadOther(s32 other, bool should);
        bool ShouldLoadOtherArea(s32 other) const;
        zeus::CVector3f GetPoint(s32 idx) const;
    };
};
}

#endif // __URDE_IGAMEAREA_HPP__
