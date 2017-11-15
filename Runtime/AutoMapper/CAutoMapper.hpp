#ifndef __URDE_CAUTOMAPPER_HPP__
#define __URDE_CAUTOMAPPER_HPP__

#include "RetroTypes.hpp"
#include "CInGameTweakManagerBase.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CVector3f.hpp"
#include "MP1/CInGameGuiManager.hpp"

namespace urde
{
class CFinalInput;
class IWorld;
class CMapWorldInfo;
class CStateManager;
class CMapUniverse;

class CAutoMapper
{
public:
    using EInGameGuiState = MP1::EInGameGuiState;
    enum class ELoadPhase
    {
        LoadResources,
        LoadUniverse,
        Done
    };
    enum class EAutoMapperState
    {
        MiniMap,
        MapScreen,
        MapScreenUniverse
    };
    struct SAutoMapperRenderState
    {
        enum class Ease
        {
            None,
            Linear,
            Out,
            In,
            InOut
        };
        zeus::CVector2i x0_viewportSize;
        zeus::CQuaternion x8_camOrientation;
        float x18_camDist;
        float x1c_camAngle;
        zeus::CVector3f x20_areaPoint;
        float x2c_drawDepth1;
        float x30_drawDepth2;
        float x34_alphaSurfaceVisited;
        float x38_alphaOutlineVisited;
        float x3c_alphaSurfaceUnvisited;
        float x40_alphaOutlineUnvisited;
        Ease x44_viewportEase;
        Ease x48_camEase;
        Ease x4c_pointEase;
        Ease x50_depth1Ease;
        Ease x54_depth2Ease;
        Ease x58_alphaEase;
        SAutoMapperRenderState() = default;
        SAutoMapperRenderState(const zeus::CVector2i& v1, const zeus::CQuaternion& rot,
                               float f1, float f2, const zeus::CVector3f& v2, float f3, float f4,
                               float f5, float f6, float f7, float f8)
        : x0_viewportSize(v1), x8_camOrientation(rot), x18_camDist(f1), x1c_camAngle(f2),
          x20_areaPoint(v2), x2c_drawDepth1(f3), x30_drawDepth2(f4),
          x34_alphaSurfaceVisited(f5), x38_alphaOutlineVisited(f6),
          x3c_alphaSurfaceUnvisited(f7), x40_alphaOutlineUnvisited(f8),
          x44_viewportEase(Ease::None), x48_camEase(Ease::None), x4c_pointEase(Ease::None),
          x50_depth1Ease(Ease::None), x54_depth2Ease(Ease::None), x58_alphaEase(Ease::None) {}

        static void InterpolateWithClamp(const SAutoMapperRenderState& a,
                                         SAutoMapperRenderState& out,
                                         const SAutoMapperRenderState& b,
                                         float t);
        void ResetInterpolation()
        {
            x44_viewportEase = Ease::None;
            x48_camEase = Ease::None;
            x4c_pointEase = Ease::None;
            x50_depth1Ease = Ease::None;
            x54_depth2Ease = Ease::None;
            x58_alphaEase = Ease::None;
        }
    };

    struct SAutoMapperHintStep
    {
        enum class Type
        {
            PanToArea,
            PanToWorld,
            SwitchToUniverse,
            SwitchToWorld,
            ShowBeacon,
            ZoomIn,
            ZoomOut
        };
        struct PanToArea {};
        struct PanToWorld {};
        struct SwitchToUniverse {};
        struct SwitchToWorld {};
        struct ShowBeacon {};
        struct ZoomIn {};
        struct ZoomOut {};

        Type x0_type;
        union
        {
            CAssetId x4_worldId;
            TAreaId x4_areaId;
            float x4_float;
        };
        bool x8_processing = false;

        SAutoMapperHintStep(PanToArea, TAreaId areaId) : x0_type(Type::PanToArea), x4_areaId(areaId) {}
        SAutoMapperHintStep(PanToWorld, CAssetId worldId) : x0_type(Type::PanToWorld), x4_worldId(worldId) {}
        SAutoMapperHintStep(SwitchToUniverse) : x0_type(Type::SwitchToUniverse), x4_worldId(0) {}
        SAutoMapperHintStep(SwitchToWorld, CAssetId worldId) : x0_type(Type::SwitchToWorld), x4_worldId(worldId) {}
        SAutoMapperHintStep(ShowBeacon, float val) : x0_type(Type::ShowBeacon), x4_float(val) {}
        SAutoMapperHintStep(ZoomIn) : x0_type(Type::ZoomIn), x4_worldId(0) {}
        SAutoMapperHintStep(ZoomOut) : x0_type(Type::ZoomOut), x4_worldId(0) {}
    };

