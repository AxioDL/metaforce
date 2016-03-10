#ifndef __CSPLASHSCREEN_HPP__
#define __CSPLASHSCREEN_HPP__

#include "CIOWin.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;
namespace MP1
{
class CSplashScreen : public CIOWin
{
public:
    enum ESplashScreen
    {
        NintendoLogo,
        RetroLogo,
        DolbyLogo
    };
private:
    ESplashScreen x14_splashScreen;
    float x18_ = 2.0;
    float x1c_ = 0.0;
    u32 x20_phase = 0;
    bool x24_progressiveEnabled = true;
    bool x25_ = false;
    TLockedToken<CTexture> x28_texture;

public:
    CSplashScreen(ESplashScreen splash);
    EMessageReturn OnMessage(const CArchitectureMessage &, CArchitectureQueue &);
    void Draw() const;
};
}

#endif // __CSPLASHSCREEN_HPP__
