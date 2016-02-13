#ifndef __RETRO_CGENDESCRIPTION_HPP__
#define __RETRO_CGENDESCRIPTION_HPP__

#include <memory>
#include "CRealElement.hpp"
#include "CIntElement.hpp"
#include "CVectorElement.hpp"
#include "CModVectorElement.hpp"
#include "CColorElement.hpp"
#include "CUVElement.hpp"
#include "CEmitterElement.hpp"
#include "CSpawnSystemKeyframeData.hpp"
#include "CParticleDataFactory.hpp"

namespace Retro
{

class CGenDescription
{
public:
    std::unique_ptr<CVectorElement> x0_PSIV;
    std::unique_ptr<CModVectorElement> x4_PSVM;
    std::unique_ptr<CVectorElement> x8_PSOV;
    std::unique_ptr<CIntElement> xc_PSLT;
    std::unique_ptr<CIntElement> x10_PSWT;
    std::unique_ptr<CRealElement> x14_PSTS;
    std::unique_ptr<CVectorElement> x18_POFS;
    std::unique_ptr<CIntElement> x1c_SEED;
    std::unique_ptr<CRealElement> x20_LENG;
    std::unique_ptr<CRealElement> x24_WIDT;
    std::unique_ptr<CIntElement> x28_MAXP;
    std::unique_ptr<CRealElement> x2c_GRTE;
    std::unique_ptr<CColorElement> x30_COLR;
    std::unique_ptr<CIntElement> x34_LTME;
    std::unique_ptr<CVectorElement> x38_ILOC;
    std::unique_ptr<CVectorElement> x3c_IVEC;
    std::unique_ptr<CEmitterElement> x40_EMTR;
    union
    {
        struct
        {
            bool x44_28_SORT : 1; bool x44_30_MBLR : 1;  bool x44_24_LINE : 1; bool x44_29_LIT_ : 1;
            bool x44_26_AAPH : 1; bool x44_27_ZBUF : 1;  bool x44_25_FXLL : 1; bool x44_31_PMAB : 1;
            bool x45_29_VMD4 : 1; bool x45_28_VMD3 : 1;  bool x45_27_VMD2 : 1; bool x45_26_VMD1 : 1;
            bool x45_31_OPTS : 1; bool x45_24_PMUS : 1;  bool x45_25_PMOO : 1; bool x45_30_CIND : 1;
        };
        uint16_t dummy1 = 0;
    };
    std::unique_ptr<CIntElement> x48_MBSP;
    std::unique_ptr<CRealElement> x4c_SIZE;
    std::unique_ptr<CRealElement> x50_ROTA;
    std::unique_ptr<CUVElement> x54_TEXR;
    std::unique_ptr<CUVElement> x58_TIND;
    SParticleModel x5c_PMDL;
    std::unique_ptr<CVectorElement> x6c_PMOP;
    std::unique_ptr<CVectorElement> x70_PMRT;
    std::unique_ptr<CVectorElement> x74_PMSC;
    std::unique_ptr<CColorElement> x78_PMCL;
    std::unique_ptr<CModVectorElement> x7c_VEL1;
    std::unique_ptr<CModVectorElement> x80_VEL2;
    std::unique_ptr<CModVectorElement> x84_VEL3;
    std::unique_ptr<CModVectorElement> x88_VEL4;
    SChildGeneratorDesc x8c_ICTS;
    std::unique_ptr<CIntElement> x9c_NCSY;
    std::unique_ptr<CIntElement> xa0_CSSD;
    SChildGeneratorDesc xa4_IDTS;
    std::unique_ptr<CIntElement> xb4_NDSY;
    SChildGeneratorDesc xb8_IITS;
    std::unique_ptr<CIntElement> xc8_PISY;
    std::unique_ptr<CIntElement> xcc_SISY;
    std::unique_ptr<CSpawnSystemKeyframeData> xd0_KSSM;
    SSwooshGeneratorDesc xd4_SSWH;
    std::unique_ptr<CIntElement> xe4_SSSD;
    std::unique_ptr<CVectorElement> xe8_SSPO;
    SElectricGeneratorDesc xec_SELC;
    std::unique_ptr<CIntElement> xf8_SESD;
    std::unique_ptr<CVectorElement> xfc_SEPO;
    SChildGeneratorDesc xec_PMLC;
    std::unique_ptr<CIntElement> x100_LTYP;
    std::unique_ptr<CColorElement> x104_LCLR;
    std::unique_ptr<CRealElement> x108_LINT;
    std::unique_ptr<CVectorElement> x10c_LOFF;
    std::unique_ptr<CVectorElement> x110_LDIR;
    std::unique_ptr<CIntElement> x114_LFOT;
    std::unique_ptr<CRealElement> x118_LFOR;
    std::unique_ptr<CRealElement> x11c_LSLA;

    /* 0-00 additions */
    union
    {
        struct
        {
            bool x30_30_ORNT : 1;
            bool x30_31_RSOP : 1;
        };
        uint16_t dummy2 = 0;
    };
    std::unique_ptr<CRealElement> x10c_ADV1;
    std::unique_ptr<CRealElement> x110_ADV2;
    std::unique_ptr<CRealElement> x114_ADV3;
    std::unique_ptr<CRealElement> x118_ADV4;
    std::unique_ptr<CRealElement> x11c_ADV5;
    std::unique_ptr<CRealElement> x120_ADV6;
    std::unique_ptr<CRealElement> x124_ADV7;
    std::unique_ptr<CRealElement> x128_ADV8;

    CGenDescription()
    {
        x45_25_PMOO = true;
    }
};

}

#endif // __RETRO_CGENDESCRIPTION_HPP__