    struct SAutoMapperHintLocation
    {
        u32 x0_showBeacon;
        float x4_beaconAlpha;
        CAssetId x8_worldId;
        TAreaId xc_areaId;
    };

private:
    enum class EZoomState
    {
        None,
        In,
        Out
    };

    ELoadPhase x4_loadPhase = ELoadPhase::LoadResources;
    TLockedToken<CMapUniverse> x8_mapu;
    std::vector<std::unique_ptr<IWorld>> x14_dummyWorlds;
    const CWorld* x24_world;
    TLockedToken<CGuiFrame> x28_frmeMapScreen; // Used to be ptr
    bool m_frmeInitialized = false;
    TLockedToken<CModel> x30_miniMapSamus;
    TLockedToken<CTexture> x3c_hintBeacon;
    std::vector<CTexturedQuadFilter> m_hintBeaconFilters;
    rstl::reserved_vector<TLockedToken<CTexture>, 5> x48_mapIcons;
    CAssetId x74_areaHintDescId;
    TLockedToken<CStringTable> x78_areaHintDesc;
    u32 x84_ = 0;
    CAssetId x88_mapAreaStringId;
    TLockedToken<CStringTable> x8c_mapAreaString; // Used to be optional
    u32 x9c_worldIdx = 0;
    TAreaId xa0_curAreaId;
    TAreaId xa4_otherAreaId;
    SAutoMapperRenderState xa8_renderStates[3]; // xa8, x104, x160; current, next, prev
    EAutoMapperState x1bc_state = EAutoMapperState::MiniMap;
    EAutoMapperState x1c0_nextState = EAutoMapperState::MiniMap;
    float x1c4_interpDur = 0.f;
    float x1c8_interpTime = 0.f;
    CSfxHandle x1cc_panningSfx;
    CSfxHandle x1d0_rotatingSfx;
    CSfxHandle x1d4_zoomingSfx;
    float x1d8_flashTimer = 0.f;
    float x1dc_playerFlashPulse = 0.f;
    std::list<SAutoMapperHintStep> x1e0_hintSteps;
    std::list<SAutoMapperHintLocation> x1f8_hintLocations;
    rstl::reserved_vector<TLockedToken<CTexture>, 9> x210_lstick;
    rstl::reserved_vector<TLockedToken<CTexture>, 9> x25c_cstick;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2a8_ltrigger;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2bc_rtrigger;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2d0_abutton;
    u32 x2e4_lStickPos = 0;
    u32 x2e8_rStickPos = 0;
    u32 x2ec_lTriggerPos = 0;
    u32 x2f0_rTriggerPos = 0;
    u32 x2f4_aButtonPos = 0;
    CGuiTextPane* x2f8_textpane_areaname = nullptr;
    CGuiTextPane* x2fc_textpane_hint = nullptr;
    CGuiTextPane* x300_textpane_instructions = nullptr;
    CGuiTextPane* x304_textpane_instructions1 = nullptr;
    CGuiTextPane* x308_textpane_instructions2 = nullptr;
    CGuiWidget* x30c_basewidget_leftPane = nullptr;
    CGuiWidget* x310_basewidget_yButtonPane = nullptr;
    CGuiWidget* x314_basewidget_bottomPane = nullptr;
    float x318_leftPanePos = 0.f;
    float x31c_yButtonPanePos = 0.f;
    float x320_bottomPanePos = 0.f;
    EZoomState x324_zoomState = EZoomState::None;
    u32 x328_ = 0;
    bool x32c_loadingDummyWorld = false;

