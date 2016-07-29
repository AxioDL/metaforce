#ifndef __URDE_CMAPAREA_HPP__
#define __URDE_CMAPAREA_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CMapArea
{
public:
    class CMapAreaSurface
    {
    public:
        void PostConstruct(const void*);
        void Draw(const zeus::CVector3f*, const zeus::CColor&, const zeus::CColor&, float) const;
        const zeus::CVector3f& GetNormal() const;
        const zeus::CVector3f& GetCenterPosition() const;
    };

private:
public:
    CMapArea(CInputStream&, uint);
    void PostConstruct();
    bool GetIsVisibleToAutoMapper(bool, bool) const;
    zeus::CVector3f GetAreaCenterPoint() const;
    zeus::CAABox GetBoundingBox() const;
    const zeus::CVector3f& GetVertices() const;
    void GetMappableObject(s32) const;
    void GetSurface(s32) const;
    u32 GetNumMappableObjects() const;
    u32 GetNumSurfaces() const;

};
}
#endif // __URDE_CMAPAREA_HPP__
