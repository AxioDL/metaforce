#pragma once

#include "zeus/CColor.hpp"
#include "zeus/CVector3f.hpp"
#include "RetroTypes.hpp"
#include "CParticleGen.hpp"

namespace urde
{

class CWarp
{
public:
    virtual ~CWarp() = default;
    virtual bool UpdateWarp()=0;
    virtual void ModifyParticles(std::vector<CParticle>& particles)=0;
    virtual void Activate(bool)=0;
    virtual bool IsActivated()=0;
    virtual FourCC Get4CharID()=0;
};

}

