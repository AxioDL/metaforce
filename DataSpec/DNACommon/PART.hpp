#ifndef __COMMON_PART_HPP__
#define __COMMON_PART_HPP__

#include "ParticleCommon.hpp"
#include "PAK.hpp"

namespace Retro
{
namespace DNAParticle
{

template <class IDType>
struct GPSM : BigYAML
{
    static const char* DNAType() {return "Retro::GPSM";}
    const char* DNATypeV() const {return DNAType();}

    VectorElementFactory x0_PSIV;
    ModVectorElementFactory x4_PSVM;
    VectorElementFactory x8_PSOV;
    IntElementFactory xc_PSLT;
    IntElementFactory x10_PSWT;
    RealElementFactory x14_PSTS;
    VectorElementFactory x18_POFS;
    IntElementFactory x1c_PMED;
    RealElementFactory x20_LENG;
    RealElementFactory x24_WIDT;
    IntElementFactory x28_MAXP;
    RealElementFactory x2c_GRTE;
    ColorElementFactory x30_COLR;
    IntElementFactory x34_LTME;
    VectorElementFactory x38_ILOC;
    VectorElementFactory x3c_IVEC;
    EmitterElementFactory x40_EMTR;
    bool x44_0_SORT : 1; bool x44_1_MBLR : 1;  bool x44_2_LINE : 1; bool x44_3_LIT_ : 1;
    bool x44_4_AAPH : 1; bool x44_5_ZBUF : 1;  bool x44_6_FXLL : 1; bool x44_7_PMAB : 1;
    bool x45_0_VMD4 : 1; bool x45_1_VMD3 : 1;  bool x45_2_VMD2 : 1; bool x45_3_VMD1 : 1;
    bool x45_4_OPTS : 1; bool x45_5_PMUS : 1;  bool x45_6_PMOO : 1; bool x45_7_CIND : 1;
    IntElementFactory x48_MBSP;
    RealElementFactory x4c_SIZE;
    RealElementFactory x50_ROTA;
    UVElementFactory<IDType> x54_TEXR;
    UVElementFactory<IDType> x58_TIND;
    ChildResourceFactory<IDType> x5c_PMDL;
    VectorElementFactory x6c_PMOP;
    VectorElementFactory x70_PMRT;
    VectorElementFactory x74_PMSC;
    ColorElementFactory x78_PMCL;
    ModVectorElementFactory x7c_VEL1;
    ModVectorElementFactory x80_VEL2;
    ModVectorElementFactory x84_VEL3;
    ModVectorElementFactory x88_VEL4;
    ChildResourceFactory<IDType> x8c_ICTS;
    IntElementFactory x9c_NCSY;
    IntElementFactory xb4_NDSY;
    ChildResourceFactory<IDType> xb8_IITS;
    IntElementFactory xa0_CSSD;
    ChildResourceFactory<IDType> xa4_IDTS;
    IntElementFactory xc8_PISY;
    IntElementFactory xcc_SISY;
    SpawnSystemKeyframeData<IDType> xd0_KSSM;
    ChildResourceFactory<IDType> xd4_SSWH;
    IntElementFactory xe4_SSSD;
    VectorElementFactory xe8_SSPO;
    IntElementFactory xf8_SESD;
    VectorElementFactory xfc_SEPO;
    ChildResourceFactory<IDType> xec_PMLC;
    IntElementFactory x100_LTYP;
    ColorElementFactory x104_LCLR;
    RealElementFactory x108_LINT;
    VectorElementFactory x10c_LOFF;
    VectorElementFactory x110_LDIR;
    IntElementFactory x114_LFOT;
    RealElementFactory x118_LFOR;
    RealElementFactory x11c_LSLA;

    /* 0-00 additions */
    bool x30_30_ORNT : 1;
    bool xUNK_RSOP : 1;

