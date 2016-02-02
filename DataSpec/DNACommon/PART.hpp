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

    RealElement x118_LFOR;
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

            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
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
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IITS"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("IVEC"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LDIR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("COLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LCLR"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LENG"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("MAXP"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LOFF"))
        {
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
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LIT_"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTME"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LSLA"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("LTYP"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("NDSY"))
        {
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
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("SSWH"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("TIND"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("VMD4"))
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
        if (r.enterSubRecord("VMD3"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("ZBUF"))
        {
            r.leaveSubRecord();
        }
        if (r.enterSubRecord("WIDT"))
        {
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
