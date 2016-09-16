#ifndef __URDE_MP1_CSLIDESHOW_HPP__
#define __URDE_MP1_CSLIDESHOW_HPP__

#include "RetroTypes.hpp"
#include "CIOWin.hpp"
#include "CToken.hpp"

namespace urde
{
class CTexture;

class CSlideShow : public CIOWin
{
    u32 x14_ = 0;
    u32 x1c_ = 0;
    u32 x20_ = 0;
    u32 x24_ = 0;
    u32 x2c_ = 0;
    u32 x30_ = 0;
    u32 x34_ = 0;
    u32 x38_ = 0;
    u32 x3c_ = 0;
    u32 x40_ = 0;
    u32 x44_ = 0;
    u32 x48_ = -1;
    float x4c_ = 0.f;
    float x50_ = 0.f;
    float x54_ = 0.f;
    float x58_ = 0.f;
    u32 x5c_ = 0;
    u32 x60_ = -1;
    u32 x64_ = 0;
    bool x68_ = false;
    u32 x6c_ = 0;
    bool x70_ = false;

    u32 xc4_ = 0;
    u32 xc8_ = 0;
    u32 xcc_ = 0;
    u32 xd4_ = 0;
    u32 xd8_ = 0;
    u32 xdc_ = 0;
    u32 xe0_ = 0;
    u32 xe4_ = 0;
    u32 xe8_ = 0;
    u32 xec_ = 0;
    u32 xf0_ = 0;
    u32 xf4_ = 0;
    u32 xfc_ = 0;
    u32 x100_ = 0;
    u32 x104_ = 0;
    u32 x10c_ = 0;
    u32 x110_ = 0;
    u32 x114_ = 0;
    u32 x11c_ = 0;
    u32 x120_ = 0;
    u32 x124_ = 0;
    float x128_ = 32.f;
    float x12c_ = 32.f;

    union
    {
        struct
        {
            bool x134_24_ : 1;
            bool x134_25_ : 1;
            bool x134_26_ : 1;
            bool x134_27_ : 1;
            bool x134_28_ : 1;
            bool x134_29_ : 1;
            bool x134_30_ : 1;
            bool x134_31_ : 1;
            bool x135_24_ : 1;
        };
        u32 dummy = 0;
    };
public:
    CSlideShow();
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&);
    bool GetIsContinueDraw() const {return false;}
    void Draw() const;
};

}

#endif // __URDE_MP1_CSLIDESHOW_HPP__
