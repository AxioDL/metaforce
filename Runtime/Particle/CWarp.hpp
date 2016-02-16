#ifndef __PSHAG_CWARP_HPP__
#define __PSHAG_CWARP_HPP__

#include <CColor.hpp>
#include <CVector3f.hpp>
#include "RetroTypes.hpp"

namespace pshag
{

class CWarp
{
public:
    virtual ~CWarp() {}
    virtual bool UpdateWarp()=0;
    virtual void ModifyParticles(int, int, int*,
                                 Zeus::CVector3f*,
                                 Zeus::CVector3f*,
                                 Zeus::CVector3f*,
                                 Zeus::CColor*,
                                 float*, float*)=0;
    virtual void Activate(bool)=0;
    virtual bool IsActivated()=0;
    virtual FourCC Get4CharID()=0;
};

}

#endif // __PSHAG_CWARP_HPP__
