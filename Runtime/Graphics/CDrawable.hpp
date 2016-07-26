#ifndef __URDE_CDRAWABLE_HPP__
#define __URDE_CDRAWABLE_HPP__

#include "GCNTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
enum class EDrawableType : u16
{
    Surface,
    Particle,
    UnsortedCallback,
    SortedCallback
};

class CDrawable
{
    EDrawableType x0_type;
    u16 x2_extraSort;
    const void* x4_data;
    zeus::CAABox x8_aabb;
    float x20_viewDist;
public:
    CDrawable(EDrawableType dtype, u16 extraSort, float planeDot, const zeus::CAABox& aabb, const void* data)
    : x0_type(dtype), x2_extraSort(extraSort), x4_data(data), x8_aabb(aabb), x20_viewDist(planeDot) {}

    EDrawableType GetType() const {return x0_type;}
    const zeus::CAABox& GetBounds() const {return x8_aabb;}
    float GetDistance() const {return x20_viewDist;}
    const void* GetData() const {return x4_data;}
    u16 GetExtraSort() const {return x2_extraSort;}
};
}

#endif // __URDE_CDRAWABLE_HPP__
