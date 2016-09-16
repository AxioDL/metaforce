#ifndef __URDE_CAUDIOSYS_HPP__
#define __URDE_CAUDIOSYS_HPP__

#include "../GCNTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "amuse/amuse.hpp"

namespace urde
{

class CAudioSys
{
    amuse::Engine m_engine;
public:
    struct C3DEmitterParmData
    {
        zeus::CVector3f x0_pos;
        zeus::CVector3f xc_dir;
        float x18_maxDist;
        float x1c_distComp;
        u32 x20_flags;
        u16 x24_sfxId;
        u8 x26_maxVol;
        u8 x27_minVol;
        u8 x28_extra[2];
    };
    CAudioSys(amuse::IBackendVoiceAllocator& backend, u8,u8,u8,u8,u32)
    : m_engine(backend)
    {
    }
};

}

#endif // __URDE_CAUDIOSYS_HPP__
