#ifndef __PSHAG_CTEXTURE_HPP__
#define __PSHAG_CTEXTURE_HPP__

#include "GCNTypes.hpp"

namespace pshag
{

class CTexture
{
    u16 x4_w;
    u16 x6_h;
public:
    enum class EClampMode
    {
        None,
        One
    };
    u16 GetWidth() const {return x4_w;}
    u16 GetHeight() const {return x6_h;}
    void Load(int slot, EClampMode clamp);
};

}

#endif // __PSHAG_CTEXTURE_HPP__
