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
    };

    class CMapAreaBFSInfo
    {
        s32 x0_areaIdx;
        s32 x4_depth;
        float x8_;
        float xc_;
    public:
        CMapAreaBFSInfo(s32 areaIdx, s32 depth, float a, float b)
        : x0_areaIdx(areaIdx), x4_depth(depth), x8_(a), xc_(b) {}
        s32 GetAreaIndex() const { return x0_areaIdx; }
        s32 GetDepth() const { return x4_depth; }
        float GetOutlineDrawDepth() const;
        float GetSurfaceDrawDepth() const;
    };

    class CMapObjectSortInfo
    {
    public:
        enum class EObjectCode
        {
        };
    private:
    public:
        CMapObjectSortInfo(float, int, EObjectCode, int, const zeus::CColor&, const zeus::CColor& );
        const zeus::CColor& GetOutlineColor() const;
        const zeus::CColor& GetSurfaceColor() const;
        u32 GetLocalObjectIndex();
        EObjectCode GetObjectCode() const;
        u32 GetAreaIndex() const;
        float GetZDistance() const;
    };

    class CMapAreaData
    {
        TCachedToken<CMapArea> x0_area;
    public:
        CMapAreaData(u32, EMapAreaList, CMapAreaData*);
        void Lock();
        void Unlock();
        bool IsLoaded() const;
        const CMapArea* GetMapArea() const { return x0_area.IsLoaded() ? x0_area.GetObj() : nullptr; }
        void GetNextMapAreaData() const;
        void GetContainingList() const;
        void NextMapAreaData();
        void SetContainingList(EMapAreaList);
        void SetNextMapArea(CMapAreaData*);
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
        float x2c_flashPulse;
        float x30_flashIntensity;
        float x34_objectScale;
        bool x38_sortDoorSurfs;
    public:
        CMapWorldDrawParms(float alphaSurfVisited, float alphaOlVisited,
                           float alphaSurfUnvisited, float alphaOlUnvisited,
                           float alpha, float outlineWidthScale, const CStateManager& mgr,
                           const zeus::CTransform& modelXf, const zeus::CTransform& viewXf,
                           const IWorld& wld, const CMapWorldInfo& mwInfo, float flashPulse,
                           float flashIntensity, float objectScale, bool sortDoorSurfs)
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
          x2c_flashPulse(flashPulse),
          x30_flashIntensity(flashIntensity),
          x34_objectScale(objectScale),
          x38_sortDoorSurfs(sortDoorSurfs)
        {}
        const IWorld& GetWorld() const { return x24_wld; }
        float GetOutlineWidthScale() const { return x14_outlineWidthScale; }
        const zeus::CTransform& GetPlaneProjectionTransform() const { return x1c_modelXf; }
        float GetPlayerAreaFlashIntensity() const { return x30_flashIntensity; }
        float GetPlayerAreaFlashPulse() const { return x2c_flashPulse; }
        const zeus::CTransform& GetCameraTransform() const { return x20_viewXf; }
        float GetAlphaOutlineUnvisited() const { return xc_alphaOlUnvisited; }
        float GetAlphaSurfaceUnvisited() const { return x8_alphaSurfUnvisited; }
        float GetAlphaOutlineVisited() const { return x4_alphaOlVisited; }
        float GetAlphaSurfaceVisited() const { return x0_alphaSurfVisited; }
        const CMapWorldInfo& GetMapWorldInfo() const { return x28_mwInfo; }
        const CStateManager& GetStateManager() const { return x18_mgr; }
        bool GetIsSortDoorSurfaces() const { return x38_sortDoorSurfs; }
    };

private:
    std::vector<CMapAreaData> x0_areas;
public:
    CMapWorld(CInputStream&);
    u32 GetNumAreas() const { return x0_areas.size(); }
    const CMapArea* GetMapArea(TAreaId aid) const { return x0_areas[aid].GetMapArea(); }
    void IsMapAreaInBFSInfoVector(const CMapAreaData*, const std::vector<CMapAreaBFSInfo>&) const;
    void SetWhichMapAreasLoaded(const IWorld&, int start, int count);
    bool IsMapAreasStreaming() const;
    void MoveMapAreaToList(CMapAreaData*, EMapAreaList);
    s32 GetCurrentMapAreaDepth(const IWorld&, TAreaId) const;
    std::vector<TAreaId> GetVisibleAreas(const IWorld&, const CMapWorldInfo&) const;
    void Draw(const CMapWorldDrawParms&, int, int, float, float, bool) const;
    void DoBFS(const IWorld&, TAreaId, int, float, float, bool, std::vector<CMapAreaBFSInfo>&) const;
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
