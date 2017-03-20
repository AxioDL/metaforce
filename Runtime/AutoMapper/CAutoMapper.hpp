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
        Zero
    };
    struct SAutoMapperRenderState
    {
        zeus::CVector2i x0_viewportSize;
        zeus::CQuaternion x8_camOrientation;
        float x18_f1;
        float x1c_f2;
        zeus::CVector3f x20_areaPoint;
        float x2c_drawDepth1;
        float x30_drawDepth2;
        float x34_alphaSurfaceVisited;
        float x38_alphaOutlineVisited;
        float x3c_alphaSurfaceUnvisited;
        float x40_alphaOutlineUnvisited;
        u32 x44_;
        u32 x48_;
        u32 x4c_;
        u32 x50_;
        u32 x54_;
        u32 x58_;
        SAutoMapperRenderState() = default;
        SAutoMapperRenderState(const zeus::CVector2i& v1, const zeus::CQuaternion& rot,
                               float f1, float f2, const zeus::CVector3f& v2, float f3, float f4,
                               float f5, float f6, float f7, float f8)
        : x0_viewportSize(v1), x8_camOrientation(rot), x18_f1(f1), x1c_f2(f2),
          x20_areaPoint(v2), x2c_drawDepth1(f3), x30_drawDepth2(f4),
          x34_alphaSurfaceVisited(f5), x38_alphaOutlineVisited(f6),
          x3c_alphaSurfaceUnvisited(f7), x40_alphaOutlineUnvisited(f8),
          x44_(0), x48_(0), x4c_(0), x50_(0), x54_(0), x58_(0) {}
    };

    class CAudioMenu
    {
    public:
        enum class EMenu
        {
        };
        struct SMenuOption
        {
            SMenuOption(const std::string&, s32, s32, s32);
            void BuildDrawableString();
        };

    private:
    public:
        CAudioMenu()=default;
        void SetIsHighlighted(bool);
        void AddOption(const SMenuOption&);
        void ProcessControllerInput(const CFinalInput&);
        u32 GetSelectionIndex() const;
        const SMenuOption& GetOption(s32) const;
        SMenuOption& Option(s32);
        void SetSelectionIndex(s32);
        void Draw(EMenu) const;
    };

private:
    ELoadPhase x4_loadPhase = ELoadPhase::LoadResources;
    TLockedToken<CMapUniverse> x8_mapu;
    std::vector<std::unique_ptr<IWorld>> x14_dummyWorlds;
    CWorld& x24_world;
    u32 x28_ = 0;
    u32 x2c_ = 0;
    TLockedToken<CModel> x30_miniMapSamus;
    TLockedToken<CTexture> x3c_hintBeacon;
    rstl::reserved_vector<TLockedToken<CTexture>, 5> x48_mapIcons;
    ResId x74_ = -1;
    u32 x84_ = 0;
    ResId x88_ = -1;
    u32 x98_ = 0;
    u32 x9c_worldIdx = 0;
    TAreaId xa0_curAreaId;
    TAreaId xa4_otherAreaId;
    SAutoMapperRenderState xa8_[3]; // xa8, x104, x160
    EAutoMapperState x1bc_state = EAutoMapperState::Zero;
    EAutoMapperState x1c0_nextState = EAutoMapperState::Zero;
    float x1c4_ = 0.f;
    float x1c8_ = 0.f;
    u32 x1cc_ = 0;
    u32 x1d0_ = 0;
    u32 x1d4_ = 0;
    float x1d8_ = 0.f;
    float x1dc_ = 0.f;
    std::list<u32> x1e0_;
    u32 x1f4_ = 0;
    std::list<u32> x1f8_;
    u32 x20c_ = 0;
    rstl::reserved_vector<TLockedToken<CTexture>, 9> x210_lstick;
    rstl::reserved_vector<TLockedToken<CTexture>, 9> x25c_cstick;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2a8_ltrigger;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2bc_rtrigger;
    rstl::reserved_vector<TLockedToken<CTexture>, 2> x2d0_abutton;
    u32 x2e4_ = 0;
    u32 x2e8_ = 0;
    u32 x2ec_ = 0;
    u32 x2f0_ = 0;
    u32 x2f4_ = 0;
    u32 x2f8_ = 0;
    u32 x2fc_ = 0;
    u32 x300_ = 0;
    u32 x304_ = 0;
    u32 x308_ = 0;
    u32 x30c_ = 0;
    u32 x310_ = 0;
    u32 x314_ = 0;
    float x318_ = 0.f;
    float x31c_ = 0.f;
    float x320_ = 0.f;
    u32 x324_ = 0;
    u32 x328_ = 0;
    u32 x32c_ = 0;

