#ifndef __URDE_CFRONTENDUITOUCHBAR_HPP__
#define __URDE_CFRONTENDUITOUCHBAR_HPP__

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
        BackConfirm,
        FileSelect
    };
    enum class EAction
    {
        None,
        Start,
        Back,
        Confirm,
        FileA,
        FileB,
        FileC,
        FusionBonus,
        ImageGallery
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

    virtual ~CFrontEndUITouchBar();
    virtual void SetPhase(EPhase ph);
    virtual void SetFileSelectPhase(const SFileSelectDetail details[3],
                                    bool eraseGame, bool galleryActive);
    virtual EAction PopAction();
};

std::unique_ptr<CFrontEndUITouchBar> NewFrontEndUITouchBar();

}

#endif // __URDE_CFRONTENDUITOUCHBAR_HPP__
