#ifndef __RETRO_CAUDIOSYS_HPP__
#define __RETRO_CAUDIOSYS_HPP__

#include "../GCNTypes.hpp"
#include "CVector3f.hpp"

namespace Retro
{

class CAudioSys
{
public:
    struct C3DEmitterParmData
    {
        Zeus::CVector3f pos;
        Zeus::CVector3f dir;
        float maxDist;
        float distComp;
        u32 flags;
        u16 sfxId;
        u8 maxVol;
        u8 minVol;
        u8 extra[2];
    };
    CAudioSys(u8,u8,u8,u8,u32)
    {
    }
};

}

#endif // __RETRO_CAUDIOSYS_HPP__
