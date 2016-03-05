#ifndef __PSHAG_CELECTRICDESCRIPTION_HPP__
#define __PSHAG_CELECTRICDESCRIPTION_HPP__

#include "CParticleDataFactory.hpp"
#include "CRealElement.hpp"
#include "CIntElement.hpp"
#include "CVectorElement.hpp"
#include "CModVectorElement.hpp"
#include "CColorElement.hpp"
#include "CUVElement.hpp"

namespace urde
{
class CElectricDescription
{
public:
    std::unique_ptr<CIntElement> x0_LIFE;
    std::unique_ptr<CIntElement> x4_SLIF;
    std::unique_ptr<CRealElement> x8_GRAT;
    std::unique_ptr<CIntElement> xc_SCNT;
    std::unique_ptr<CIntElement> x10_SSEG;
    std::unique_ptr<CColorElement> x14_COLR;
    std::unique_ptr<CEmitterElement> x18_IEMT;
    std::unique_ptr<CEmitterElement> x1c_FEMT;
    std::unique_ptr<CRealElement> x20_AMPL;
    std::unique_ptr<CRealElement> x24_AMPD;
    std::unique_ptr<CRealElement> x28_LWD1;
    std::unique_ptr<CRealElement> x2c_LWD2;
    std::unique_ptr<CRealElement> x30_LWD3;
    std::unique_ptr<CColorElement> x34_LCL1;
    std::unique_ptr<CColorElement> x38_LCL2;
    std::unique_ptr<CColorElement> x3c_LCL3;
    SSwooshGeneratorDesc x40_SSWH;
    SChildGeneratorDesc  x50_GPSM;
    SChildGeneratorDesc  x60_EPSM;
    bool x70_ZERY;
};
}

#endif // __PSHAG_CELECTRICDESCRIPTION_HPP__
