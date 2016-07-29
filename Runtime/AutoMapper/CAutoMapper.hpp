#ifndef __URDE_CAUTOMAPPER_HPP__
#define __URDE_CAUTOMAPPER_HPP__

#include "RetroTypes.hpp"
#include "CInGameTweakManagerBase.hpp"
#include "zeus/CQuaternion.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CFinalInput;
class IWorld;
class CMapWorldInfo;
class CStateManager;
enum class EInGameGuiState;
class CAutoMapper
{
public:
    enum class EAutoMapperState
    {
    };
    struct SAutoMapperRenderState
    {
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
public:
    CAutoMapper(CStateManager&);
    bool CheckLoadComplete();
    bool CanLeaveMapScrean(const CStateManager&) const;
    float GetMapRotationX() const;
    float GetMapRotationZ() const;
    u32 GetFocusAreaIndex() const;
    ResId GetCurrWorldAssetId() const;
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
    void BuildMiniMapWorldRenderState(const CStateManager&, const zeus::CQuaternion&, s32) const;
    void BuildMapScreenWorldRenderState(const CStateManager&, const zeus::CQuaternion&, s32) const;
    void BuildMapScreenUniverseRenderState(const CStateManager&, const zeus::CQuaternion&, s32) const;
    void SetShouldPanningSoundBePlaying(bool);
    void SetShouldZoomingSoundBePlaying(bool);
    void SetShouldRotatingSoundBePlaying(bool);
    void LeaveMapScreenState();
    void GetMiniMapCameraOrientation(CStateManager&);
    void GetAreaPointOfInterest(CStateManager, s32);
    void FindClosestVisibleArea(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&,
                           const IWorld&, const CMapWorldInfo&) const;
    void FindClosestVisibleWorld(const zeus::CVector3f&, const zeus::CUnitVector3f&, const CStateManager&) const;

    void GetMiniMapViewportSize();
    void GetMapScreenViewportSize();
    float GetMapAreaMiniDrawDepth();
    float GetMapAreaMaxDrawDepth();
    void GetMapAreaMiniMapDrawAlphaSurfaceVisited(const CStateManager&);
    void GetMapAreaMiniMapDrawAlphaOutlineVisited(const CStateManager&);
    void GetMapAreaMiniMapDrawAlphaSurfaceUnvisited(const CStateManager&);
    void GetMapAreaMiniMapDrawAlphaOutlineUnvisited(const CStateManager&);
    void GetClampedMapScreenCameraDistance(float) const;
    void GetDesiredMiniMapCameraDistance(const CStateManager&) const;
    float GetBaseCameraMoveSpeed() const;
    float GetFinalCameraMoveSpeed() const;
    bool IsInMapperState(EAutoMapperState) const;
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
