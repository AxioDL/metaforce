#ifndef __URDE_CMAPWORLD_HPP__
#define __URDE_CMAPWORLD_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CTransform.hpp"
#include "CMapArea.hpp"

namespace urde
{
class IWorld;
class CMapWorldInfo;
class CStateManager;
class CMapWorld
{
public:
    /* skDrawProfileItemNames; */
    enum class EMapAreaList
    {
        Loaded,
        Loading,
        Unloaded
    };

    class CMapAreaBFSInfo
    {
        int x0_areaIdx;
        int x4_depth;
        float x8_surfDrawDepth;
        float xc_outlineDrawDepth;
    public:
        CMapAreaBFSInfo(int areaIdx, int depth, float a, float b)
        : x0_areaIdx(areaIdx), x4_depth(depth), x8_surfDrawDepth(a), xc_outlineDrawDepth(b) {}
        int GetAreaIndex() const { return x0_areaIdx; }
        int GetDepth() const { return x4_depth; }
        float GetOutlineDrawDepth() const { return x8_surfDrawDepth; }
        float GetSurfaceDrawDepth() const { return xc_outlineDrawDepth; }
    };

    class CMapObjectSortInfo
    {
        float x0_zDist;
        int x4_areaIdx;
        int x8_typeAndIdx;
        zeus::CColor xc_surfColor;
        zeus::CColor x10_outlineColor;
    public:
        enum class EObjectCode
        {
            Invalid = -1,
            Object = 1 << 16,
            DoorSurface = 2 << 16,
            Door = 3 << 16,
            Surface = 4 << 16
        };

        CMapObjectSortInfo(float zDist, int areaIdx, EObjectCode type, int idx,
                           const zeus::CColor& surfColor, const zeus::CColor& outlineColor)
        : x0_zDist(zDist), x4_areaIdx(areaIdx), x8_typeAndIdx(int(type) | idx),
          xc_surfColor(surfColor), x10_outlineColor(outlineColor) {}
        const zeus::CColor& GetOutlineColor() const { return x10_outlineColor; }
        const zeus::CColor& GetSurfaceColor() const { return xc_surfColor; }
        u32 GetLocalObjectIndex() const { return x8_typeAndIdx & 0xffff; }
        EObjectCode GetObjectCode() const { return EObjectCode(x8_typeAndIdx & 0xffff0000); }
        u32 GetAreaIndex() const { return x4_areaIdx; }
        float GetZDistance() const { return x0_zDist; }
    };

    class CMapAreaData
    {
        TCachedToken<CMapArea> x0_area;
        EMapAreaList x10_list;
        CMapAreaData* x14_next = nullptr;
    public:
        CMapAreaData(CAssetId areaRes, EMapAreaList list, CMapAreaData* next);
        void Lock() { x0_area.Lock(); }
        void Unlock() { x0_area.Unlock(); }
        bool IsLoaded() const { return x0_area.IsLoaded(); }
        const CMapArea* GetMapArea() const { return x0_area.GetObj(); }
        const CMapAreaData* GetNextMapAreaData() const { return x14_next; }
        EMapAreaList GetContainingList() const { return x10_list; }
        CMapAreaData* NextMapAreaData() { return x14_next; }
        void SetContainingList(EMapAreaList list) { x10_list = list; }
        void SetNextMapArea(CMapAreaData* next) { x14_next = next; }
    };

