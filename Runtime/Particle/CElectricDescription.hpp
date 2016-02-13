#ifndef __RETRO_CELECTRICDESCRIPTION_HPP__
#define __RETRO_CELECTRICDESCRIPTION_HPP__

#include "CParticleDataFactory.hpp"

namespace Retro
{
class CElectricDescription
{
    std::unique_ptr<CIntElement> x0_LIFE;
    std::unique_ptr<CIntElement> x4_SLIF;
    std::unique_ptr<CRealElement> x8_GRAT;
    std::unique_ptr<CIntElement> xc_SCNT;
    std::unique_ptr<CIntElement> x10_SSEG;
    std::unique_ptr<CColorElement> x14_COLR;
    bool x18_IEMT;
    bool x1c_FEMT;
    bool x20_AMPL;
    bool x24_AMPD;
    bool x28_LWD1;
    bool x2c_LWD2;
    bool x30_LWD3;
    bool x34_LCL1;
    bool x38_LCL2;
    bool x3c_LCL3;
    SSwooshGeneratorDesc x40_SSWH;
    SChildGeneratorDesc  x50_GPSM;
    SChildGeneratorDesc  x60_EPSM;
    bool x70_ZERY;
};
}

#endif // __RETRO_CELECTRICDESCRIPTION_HPP__
