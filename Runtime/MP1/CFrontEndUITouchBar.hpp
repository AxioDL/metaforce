#pragma once

#include <memory>

namespace urde
{

class CFrontEndUITouchBar
{
public:
    enum class EPhase
    {
        None,
        PressStart,
        ProceedBack,
        StartOptions,
        EraseBack,
        FileSelect,
        NoCardSelect,
        FusionBonus
    };
    enum class EAction
    {
        None,
        Start,
        Normal,
        Hard,
        Back,
        Confirm,
        Options,
        FileA,
        FileB,
        FileC,
        Erase,
        FusionBonus,
        ImageGallery,
        NESMetroid,
        FusionSuit
    };
    enum class EFileState
    {
        New,
        Normal,
        Hard
    };
    struct SFileSelectDetail
    {
        EFileState state;
        int percent;
    };

protected:
    EPhase m_phase = EPhase::None;

public:
    virtual ~CFrontEndUITouchBar();
    virtual void SetPhase(EPhase ph);
    virtual EPhase GetPhase();
    virtual void SetFileSelectPhase(const SFileSelectDetail details[3],
                                    bool eraseGame, bool galleryActive);
    virtual void SetNoCardSelectPhase(bool galleryActive);
    virtual void SetFusionBonusPhase(bool fusionSuitActive);
    virtual void SetStartOptionsPhase(bool normalBeat);
    virtual EAction PopAction();
};

std::unique_ptr<CFrontEndUITouchBar> NewFrontEndUITouchBar();

}

