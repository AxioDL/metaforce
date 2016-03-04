#ifndef __PSHAG_CWARP_HPP__
#define __PSHAG_CWARP_HPP__

#include "zeus/CColor.hpp"
#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"

namespace urde
{

class CWarp
{
public:
    virtual ~CWarp() {}
    virtual bool UpdateWarp()=0;
    virtual void ModifyParticles(int, int, int*,
                                 zeus::CVector3f*,
                                 zeus::CVector3f*,
                                 zeus::CVector3f*,
                                 zeus::CColor*,
                                 float*, float*)=0;
    virtual void Activate(bool)=0;
    virtual bool IsActivated()=0;
    virtual FourCC Get4CharID()=0;
};

}

#endif // __PSHAG_CWARP_HPP__
