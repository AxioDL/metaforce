#ifndef __PSHAG_CAUDIOSYS_HPP__
#define __PSHAG_CAUDIOSYS_HPP__

#include "../GCNTypes.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{

class CAudioSys
{
public:
    struct C3DEmitterParmData
    {
        zeus::CVector3f pos;
        zeus::CVector3f dir;
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

#endif // __PSHAG_CAUDIOSYS_HPP__