public:
    CAutoMapper(CStateManager& stateMgr);
    bool CheckLoadComplete();
    bool CanLeaveMapScrean(const CStateManager&) const;
    float GetMapRotationX() const;
    float GetMapRotationZ() const;
    u32 GetFocusAreaIndex() const;
    ResId GetCurrWorldAssetId() const;
    void SetCurWorldAssetId(ResId mlvlId);
    void MuteAllLoopedSounds();
    void UnmuteAllLoopedSounds();
    void ProcessControllerInput(const CFinalInput&, CStateManager&);
    bool IsInPlayerControlState() const;
    void Draw(const CStateManager&, const zeus::CTransform&, float) const;
    bool IsInOrTransitioningToMapScreenState() const;
    float GetTimeIntoInterpolation() const;
    bool IsFullyInMapScreenState() const;
    void BeginMapperStateTransition(EAutoMapperState, const CStateManager&);
    void CompleteMapperStateTransition();
    void ResetInterpolationTimer(float);
    SAutoMapperRenderState BuildMiniMapWorldRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId) const;
    SAutoMapperRenderState BuildMapScreenWorldRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId) const;
    SAutoMapperRenderState BuildMapScreenUniverseRenderState(const CStateManager&, const zeus::CQuaternion&, TAreaId) const;
    void SetShouldPanningSoundBePlaying(bool);
    void SetShouldZoomingSoundBePlaying(bool);
    void SetShouldRotatingSoundBePlaying(bool);
    void LeaveMapScreenState();
    zeus::CQuaternion GetMiniMapCameraOrientation(const CStateManager&) const;
    zeus::CVector3f GetAreaPointOfInterest(const CStateManager&, TAreaId) const;
    void FindClosestVisibleArea(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&,
                           const IWorld&, const CMapWorldInfo&) const;
    void FindClosestVisibleWorld(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&) const;

    static zeus::CVector2i GetMiniMapViewportSize();
    static void GetMapScreenViewportSize();
    static float GetMapAreaMiniMapDrawDepth() { return 2.f; }
    static float GetMapAreaMaxDrawDepth();
    static float GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager&);
    static float GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager&);
    static void GetClampedMapScreenCameraDistance(float);
    static void GetDesiredMiniMapCameraDistance(const CStateManager&);
    static float GetBaseCameraMoveSpeed();
    static float GetFinalCameraMoveSpeed();

    bool IsInMapperState(EAutoMapperState state) const
    {
        return state == x1bc_state && state == x1c0_nextState;
    }
    bool IsInMapperTransition() const;
    bool IsRenderStateInterpolating() const;
    void TransformRenderStatesWorldToUniverse();
    void TransformRenderStatesUniverseToWorld();
    void TransformRenderStateWorldToUniverse(SAutoMapperRenderState&);
    void UpdateOptionsMenu(const CTweakValue::Audio&);
    void UpdateAudioMusicMenu();
    void UpdateAudioEvents();
    void UpdateAudioEventMenu();
    void GetCurrentAudioInfo() const;
    void PresentAudioMenuInput(const CFinalInput&);
    void SetFocusAudioMenu(CAudioMenu::EMenu);
    bool IsStateTransitioning() const;
    bool IsFullyInMiniMapState() const;
    static bool IsDrawState(EAutoMapperState);

    void OnNewInGameGuiState(EInGameGuiState, const CStateManager&);
    void OnChangeAudioMusicSelection();

};
}

#endif // __URDE_CAUTOMAPPER_HPP__
