#ifndef __URDE_CMAPAREA_HPP__
#define __URDE_CMAPAREA_HPP__

#include "RetroTypes.hpp"
#include "CResFactory.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CWorld;
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
    u32 x0_magic;
    u32 x4_version;
    u32 x8_;
    u32 xc_;
    zeus::CAABox x10_box;
    u32 x28_mappableObjCount;
    u32 x2c_vertexCount;
    u32 x30_surfaceCount;
    u32 x34_size;
    u8* x38_moStart;
    u8* x3c_vertexStart;
    u8* x40_surfaceStart;
    std::unique_ptr<u8[]> x44_buf;
public:
    CMapArea(CInputStream&, u32);
    void PostConstruct();
    bool GetIsVisibleToAutoMapper(bool, bool) const;
    zeus::CVector3f GetAreaCenterPoint() const { return x10_box.center(); }
    zeus::CAABox GetBoundingBox() const;
    const zeus::CVector3f& GetVertices() const;
    void GetMappableObject(s32) const;
    void GetSurface(s32) const;
    u32 GetNumMappableObjects() const;
    u32 GetNumSurfaces() const;
    zeus::CTransform GetAreaPostTransform(const CWorld& world, TAreaId aid) const;
};

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&);
}
#endif // __URDE_CMAPAREA_HPP__