    template <class T>
    static void SetResLockState(T& list, bool lock)
    {
        for (auto& res : list)
            if (lock)
                res.Lock();
            else
                res.Unlock();
    }
    bool NotHintNavigating() const;
    bool CanLeaveMapScreenInternal(const CStateManager& mgr) const;
    void LeaveMapScreen(const CStateManager& mgr);
    void SetupMiniMapWorld(const CStateManager& mgr);
    bool HasCurrentMapUniverseWorld() const;
    bool CheckDummyWorldLoad(const CStateManager& mgr);
    void UpdateHintNavigation(float dt, const CStateManager& mgr);
    static zeus::CVector2i GetMiniMapViewportSize();
    static zeus::CVector2i GetMapScreenViewportSize();
    static float GetMapAreaMiniMapDrawDepth() { return 2.f; }
    float GetMapAreaMaxDrawDepth(const CStateManager&, TAreaId) const;
    static float GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager&);
    float GetDesiredMiniMapCameraDistance(const CStateManager&) const;
    static float GetBaseMapScreenCameraMoveSpeed();
    float GetClampedMapScreenCameraDistance(float);
    float GetFinalMapScreenCameraMoveSpeed() const;
    void ProcessMapRotateInput(const CFinalInput& input, const CStateManager& mgr);
    void ProcessMapZoomInput(const CFinalInput& input, const CStateManager& mgr);
    void ProcessMapPanInput(const CFinalInput& input, const CStateManager& mgr);
    void SetShouldPanningSoundBePlaying(bool);
    void SetShouldZoomingSoundBePlaying(bool);
    void SetShouldRotatingSoundBePlaying(bool);
    void TransformRenderStatesWorldToUniverse();
    void TransformRenderStatesUniverseToWorld();
    void TransformRenderStateWorldToUniverse(SAutoMapperRenderState&);
    void SetupHintNavigation();
    CAssetId GetAreaHintDescriptionString(CAssetId mreaId);

public:
    CAutoMapper(CStateManager& stateMgr);
    bool CheckLoadComplete();
    bool CanLeaveMapScreen(const CStateManager&) const;
    float GetMapRotationX() const;
    float GetMapRotationZ() const;
    u32 GetFocusAreaIndex() const;
    CAssetId GetCurrWorldAssetId() const;
    void SetCurWorldAssetId(CAssetId mlvlId);
    void MuteAllLoopedSounds();
    void UnmuteAllLoopedSounds();
    void ProcessControllerInput(const CFinalInput&, CStateManager&);
    bool IsInPlayerControlState() const;
    void Update(float dt, const CStateManager& mgr);
    void Draw(const CStateManager&, const zeus::CTransform&, float) const;
    bool IsInOrTransitioningToMapScreenState() const;
    float GetTimeIntoInterpolation() const;
    bool IsFullyInMapScreenState() const;
    void BeginMapperStateTransition(EAutoMapperState, const CStateManager&);
    void CompleteMapperStateTransition(const CStateManager&);
    void ResetInterpolationTimer(float);
    SAutoMapperRenderState BuildMiniMapWorldRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId) const;
    SAutoMapperRenderState BuildMapScreenWorldRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId, bool) const;
    SAutoMapperRenderState BuildMapScreenUniverseRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId) const;
    void LeaveMapScreenState();
    void ProcessMapScreenInput(const CFinalInput& input, const CStateManager& mgr);
    zeus::CQuaternion GetMiniMapCameraOrientation(const CStateManager&) const;
    zeus::CVector3f GetAreaPointOfInterest(const CStateManager&, TAreaId) const;
    TAreaId FindClosestVisibleArea(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&,
                                   const IWorld&, const CMapWorldInfo&) const;
    std::pair<int, int>
    FindClosestVisibleWorld(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&) const;

    EAutoMapperState GetNextState() const { return x1c0_nextState; }
    bool IsInMapperState(EAutoMapperState state) const
    {
        return state == x1bc_state && state == x1c0_nextState;
    }
    bool IsInMapperStateTransition() const
    {
        return x1c0_nextState != x1bc_state;
    }
    bool IsRenderStateInterpolating() const
    {
        return x1c8_interpTime < x1c4_interpDur;
    }
    bool IsStateTransitioning() const { return x1bc_state != x1c0_nextState; }
    bool IsFullyInMiniMapState() const { return IsInMapperState(EAutoMapperState::MiniMap); }
    bool IsFullyOutOfMiniMapState() const { return x1bc_state != EAutoMapperState::MiniMap &&
                                                   x1c0_nextState != EAutoMapperState::MiniMap; }
    void OnNewInGameGuiState(EInGameGuiState, const CStateManager&);
    float GetInterp() const
    {
        if (x1c4_interpDur > 0.f)
            return x1c8_interpTime / x1c4_interpDur;
        return 0.f;
    }
};
}

#endif // __URDE_CAUTOMAPPER_HPP__