    class CMapWorldDrawParms
    {
        float x0_alphaSurfVisited;
        float x4_alphaOlVisited;
        float x8_alphaSurfUnvisited;
        float xc_alphaOlUnvisited;
        float x10_alpha;
        float x14_outlineWidthScale;
        const CStateManager& x18_mgr;
        const zeus::CTransform& x1c_modelXf;
        const zeus::CTransform& x20_viewXf;
        const IWorld& x24_wld;
        const CMapWorldInfo& x28_mwInfo;
        float x2c_playerFlashIntensity;
        float x30_hintFlashIntensity;
        float x34_objectScale;
        bool x38_sortDoorSurfs;
    public:
        CMapWorldDrawParms(float alphaSurfVisited, float alphaOlVisited,
                           float alphaSurfUnvisited, float alphaOlUnvisited,
                           float alpha, float outlineWidthScale, const CStateManager& mgr,
                           const zeus::CTransform& modelXf, const zeus::CTransform& viewXf,
                           const IWorld& wld, const CMapWorldInfo& mwInfo, float playerFlash,
                           float hintFlash, float objectScale, bool sortDoorSurfs)
        : x0_alphaSurfVisited(alphaSurfVisited),
          x4_alphaOlVisited(alphaOlVisited),
          x8_alphaSurfUnvisited(alphaSurfUnvisited),
          xc_alphaOlUnvisited(alphaOlUnvisited),
          x10_alpha(alpha),
          x14_outlineWidthScale(outlineWidthScale),
          x18_mgr(mgr),
          x1c_modelXf(modelXf),
          x20_viewXf(viewXf),
          x24_wld(wld),
          x28_mwInfo(mwInfo),
          x2c_playerFlashIntensity(playerFlash),
          x30_hintFlashIntensity(hintFlash),
          x34_objectScale(objectScale),
          x38_sortDoorSurfs(sortDoorSurfs)
        {}
        const IWorld& GetWorld() const { return x24_wld; }
        float GetOutlineWidthScale() const { return x14_outlineWidthScale; }
        const zeus::CTransform& GetPlaneProjectionTransform() const { return x1c_modelXf; }
        float GetHintAreaFlashIntensity() const { return x30_hintFlashIntensity; }
        float GetPlayerAreaFlashIntensity() const { return x2c_playerFlashIntensity; }
        const zeus::CTransform& GetCameraTransform() const { return x20_viewXf; }
        float GetAlphaOutlineUnvisited() const { return xc_alphaOlUnvisited; }
        float GetAlphaSurfaceUnvisited() const { return x8_alphaSurfUnvisited; }
        float GetAlphaOutlineVisited() const { return x4_alphaOlVisited; }
        float GetAlphaSurfaceVisited() const { return x0_alphaSurfVisited; }
        float GetAlpha() const { return x10_alpha; }
        const CMapWorldInfo& GetMapWorldInfo() const { return x28_mwInfo; }
        const CStateManager& GetStateManager() const { return x18_mgr; }
        bool GetIsSortDoorSurfaces() const { return x38_sortDoorSurfs; }
        float GetObjectScale() const { return x34_objectScale; }
    };

private:
    std::vector<CMapAreaData> x0_areas;
    rstl::reserved_vector<CMapAreaData*, 3> x10_listHeads;
    std::vector<bool> x20_traversed;
    zeus::CVector3f x30_worldSpherePoint;
    float x3c_worldSphereRadius = 0.f;
    float x40_worldSphereHalfDepth = 0.f;
public:
    CMapWorld(CInputStream&);
    u32 GetNumAreas() const { return x0_areas.size(); }
    const CMapArea* GetMapArea(int aid) const { return x0_areas[aid].GetMapArea(); }
    bool IsMapAreaInBFSInfoVector(const CMapAreaData*, const std::vector<CMapAreaBFSInfo>&) const;
    void SetWhichMapAreasLoaded(const IWorld&, int start, int count);
    bool IsMapAreasStreaming() const;
    void MoveMapAreaToList(CMapAreaData*, EMapAreaList);
    s32 GetCurrentMapAreaDepth(const IWorld&, int areaIdx) const;
    std::vector<int> GetVisibleAreas(const IWorld&, const CMapWorldInfo&) const;
    void Draw(const CMapWorldDrawParms&, int, int, float, float, bool) const;
    void DoBFS(const IWorld&, int, int, float, float, bool, std::vector<CMapAreaBFSInfo>&) const;
    bool IsMapAreaValid(const IWorld&, int, bool) const;
    void DrawAreas(const CMapWorldDrawParms&, int, const std::vector<CMapAreaBFSInfo>&, bool) const;
    void RecalculateWorldSphere(const CMapWorldInfo&, const IWorld&) const;
    zeus::CVector3f ConstrainToWorldVolume(const zeus::CVector3f&, const zeus::CVector3f&) const;
    void ClearTraversedFlags() const;
};

CFactoryFnReturn FMapWorldFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                  CObjectReference* selfRef);

}

#endif // __URDE_CMAPWORLD_HPP__
