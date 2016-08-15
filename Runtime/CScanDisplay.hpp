#ifndef __URDE_CSCANDISPLAY_HPP__
#define __URDE_CSCANDISPLAY_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "zeus/CVector2f.hpp"
#include "zeus/CQuaternion.hpp"
#include "CScannableObjectInfo.hpp"
#include "Graphics/CTexture.hpp"

namespace urde
{
class CGuiTextPane;
class CGuiWidget;
class CScanDisplay
{
public:
    class CDataDot
    {
    public:
        enum class EDotState
        {
            Zero,
            One
        };
    private:
        EDotState x0_ = EDotState::Zero;
        zeus::CVector2f x4_;
        zeus::CVector2f xc_;
        zeus::CVector2f x14_;
        float x1c_ = 0.f;
        float x20_ = 0.f;
        float x24_ = 0.f;
        float x28_ = 0.f;
    public:
        CDataDot() = default;
        void Update(float);
        void Draw(const zeus::CColor&, float) const;
        float GetTransitionFactor() const;
        void StartTransitionTo(const zeus::CVector2f&, float);
        void SetDestPosition(const zeus::CVector2f&);
        void SetDesiredAlpha(float);
        void SetDotState(EDotState);
        void SetAlpha(float);
        zeus::CVector2f GetCurrPosition() const;
        EDotState GetDotState() const;
    };

    struct SBucketBackground
    {
    };

public:
    CScanDisplay() = default;
    void StartScan(TUniqueId, const CScannableObjectInfo&, CGuiTextPane*, CGuiWidget*, float);
    void StopScan();
    void InitializeFrame(float);
    void Update(float, float);
    void SetBackgroundBucketOccluded(s32, float);
    bool PanelCoversBucketBackground(CScannableObjectInfo::EPanelType, s32);
    void Draw() const;
    void SetBaseOrientation(const zeus::CQuaternion&);
    void SetDebugCameraOptions(float, float, float);
    void SetPanelTranslationX(float);
    void GetDownloadStartTime(s32) const;
    void GetDownloadFraction(s32, float) const;
    void GetScanState() const;
    void ScanTarget() const;
};
}
#endif // __URDE_CSCANDISPLAY_HPP__
