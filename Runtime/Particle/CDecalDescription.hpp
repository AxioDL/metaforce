#ifndef __PSHAG_CDECALDESCRIPTION_HPP__
#define __PSHAG_CDECALDESCRIPTION_HPP__

#include "CRealElement.hpp"
#include "CIntElement.hpp"
#include "CVectorElement.hpp"
#include "CColorElement.hpp"
#include "CUVElement.hpp"
#include "CParticleDataFactory.hpp"

namespace urde
{
struct SQuadDescr
{
    std::unique_ptr<CIntElement> x0_LFT;
    std::unique_ptr<CRealElement> x4_SZE;
    std::unique_ptr<CRealElement> x8_ROT;
    std::unique_ptr<CVectorElement> xc_OFF;
    std::unique_ptr<CColorElement> x10_CLR;
    std::unique_ptr<CUVElement> x14_TEX;
    bool x18_ADD = false;
};

class CDecalDescription
{
public:
    SQuadDescr x0_Quad;
    SQuadDescr x1c_Quad;
    SParticleModel x38_DMDL;
    std::unique_ptr<CIntElement> x48_DFLT;
    std::unique_ptr<CVectorElement> x4c_DMOP;
    std::unique_ptr<CVectorElement> x50_DMRT;
    std::unique_ptr<CVectorElement> x54_DMSC;
    std::unique_ptr<CColorElement>  x58_DMCL;
    union
    {
        struct { bool x5c_24_DMAB : 1; bool x5c_25_DMOO : 1;};
        u8 dummy = 0;
    };
};

}
#endif // __PSHAG_CDECALDESCRIPTION_HPP__
