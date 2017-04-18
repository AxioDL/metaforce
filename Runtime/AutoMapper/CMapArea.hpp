#ifndef __URDE_CMAPAREA_HPP__
#define __URDE_CMAPAREA_HPP__

#include "RetroTypes.hpp"
#include "CResFactory.hpp"
#include "zeus/CAABox.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class IWorld;
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
    enum class EVisMode
    {
        Always,
        MapStationOrVisit,
        Visit,
        Never
    };

private:
    u32 x0_magic;
    u32 x4_version;
    u32 x8_;
    EVisMode xc_visibilityMode;
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
    const zeus::CAABox& GetBoundingBox() const { return x10_box; }
    const zeus::CVector3f& GetVertices() const;
    void GetMappableObject(s32) const;
    void GetSurface(s32) const;
    u32 GetNumMappableObjects() const;
    u32 GetNumSurfaces() const;
    zeus::CTransform GetAreaPostTransform(const IWorld& world, TAreaId aid) const;
    static const zeus::CVector3f& GetAreaPostTranslate(const IWorld& world, TAreaId aid);
};

CFactoryFnReturn FMapAreaFactory(const SObjectTag& objTag, CInputStream& in, const CVParamTransfer&,
                                 CObjectReference*);
}
#endif // __URDE_CMAPAREA_HPP__