    void read(Athena::io::YAMLDocReader& r)
    {
        for (const auto& elem : r.getCurNode()->m_mapChildren)
        {
            if (elem.first.size() < 4)
            {
                LogModule.report(LogVisor::Warning, "short FourCC in element '%s'", elem.first.c_str());
                continue;
            }

            r.enterSubRecord(elem.first.c_str());
            switch (*reinterpret_cast<const uint32_t*>(elem.first.data()))
            {
            case SBIG('PMCL'):
                x78_PMCL.read(r);
                break;
            case SBIG('LFOR'):
                x118_LFOR.read(r);
                break;
            case SBIG('IDTS'):
                xa4_IDTS.read(r);
                break;
            case SBIG('EMTR'):
                x40_EMTR.read(r);
                break;
            case SBIG('COLR'):
                x30_COLR.read(r);
                break;
            case SBIG('CIND'):
                x45_7_CIND = r.readBool(nullptr);
                break;
            case SBIG('AAPH'):
                x44_4_AAPH = r.readBool(nullptr);
                break;
            case SBIG('CSSD'):
                xa0_CSSD.read(r);
                break;
            case SBIG('GRTE'):
                x2c_GRTE.read(r);
                break;
            case SBIG('FXLL'):
                x44_6_FXLL = r.readBool(nullptr);
                break;
            case SBIG('ICTS'):
                x8c_ICTS.read(r);
                break;
            case SBIG('KSSM'):
                xd0_KSSM.read(r);
                break;
            case SBIG('ILOC'):
                x38_ILOC.read(r);
                break;
            case SBIG('IITS'):
                break;
            case SBIG('IVEC'):
                x3c_IVEC.read(r);
                break;
            case SBIG('LDIR'):
                x110_LDIR.read(r);
                break;
            case SBIG('LCLR'):
                x104_LCLR.read(r);
                break;
            case SBIG('LENG'):
                x20_LENG.read(r);
                break;
            case SBIG('MAXP'):
                x28_MAXP.read(r);
                break;
            case SBIG('LOFF'):
                x10c_LOFF.read(r);
                break;
            case SBIG('LINT'):
                x108_LINT.read(r);
                break;
            case SBIG('LINE'):
                x44_2_LINE = r.readBool(nullptr);
                break;
            case SBIG('LFOT'):
                x114_LFOT.read(r);
                break;
            case SBIG('LIT_'):
                x44_3_LIT_ = r.readBool(nullptr);
                break;
            case SBIG('LTME'):
                x34_LTME.read(r);
                break;
            case SBIG('LSLA'):
                x11c_LSLA.read(r);
                break;
            case SBIG('LTYP'):
                x100_LTYP.read(r);
                break;
            case SBIG('NDSY'):
                xb4_NDSY.read(r);
                break;
            case SBIG('MBSP'):
                x48_MBSP.read(r);
                break;
            case SBIG('MBLR'):
                x44_1_MBLR = r.readBool(nullptr);
                break;
            case SBIG('NCSY'):
                x9c_NCSY.read(r);
                break;
            case SBIG('PISY'):
                xc8_PISY.read(r);
                break;
            case SBIG('OPTS'):
                x45_4_OPTS = r.readBool(nullptr);
                break;
            case SBIG('PMAB'):
                x44_7_PMAB = r.readBool(nullptr);
                break;
            case SBIG('SESD'):
                xf8_SESD.read(r);
                break;
            case SBIG('SEPO'):
                xfc_SEPO.read(r);
                break;
            case SBIG('PSLT'):
                xc_PSLT.read(r);
                break;
            case SBIG('PMSC'):
                x74_PMSC.read(r);
                break;
            case SBIG('PMOP'):
                x6c_PMOP.read(r);
                break;
            case SBIG('PMDL'):
                x5c_PMDL.read(r);
                break;
            case SBIG('PMRT'):
                x70_PMRT.read(r);
                break;
            case SBIG('POFS'):
                x18_POFS.read(r);
                break;
            case SBIG('PMUS'):
                x45_5_PMUS = r.readBool(nullptr);
                break;
            case SBIG('PSIV'):
                x0_PSIV.read(r);
                break;
            case SBIG('ROTA'):
                x50_ROTA.read(r);
                break;
            case SBIG('PSVM'):
                x4_PSVM.read(r);
                break;
            case SBIG('PSTS'):
                x14_PSTS.read(r);
                break;
            case SBIG('PSOV'):
                x8_PSOV.read(r);
                break;
            case SBIG('PSWT'):
                x10_PSWT.read(r);
                break;
            case SBIG('PMLC'):
                xec_PMLC.read(r);
                break;
            case SBIG('PMED'):
                x1c_PMED.read(r);
                break;
            case SBIG('PMOO'):
                x45_6_PMOO = r.readBool(nullptr);
                break;
            case SBIG('SSSD'):
                xe4_SSSD.read(r);
                break;
            case SBIG('SORT'):
                x44_0_SORT = r.readBool(nullptr);
                break;
            case SBIG('SIZE'):
                x4c_SIZE.read(r);
                break;
            case SBIG('SISY'):
                xcc_SISY.read(r);
                break;
            case SBIG('SSPO'):
                xe8_SSPO.read(r);
                break;
            case SBIG('TEXR'):
                x54_TEXR.read(r);
                break;
            case SBIG('SSWH'):
                xd4_SSWH.read(r);
                break;
            case SBIG('TIND'):
                x58_TIND.read(r);
                break;
            case SBIG('VMD4'):
                x45_0_VMD4 = r.readBool(nullptr);
                break;
            case SBIG('VMD3'):
                x45_1_VMD3 = r.readBool(nullptr);
                break;
            case SBIG('VMD2'):
                x45_2_VMD2 = r.readBool(nullptr);
                break;
            case SBIG('VMD1'):
                x45_3_VMD1 = r.readBool(nullptr);
                break;
            case SBIG('VEL4'):
                x88_VEL4.read(r);
                break;
            case SBIG('VEL3'):
                x84_VEL3.read(r);
                break;
            case SBIG('VEL2'):
                x80_VEL2.read(r);
                break;
            case SBIG('VEL1'):
                x7c_VEL1.read(r);
                break;
            case SBIG('ZBUF'):
                x44_5_ZBUF = r.readBool(nullptr);
                break;
            case SBIG('WIDT'):
                x24_WIDT.read(r);
                break;
            case SBIG('ORNT'):
                x30_30_ORNT = r.readBool(nullptr);
                break;
            case SBIG('RSOP'):
                xUNK_RSOP = r.readBool(nullptr);
                break;
            default:
                break;
            }
            r.leaveSubRecord();
        }
    }
    void write(Athena::io::YAMLDocWriter& w) const
    {

    }
    size_t binarySize(size_t __isz) const
    {
        /* TODO: Figure out order of emitting */
        return __isz;
    }
    void read(Athena::io::IStreamReader& r)
    {
        uint32_t clsId;
        r.readBytesToBuf(&clsId, 4);
        if (clsId != SBIG('GPSM'))
        {
            LogModule.report(LogVisor::Warning, "non GPSM provided to GPSM parser");
            return;
        }
        r.readBytesToBuf(&clsId, 4);
        while (clsId != SBIG('_END'))
        {
            switch (clsId)
            {
            case SBIG('PMCL'):
                x78_PMCL.read(r);
                break;
            case SBIG('LFOR'):
                x118_LFOR.read(r);
                break;
            case SBIG('IDTS'):
                xa4_IDTS.read(r);
                break;
            case SBIG('EMTR'):
                x40_EMTR.read(r);
                break;
            case SBIG('COLR'):
                x30_COLR.read(r);
                break;
            case SBIG('CIND'):
                r.readUint32Big();
                x45_7_CIND = r.readBool();
                break;
            case SBIG('AAPH'):
                r.readUint32Big();
                x44_4_AAPH = r.readBool();
                break;
            case SBIG('CSSD'):
                xa0_CSSD.read(r);
                break;
            case SBIG('GRTE'):
                x2c_GRTE.read(r);
                break;
            case SBIG('FXLL'):
                r.readUint32Big();
                x44_6_FXLL = r.readBool();
                break;
            case SBIG('ICTS'):
                x8c_ICTS.read(r);
                break;
            case SBIG('KSSM'):
                xd0_KSSM.read(r);
                break;
            case SBIG('ILOC'):
                x38_ILOC.read(r);
                break;
            case SBIG('IITS'):
                break;
            case SBIG('IVEC'):
                x3c_IVEC.read(r);
                break;
            case SBIG('LDIR'):
                x110_LDIR.read(r);
                break;
            case SBIG('LCLR'):
                x104_LCLR.read(r);
                break;
            case SBIG('LENG'):
                x20_LENG.read(r);
                break;
            case SBIG('MAXP'):
                x28_MAXP.read(r);
                break;
            case SBIG('LOFF'):
                x10c_LOFF.read(r);
                break;
            case SBIG('LINT'):
                x108_LINT.read(r);
                break;
            case SBIG('LINE'):
                r.readUint32Big();
                x44_2_LINE = r.readBool();
                break;
            case SBIG('LFOT'):
                x114_LFOT.read(r);
                break;
            case SBIG('LIT_'):
                r.readUint32Big();
                x44_3_LIT_ = r.readBool();
                break;
            case SBIG('LTME'):
                x34_LTME.read(r);
                break;
            case SBIG('LSLA'):
                x11c_LSLA.read(r);
                break;
            case SBIG('LTYP'):
                x100_LTYP.read(r);
                break;
            case SBIG('NDSY'):
                xb4_NDSY.read(r);
                break;
            case SBIG('MBSP'):
                x48_MBSP.read(r);
                break;
            case SBIG('MBLR'):
                r.readUint32Big();
                x44_1_MBLR = r.readBool();
                break;
            case SBIG('NCSY'):
                x9c_NCSY.read(r);
                break;
            case SBIG('PISY'):
                xc8_PISY.read(r);
                break;
            case SBIG('OPTS'):
                r.readUint32Big();
                x45_4_OPTS = r.readBool();
                break;
            case SBIG('PMAB'):
                r.readUint32Big();
                x44_7_PMAB = r.readBool();
                break;
            case SBIG('SESD'):
                xf8_SESD.read(r);
                break;
            case SBIG('SEPO'):
                xfc_SEPO.read(r);
                break;
            case SBIG('PSLT'):
                xc_PSLT.read(r);
                break;
            case SBIG('PMSC'):
                x74_PMSC.read(r);
                break;
            case SBIG('PMOP'):
                x6c_PMOP.read(r);
                break;
            case SBIG('PMDL'):
                x5c_PMDL.read(r);
                break;
            case SBIG('PMRT'):
                x70_PMRT.read(r);
                break;
            case SBIG('POFS'):
                x18_POFS.read(r);
                break;
            case SBIG('PMUS'):
                r.readUint32Big();
                x45_5_PMUS = r.readBool();
                break;
            case SBIG('PSIV'):
                x0_PSIV.read(r);
                break;
            case SBIG('ROTA'):
                x50_ROTA.read(r);
                break;
            case SBIG('PSVM'):
                x4_PSVM.read(r);
                break;
            case SBIG('PSTS'):
                x14_PSTS.read(r);
                break;
            case SBIG('PSOV'):
                x8_PSOV.read(r);
                break;
            case SBIG('PSWT'):
                x10_PSWT.read(r);
                break;
            case SBIG('PMLC'):
                xec_PMLC.read(r);
                break;
            case SBIG('PMED'):
                x1c_PMED.read(r);
                break;
            case SBIG('PMOO'):
                r.readUint32Big();
                x45_6_PMOO = r.readBool();
                break;
            case SBIG('SSSD'):
                xe4_SSSD.read(r);
                break;
            case SBIG('SORT'):
                r.readUint32Big();
                x44_0_SORT = r.readBool();
                break;
            case SBIG('SIZE'):
                x4c_SIZE.read(r);
                break;
            case SBIG('SISY'):
                xcc_SISY.read(r);
                break;
            case SBIG('SSPO'):
                xe8_SSPO.read(r);
                break;
            case SBIG('TEXR'):
                x54_TEXR.read(r);
                break;
            case SBIG('SSWH'):
                xd4_SSWH.read(r);
                break;
            case SBIG('TIND'):
                x58_TIND.read(r);
                break;
            case SBIG('VMD4'):
                r.readUint32Big();
                x45_0_VMD4 = r.readBool();
                break;
            case SBIG('VMD3'):
                r.readUint32Big();
                x45_1_VMD3 = r.readBool();
                break;
            case SBIG('VMD2'):
                r.readUint32Big();
                x45_2_VMD2 = r.readBool();
                break;
            case SBIG('VMD1'):
                r.readUint32Big();
                x45_3_VMD1 = r.readBool();
                break;
            case SBIG('VEL4'):
                x88_VEL4.read(r);
                break;
            case SBIG('VEL3'):
                x84_VEL3.read(r);
                break;
            case SBIG('VEL2'):
                x80_VEL2.read(r);
                break;
            case SBIG('VEL1'):
                x7c_VEL1.read(r);
                break;
            case SBIG('ZBUF'):
                r.readUint32Big();
                x44_5_ZBUF = r.readBool();
                break;
            case SBIG('WIDT'):
                x24_WIDT.read(r);
                break;
            case SBIG('ORNT'):
                r.readUint32Big();
                x30_30_ORNT = r.readBool();
                break;
            case SBIG('RSOP'):
                r.readUint32Big();
                xUNK_RSOP = r.readBool();
                break;
            default:
                LogModule.report(LogVisor::Warning, "Unknown GPSM class %.4s @%" PRIi64, &clsId, r.position());
                printf("");
                break;
            }
            r.readBytesToBuf(&clsId, 4);
        }
    }
    void write(Athena::io::IStreamWriter& w) const
    {

    }
};

template <class IDType>
bool ExtractGPSM(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
{
    FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("w"));
    if (fp)
    {
        GPSM<IDType> gpsm;
        gpsm.read(rs);
        gpsm.toYAMLFile(fp);
        fclose(fp);
        return true;
    }
    return false;
}

}
}

#endif // __COMMON_PART_HPP__
