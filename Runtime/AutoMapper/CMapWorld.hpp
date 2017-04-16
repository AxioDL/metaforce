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
    public:
        CMapWorldDrawParms(float, float, float, float, float, const CStateManager&, const zeus::CTransform&,
                           const zeus::CTransform&, const IWorld&, const CMapWorldInfo&, float, bool);
        void GetWorld() const;
        float GetOutlineWidthScale() const;
        void GetPlaneProjectionTransform() const;
        void GetPlayerAreaFlashIntensity() const;
        void GetCameraTransform() const;
        void GetAlphaOutlineUnvisited() const;
        void GetAlphaSurfaceUnvisited() const;
        void GetAlphaOutlineVisited() const;
        void GetAlphaSurfaceVisited() const;
        void GetMapWorldInfo() const;
        void GetStateManager() const;
        bool GetIsSortDoorSurfaces() const;
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
