#ifndef __URDE_CDRAWABLE_HPP__
#define __URDE_CDRAWABLE_HPP__

#include "GCNTypes.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
enum class EDrawableType
{
    World,
    Particle,
    UnsortedDrawable,
    SortedDrawable
};

class CDrawable
{
public:
    CDrawable(EDrawableType, u16, float, const zeus::CAABox&, const void*);

    EDrawableType GetType() const;
    const zeus::CAABox& GetBounds() const;
    float GetDistance() const;
    void* GetData() const;
    void GetExtraSort() const;
};
}

#endif // __URDE_CDRAWABLE_HPP__
