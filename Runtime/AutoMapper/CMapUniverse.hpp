#ifndef __URDE_CMAPUNIVERSE_HPP__
#define __URDE_CMAPUNIVERSE_HPP__

#include "RetroTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CTransform.hpp"
#include "IFactory.hpp"
#include "CToken.hpp"
#include "CMapArea.hpp"

namespace urde
{
class CStateManager;
class CMapUniverse
{
public:
    class CMapUniverseDrawParms
    {
    public:
        CMapUniverseDrawParms(float, int, const CStateManager&,
                              const zeus::CTransform&, const zeus::CTransform&);
        s32 GetFocusWorldIndex() const;
        zeus::CTransform GetCameraTransform() const;
        zeus::CTransform GetPaneProjectionTransform() const;
        float GetAlpha() const;
    };

    class CMapObjectSortInfo
    {
    public:
        CMapObjectSortInfo(float, int, int, int, const zeus::CColor&, const zeus::CColor&);
        zeus::CColor GetOutlineColor() const;
        zeus::CColor GetSurfaceColor() const;
        s32 GetObjectIndex() const;
        s32 GetAreaIndex() const;
        s32 GetWorldIndex() const;
        float GetZDistance() const;
    };

    class CMapWorldData
    {
        std::string x0_label;
        ResId x10_worldAssetId;
        zeus::CTransform x14_transform;
        std::vector<zeus::CTransform> x44_areaData;
        zeus::CColor x54_;
        zeus::CColor x58_ = zeus::CColor(1.0f, 0.0f, 1.0f);
        zeus::CColor x5c_ = zeus::CColor(1.0f, 0.0f, 1.0f);
        zeus::CColor x60_ = zeus::CColor(1.0f, 0.0f, 1.0f);
        zeus::CVector3f x64_ = zeus::CVector3f::skZero;
    public:
        CMapWorldData(CInputStream& in, u32 version);
        ResId GetWorldAssetId() const { return x10_worldAssetId; }
        zeus::CVector3f GetWorldCenterPoint() const;
        std::string GetWorldLabel() const;
        zeus::CTransform GetWorldTransform() const;
        void GetMapAreaData(s32) const;
        zeus::CColor GetOutlineColorUnselected() const;
        zeus::CColor GetOutlineColorSelected() const;
        zeus::CColor GetSurfaceColorUnselected() const;
        zeus::CColor GetSurfaceColorSelected() const;
        u32 GetNumMapAreaDatas() const;
    };

private:
    ResId x0_hexagonId;
    TLockedToken<CMapArea> x4_hexagonToken;
    std::vector<CMapWorldData> x10_worldDatas;
    zeus::CVector3f x20_ = zeus::CVector3f::skZero;
public:
    CMapUniverse(CInputStream&, u32);
    const CMapWorldData& GetMapWorldData(s32 idx) const { return x10_worldDatas[idx]; }
    u32 GetNumMapWorldDatas() const { return x10_worldDatas.size(); }
    float GetMapUniverseRadius() const;
    zeus::CVector3f GetMapUniverseCenterPoint() const;
    void Draw(const CMapUniverseDrawParms&, const zeus::CVector3f&, float, float) const;
};

CFactoryFnReturn FMapUniverseFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms);

}
#endif
