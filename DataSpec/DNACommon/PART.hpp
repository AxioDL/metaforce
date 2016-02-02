#ifndef __COMMON_PART_HPP__
#define __COMMON_PART_HPP__

#include "ParticleCommon.hpp"

namespace Retro
{
namespace DNAParticle
{

template <class IDType>
struct GPSM : BigYAML
{
    static const char* DNAType() {return "Retro::GPSM";}
    const char* DNATypeV() const {return DNAType();}

    VectorElement x18_POFS;
    RealElement x20_LENG;
    RealElement x24_WIDT;
    IntElement x28_MAXP;
    ColorElement x30_COLR;
    IntElement x34_LTME;
    VectorElement x38_ILOC;
    VectorElement x3c_IVEC;
    EmitterElement x40_EMTR;
    atUint8 x44_settings; // SORT,MBLR,LINE
    atUint8 x45_options; // VMD1-4, OPTS
    RealElement x4c_SIZE;
    RealElement x50_ROTA;
    TextureElement x54_TEXR;
    TextureElement x58_TIND;
    IntElement xb4_NDSY;
    VectorElement x110_LDIR;
    ColorElement x104_LCLR;
    RealElement x108_LINT;
    VectorElement x10c_LOFF;
    IntElement x114_LFOT;
    RealElement x118_LFOR;
    RealElement x11c_LSLA;
    ChildGeneratorDesc<IDType> xa4_IDTS;

    void read(Athena::io::YAMLDocReader& r)
    {
        if (r.enterSubRecord("LFOR"))
        {
            x118_LFOR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IDTS"))
        {
            xa4_IDTS.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("EMTR"))
        {
            x40_EMTR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            x30_COLR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("CIND"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("AAPH"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("CSSD"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("GRTE"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("FXLL"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ICTS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("KSSM"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ILOC"))
        {
            x38_ILOC.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IITS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IVEC"))
        {
            x3c_IVEC.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LDIR"))
        {
            x110_LDIR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LCLR"))
        {
            x104_LCLR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LENG"))
        {
            x20_LENG.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MAXP"))
        {
            x28_MAXP.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LOFF"))
        {
            x10c_LOFF.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LINT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LINE"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LFOT"))
        {
            x114_LFOT.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LIT_"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTME"))
        {
            x34_LTME.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LSLA"))
        {
            x11c_LSLA.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTYP"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("NDSY"))
        {
            xb4_NDSY.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MBSP"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MBLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("NCSY"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PISY"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("OPTS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMAB"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SESD"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSLT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMSC"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOP"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMDL"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOO"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMRT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("POFS"))
        {
            x18_POFS.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMUS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSIV"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ROTA"))
        {
            x50_ROTA.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSVM"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSTS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSOV"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PSWT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMLC"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMED"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("PMOO"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL1"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSSD"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SORT"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SIZE"))
        {
            x4c_SIZE.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SISY"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSPO"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("TEXR"))
        {
            x54_TEXR.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSWH"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("TIND"))
        {
            x58_TIND.read(r);
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD4"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD3"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD2"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD1"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL4"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL3"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL2"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VEL1"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ZBUF"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("WIDT"))
        {
            x24_WIDT.read(r);
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {

    }
    size_t binarySize(size_t __isz) const
    {

    }
    void read(Athena::io::IStreamReader& r)
    {

    }
    void write(Athena::io::IStreamWriter& w) const
    {

    }
};

}
}

#endif // __COMMON_PART_HPP__
