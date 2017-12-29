#include "PART.hpp"

namespace DataSpec::DNAParticle
{

template <class IDType>
void GPSM<IDType>::read(athena::io::YAMLDocReader& r)
{
    for (const auto& elem : r.getCurNode()->m_mapChildren)
    {
        if (elem.first.size() < 4)
        {
            LogModule.report(logvisor::Warning, "short FourCC in element '%s'", elem.first.c_str());
            continue;
        }

        if (auto rec = r.enterSubRecord(elem.first.c_str()))
        {
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
                x45_30_CIND = r.readBool(nullptr);
            break;
            case SBIG('AAPH'):
                x44_26_AAPH = r.readBool(nullptr);
            break;
            case SBIG('CSSD'):
                xa0_CSSD.read(r);
            break;
            case SBIG('GRTE'):
                x2c_GRTE.read(r);
            break;
            case SBIG('FXLL'):
                x44_25_FXLL = r.readBool(nullptr);
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
                xb8_IITS.read(r);
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
                x44_24_LINE = r.readBool(nullptr);
            break;
            case SBIG('LFOT'):
                x114_LFOT.read(r);
            break;
            case SBIG('LIT_'):
                x44_29_LIT_ = r.readBool(nullptr);
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
                x44_30_MBLR = r.readBool(nullptr);
            break;
            case SBIG('NCSY'):
                x9c_NCSY.read(r);
            break;
            case SBIG('PISY'):
                xc8_PISY.read(r);
            break;
            case SBIG('OPTS'):
                x45_31_OPTS = r.readBool(nullptr);
            break;
            case SBIG('PMAB'):
                x44_31_PMAB = r.readBool(nullptr);
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
                x45_24_PMUS = r.readBool(nullptr);
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
            case SBIG('SEED'):
                x1c_SEED.read(r);
            break;
            case SBIG('PMOO'):
                x45_25_PMOO = r.readBool(nullptr);
            break;
            case SBIG('SSSD'):
                xe4_SSSD.read(r);
            break;
            case SBIG('SORT'):
                x44_28_SORT = r.readBool(nullptr);
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
                x45_29_VMD4 = r.readBool(nullptr);
            break;
            case SBIG('VMD3'):
                x45_28_VMD3 = r.readBool(nullptr);
            break;
            case SBIG('VMD2'):
                x45_27_VMD2 = r.readBool(nullptr);
            break;
            case SBIG('VMD1'):
                x45_26_VMD1 = r.readBool(nullptr);
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
                x44_27_ZBUF = r.readBool(nullptr);
            break;
            case SBIG('WIDT'):
                x24_WIDT.read(r);
            break;
            case SBIG('ORNT'):
                x30_30_ORNT = r.readBool(nullptr);
            break;
            case SBIG('RSOP'):
                x30_31_RSOP = r.readBool(nullptr);
            break;
            case SBIG('ADV1'):
                x10c_ADV1.read(r);
            break;
            case SBIG('ADV2'):
                x110_ADV2.read(r);
            break;
            case SBIG('ADV3'):
                x114_ADV3.read(r);
            break;
            case SBIG('ADV4'):
                x118_ADV4.read(r);
            break;
            case SBIG('ADV5'):
                x11c_ADV5.read(r);
            break;
            case SBIG('ADV6'):
                x120_ADV6.read(r);
            break;
            case SBIG('ADV7'):
                x124_ADV7.read(r);
            break;
            case SBIG('SELC'):
                xd8_SELC.read(r);
            break;
            default:
            break;
            }
        }
    }
}

template <class IDType>
void GPSM<IDType>::write(athena::io::YAMLDocWriter& w) const
{
    if (x0_PSIV)
        if (auto rec = w.enterSubRecord("PSIV"))
            x0_PSIV.write(w);
    if (x4_PSVM)
        if (auto rec = w.enterSubRecord("PSVM"))
            x4_PSVM.write(w);
    if (x8_PSOV)
        if (auto rec = w.enterSubRecord("PSOV"))
            x8_PSOV.write(w);
    if (xc_PSLT)
        if (auto rec = w.enterSubRecord("PSLT"))
            xc_PSLT.write(w);
    if (x10_PSWT)
        if (auto rec = w.enterSubRecord("PSWT"))
            x10_PSWT.write(w);
    if (x14_PSTS)
        if (auto rec = w.enterSubRecord("PSTS"))
            x14_PSTS.write(w);
    if (x18_POFS)
        if (auto rec = w.enterSubRecord("POFS"))
            x18_POFS.write(w);
    if (x1c_SEED)
        if (auto rec = w.enterSubRecord("SEED"))
            x1c_SEED.write(w);
    if (x20_LENG)
        if (auto rec = w.enterSubRecord("LENG"))
            x20_LENG.write(w);
    if (x24_WIDT)
        if (auto rec = w.enterSubRecord("WIDT"))
            x24_WIDT.write(w);
    if (x28_MAXP)
        if (auto rec = w.enterSubRecord("MAXP"))
            x28_MAXP.write(w);
    if (x2c_GRTE)
        if (auto rec = w.enterSubRecord("GRTE"))
            x2c_GRTE.write(w);
    if (x30_COLR)
        if (auto rec = w.enterSubRecord("COLR"))
            x30_COLR.write(w);
    if (x34_LTME)
        if (auto rec = w.enterSubRecord("LTME"))
            x34_LTME.write(w);
    if (x38_ILOC)
        if (auto rec = w.enterSubRecord("ILOC"))
            x38_ILOC.write(w);
    if (x3c_IVEC)
        if (auto rec = w.enterSubRecord("IVEC"))
            x3c_IVEC.write(w);
    if (x40_EMTR)
        if (auto rec = w.enterSubRecord("EMTR"))
            x40_EMTR.write(w);
    if (x44_24_LINE)
        w.writeBool("LINE", true);
    if (x44_25_FXLL)
        w.writeBool("FXLL", true);
    if (x44_26_AAPH)
        w.writeBool("AAPH", true);
    if (x44_27_ZBUF)
        w.writeBool("ZBUF", true);
    if (x44_28_SORT)
        w.writeBool("SORT", true);
    if (x44_29_LIT_)
        w.writeBool("LIT_", true);
    if (x44_30_MBLR)
        w.writeBool("MBLR", true);
    if (x44_31_PMAB)
        w.writeBool("PMAB", true);
    if (x45_24_PMUS)
        w.writeBool("PMUS", true);
    if (!x45_25_PMOO)
        w.writeBool("PMOO", false);
    if (x45_26_VMD1)
        w.writeBool("VMD1", true);
    if (x45_27_VMD2)
        w.writeBool("VMD2", true);
    if (x45_28_VMD3)
        w.writeBool("VMD3", true);
    if (x45_29_VMD4)
        w.writeBool("VMD4", true);
    if (x45_30_CIND)
        w.writeBool("CIND", true);
    if (x45_31_OPTS)
        w.writeBool("OPTS", true);
    if (x30_30_ORNT)
        w.writeBool("ORNT", true);
    if (x30_31_RSOP)
        w.writeBool("RSOP", true);
    if (x48_MBSP)
        if (auto rec = w.enterSubRecord("MPSB"))
            x48_MBSP.write(w);
    if (x4c_SIZE)
        if (auto rec = w.enterSubRecord("SIZE"))
            x4c_SIZE.write(w);
    if (x50_ROTA)
        if (auto rec = w.enterSubRecord("ROTA"))
            x50_ROTA.write(w);
    if (x54_TEXR)
        if (auto rec = w.enterSubRecord("TEXR"))
            x54_TEXR.write(w);
    if (x58_TIND)
        if (auto rec = w.enterSubRecord("TIND"))
            x58_TIND.write(w);
    if (x5c_PMDL)
        if (auto rec = w.enterSubRecord("PMDL"))
            x5c_PMDL.write(w);
    if (x6c_PMOP)
        if (auto rec = w.enterSubRecord("PMOP"))
            x6c_PMOP.write(w);
    if (x70_PMRT)
        if (auto rec = w.enterSubRecord("PMRT"))
            x70_PMRT.write(w);
    if (x74_PMSC)
        if (auto rec = w.enterSubRecord("PMSC"))
            x74_PMSC.write(w);
    if (x78_PMCL)
        if (auto rec = w.enterSubRecord("PMCL"))
            x78_PMCL.write(w);
    if (x7c_VEL1)
        if (auto rec = w.enterSubRecord("VEL1"))
            x7c_VEL1.write(w);
    if (x80_VEL2)
        if (auto rec = w.enterSubRecord("VEL2"))
            x80_VEL2.write(w);
    if (x84_VEL3)
        if (auto rec = w.enterSubRecord("VEL3"))
            x84_VEL3.write(w);
    if (x88_VEL4)
        if (auto rec = w.enterSubRecord("VEL4"))
            x88_VEL4.write(w);
    if (x8c_ICTS)
        if (auto rec = w.enterSubRecord("ICTS"))
            x8c_ICTS.write(w);
    if (x9c_NCSY)
        if (auto rec = w.enterSubRecord("NCSY"))
            x9c_NCSY.write(w);
    if (xa0_CSSD)
        if (auto rec = w.enterSubRecord("CSSD"))
            xa0_CSSD.write(w);
    if (xa4_IDTS)
        if (auto rec = w.enterSubRecord("IDTS"))
            xa4_IDTS.write(w);
    if (xb4_NDSY)
        if (auto rec = w.enterSubRecord("NDSY"))
            xb4_NDSY.write(w);
    if (xb8_IITS)
        if (auto rec = w.enterSubRecord("IITS"))
            xb8_IITS.write(w);
    if (xc8_PISY)
        if (auto rec = w.enterSubRecord("PISY"))
            xc8_PISY.write(w);
    if (xcc_SISY)
        if (auto rec = w.enterSubRecord("SISY"))
            xcc_SISY.write(w);
    if (xd0_KSSM)
        if (auto rec = w.enterSubRecord("KSSM"))
            xd0_KSSM.write(w);
    if (xd4_SSWH)
        if (auto rec = w.enterSubRecord("SSWH"))
            xd4_SSWH.write(w);
    if (xd8_SELC)
        if (auto rec = w.enterSubRecord("SELC"))
            xd8_SELC.write(w);
    if (xe4_SSSD)
        if (auto rec = w.enterSubRecord("SSSD"))
            xe4_SSSD.write(w);
    if (xe8_SSPO)
        if (auto rec = w.enterSubRecord("SSPO"))
            xe8_SSPO.write(w);
    if (xf8_SESD)
        if (auto rec = w.enterSubRecord("SESD"))
            xf8_SESD.write(w);
    if (xfc_SEPO)
        if (auto rec = w.enterSubRecord("SEPO"))
            xfc_SEPO.write(w);
    if (xec_PMLC)
        if (auto rec = w.enterSubRecord("PMLC"))
            xec_PMLC.write(w);
    if (x100_LTYP)
        if (auto rec = w.enterSubRecord("LTYP"))
            x100_LTYP.write(w);
    if (x104_LCLR)
        if (auto rec = w.enterSubRecord("LCLR"))
            x104_LCLR.write(w);
    if (x108_LINT)
        if (auto rec = w.enterSubRecord("LINT"))
            x108_LINT.write(w);
    if (x10c_LOFF)
        if (auto rec = w.enterSubRecord("LOFF"))
            x10c_LOFF.write(w);
    if (x110_LDIR)
        if (auto rec = w.enterSubRecord("LDIR"))
            x110_LDIR.write(w);
    if (x114_LFOT)
        if (auto rec = w.enterSubRecord("LFOT"))
            x114_LFOT.write(w);
    if (x118_LFOR)
        if (auto rec = w.enterSubRecord("LFOR"))
            x118_LFOR.write(w);
    if (x11c_LSLA)
        if (auto rec = w.enterSubRecord("LSLA"))
            x11c_LSLA.write(w);
    if (x10c_ADV1)
        if (auto rec = w.enterSubRecord("ADV1"))
            x10c_ADV1.write(w);
    if (x110_ADV2)
        if (auto rec = w.enterSubRecord("ADV2"))
            x110_ADV2.write(w);
    if (x114_ADV3)
        if (auto rec = w.enterSubRecord("ADV3"))
            x114_ADV3.write(w);
    if (x118_ADV4)
        if (auto rec = w.enterSubRecord("ADV4"))
            x118_ADV4.write(w);
    if (x11c_ADV5)
        if (auto rec = w.enterSubRecord("ADV5"))
            x11c_ADV5.write(w);
    if (x120_ADV6)
        if (auto rec = w.enterSubRecord("ADV6"))
            x120_ADV6.write(w);
    if (x124_ADV7)
        if (auto rec = w.enterSubRecord("ADV7"))
            x124_ADV7.write(w);
    if (x128_ADV8)
        if (auto rec = w.enterSubRecord("ADV8"))
            x128_ADV8.write(w);
}

template <class IDType>
size_t GPSM<IDType>::binarySize(size_t __isz) const
{
    __isz += 4;
    if (x0_PSIV)
        __isz = x0_PSIV.binarySize(__isz + 4);
    if (x4_PSVM)
        __isz = x4_PSVM.binarySize(__isz + 4);
    if (x8_PSOV)
        __isz = x8_PSOV.binarySize(__isz + 4);
    if (xc_PSLT)
        __isz = xc_PSLT.binarySize(__isz + 4);
    if (x10_PSWT)
        __isz = x10_PSWT.binarySize(__isz + 4);
    if (x14_PSTS)
        __isz = x14_PSTS.binarySize(__isz + 4);
    if (x18_POFS)
        __isz = x18_POFS.binarySize(__isz + 4);
    if (x1c_SEED)
        __isz = x1c_SEED.binarySize(__isz + 4);
    if (x20_LENG)
        __isz = x20_LENG.binarySize(__isz + 4);
    if (x24_WIDT)
        __isz = x24_WIDT.binarySize(__isz + 4);
    if (x28_MAXP)
        __isz = x28_MAXP.binarySize(__isz + 4);
    if (x2c_GRTE)
        __isz = x2c_GRTE.binarySize(__isz + 4);
    if (x30_COLR)
        __isz = x30_COLR.binarySize(__isz + 4);
    if (x34_LTME)
        __isz = x34_LTME.binarySize(__isz + 4);
    if (x38_ILOC)
        __isz = x38_ILOC.binarySize(__isz + 4);
    if (x3c_IVEC)
        __isz = x3c_IVEC.binarySize(__isz + 4);
    if (x40_EMTR)
        __isz = x40_EMTR.binarySize(__isz + 4);
    if (x44_24_LINE)
        __isz += 9;
    if (x44_25_FXLL)
        __isz += 9;
    if (x44_26_AAPH)
        __isz += 9;
    if (x44_27_ZBUF)
        __isz += 9;
    if (x44_28_SORT)
        __isz += 9;
    if (x44_29_LIT_)
        __isz += 9;
    if (x44_30_MBLR)
        __isz += 9;
    if (x44_31_PMAB)
        __isz += 9;
    if (x45_24_PMUS)
        __isz += 9;
    if (!x45_25_PMOO)
        __isz += 9;
    if (x45_26_VMD1)
        __isz += 9;
    if (x45_27_VMD2)
        __isz += 9;
    if (x45_28_VMD3)
        __isz += 9;
    if (x45_29_VMD4)
        __isz += 9;
    if (x45_30_CIND)
        __isz += 9;
    if (x45_31_OPTS)
        __isz += 9;
    if (x30_30_ORNT)
        __isz += 9;
    if (x30_31_RSOP)
        __isz += 9;
    if (x48_MBSP)
        __isz = x48_MBSP.binarySize(__isz + 4);
    if (x4c_SIZE)
        __isz = x4c_SIZE.binarySize(__isz + 4);
    if (x50_ROTA)
        __isz = x50_ROTA.binarySize(__isz + 4);
    if (x54_TEXR)
        __isz = x54_TEXR.binarySize(__isz + 4);
    if (x58_TIND)
        __isz = x58_TIND.binarySize(__isz + 4);
    if (x5c_PMDL)
        __isz = x5c_PMDL.binarySize(__isz + 4);
    if (x6c_PMOP)
        __isz = x6c_PMOP.binarySize(__isz + 4);
    if (x70_PMRT)
        __isz = x70_PMRT.binarySize(__isz + 4);
    if (x74_PMSC)
        __isz = x74_PMSC.binarySize(__isz + 4);
    if (x78_PMCL)
        __isz = x78_PMCL.binarySize(__isz + 4);
    if (x7c_VEL1)
        __isz = x7c_VEL1.binarySize(__isz + 4);
    if (x80_VEL2)
        __isz = x80_VEL2.binarySize(__isz + 4);
    if (x84_VEL3)
        __isz = x84_VEL3.binarySize(__isz + 4);
    if (x88_VEL4)
        __isz = x88_VEL4.binarySize(__isz + 4);
    if (x8c_ICTS)
        __isz = x8c_ICTS.binarySize(__isz + 4);
    if (x9c_NCSY)
        __isz = x9c_NCSY.binarySize(__isz + 4);
    if (xa0_CSSD)
        __isz = xa0_CSSD.binarySize(__isz + 4);
    if (xa4_IDTS)
        __isz = xa4_IDTS.binarySize(__isz + 4);
    if (xb4_NDSY)
        __isz = xb4_NDSY.binarySize(__isz + 4);
    if (xb8_IITS)
        __isz = xb8_IITS.binarySize(__isz + 4);
    if (xc8_PISY)
        __isz = xc8_PISY.binarySize(__isz + 4);
    if (xcc_SISY)
        __isz = xcc_SISY.binarySize(__isz + 4);
    if (xd0_KSSM)
        __isz = xd0_KSSM.binarySize(__isz + 4);
    if (xd4_SSWH)
        __isz = xd4_SSWH.binarySize(__isz + 4);
    if (xd8_SELC)
        __isz = xd8_SELC.binarySize(__isz + 4);
    if (xe4_SSSD)
        __isz = xe4_SSSD.binarySize(__isz + 4);
    if (xe8_SSPO)
        __isz = xe8_SSPO.binarySize(__isz + 4);
    if (xf8_SESD)
        __isz = xf8_SESD.binarySize(__isz + 4);
    if (xfc_SEPO)
        __isz = xfc_SEPO.binarySize(__isz + 4);
    if (xec_PMLC)
        __isz = xec_PMLC.binarySize(__isz + 4);
    if (x100_LTYP)
        __isz = x100_LTYP.binarySize(__isz + 4);
    if (x104_LCLR)
        __isz = x104_LCLR.binarySize(__isz + 4);
    if (x108_LINT)
        __isz = x108_LINT.binarySize(__isz + 4);
    if (x10c_LOFF)
        __isz = x10c_LOFF.binarySize(__isz + 4);
    if (x110_LDIR)
        __isz = x110_LDIR.binarySize(__isz + 4);
    if (x114_LFOT)
        __isz = x114_LFOT.binarySize(__isz + 4);
    if (x118_LFOR)
        __isz = x118_LFOR.binarySize(__isz + 4);
    if (x11c_LSLA)
        __isz = x11c_LSLA.binarySize(__isz + 4);
    if (x10c_ADV1)
        __isz = x10c_ADV1.binarySize(__isz + 4);
    if (x110_ADV2)
        __isz = x110_ADV2.binarySize(__isz + 4);
    if (x114_ADV3)
        __isz = x114_ADV3.binarySize(__isz + 4);
    if (x118_ADV4)
        __isz = x118_ADV4.binarySize(__isz + 4);
    if (x11c_ADV5)
        __isz = x11c_ADV5.binarySize(__isz + 4);
    if (x120_ADV6)
        __isz = x120_ADV6.binarySize(__isz + 4);
    if (x124_ADV7)
        __isz = x124_ADV7.binarySize(__isz + 4);
    if (x128_ADV8)
        __isz = x128_ADV8.binarySize(__isz + 4);
    return __isz;
}

template <class IDType>
void GPSM<IDType>::read(athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    if (clsId != SBIG('GPSM'))
    {
        LogModule.report(logvisor::Warning, "non GPSM provided to GPSM parser");
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
            x45_30_CIND = r.readBool();
        break;
        case SBIG('AAPH'):
            r.readUint32Big();
            x44_26_AAPH = r.readBool();
        break;
        case SBIG('CSSD'):
            xa0_CSSD.read(r);
        break;
        case SBIG('GRTE'):
            x2c_GRTE.read(r);
        break;
        case SBIG('FXLL'):
            r.readUint32Big();
            x44_25_FXLL = r.readBool();
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
            xb8_IITS.read(r);
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
            x44_24_LINE = r.readBool();
        break;
        case SBIG('LFOT'):
            x114_LFOT.read(r);
        break;
        case SBIG('LIT_'):
            r.readUint32Big();
            x44_29_LIT_ = r.readBool();
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
            x44_30_MBLR = r.readBool();
        break;
        case SBIG('NCSY'):
            x9c_NCSY.read(r);
        break;
        case SBIG('PISY'):
            xc8_PISY.read(r);
        break;
        case SBIG('OPTS'):
            r.readUint32Big();
            x45_31_OPTS = r.readBool();
        break;
        case SBIG('PMAB'):
            r.readUint32Big();
            x44_31_PMAB = r.readBool();
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
            x45_24_PMUS = r.readBool();
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
        case SBIG('SEED'):
            x1c_SEED.read(r);
        break;
        case SBIG('PMOO'):
            r.readUint32Big();
            x45_25_PMOO = r.readBool();
        break;
        case SBIG('SSSD'):
            xe4_SSSD.read(r);
        break;
        case SBIG('SORT'):
            r.readUint32Big();
            x44_28_SORT = r.readBool();
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
            x45_29_VMD4 = r.readBool();
        break;
        case SBIG('VMD3'):
            r.readUint32Big();
            x45_28_VMD3 = r.readBool();
        break;
        case SBIG('VMD2'):
            r.readUint32Big();
            x45_27_VMD2 = r.readBool();
        break;
        case SBIG('VMD1'):
            r.readUint32Big();
            x45_26_VMD1 = r.readBool();
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
            x44_27_ZBUF = r.readBool();
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
            x30_31_RSOP = r.readBool();
        break;
        case SBIG('ADV1'):
            x10c_ADV1.read(r);
        break;
        case SBIG('ADV2'):
            x110_ADV2.read(r);
        break;
        case SBIG('ADV3'):
            x114_ADV3.read(r);
        break;
        case SBIG('ADV4'):
            x118_ADV4.read(r);
        break;
        case SBIG('ADV5'):
            x11c_ADV5.read(r);
        break;
        case SBIG('ADV6'):
            x120_ADV6.read(r);
        break;
        case SBIG('ADV7'):
            x124_ADV7.read(r);
        break;
        case SBIG('ADV8'):
            x128_ADV8.read(r);
        break;
        case SBIG('SELC'):
            xd8_SELC.read(r);
        break;
        default:
            LogModule.report(logvisor::Fatal, "Unknown GPSM class %.4s @%" PRIi64, &clsId, r.position());
        break;
        }
        r.readBytesToBuf(&clsId, 4);
    }
}

template <class IDType>
void GPSM<IDType>::write(athena::io::IStreamWriter& w) const
{
    w.writeBytes((atInt8*)"GPSM", 4);
    if (x0_PSIV)
    {
        w.writeBytes((atInt8*)"PSIV", 4);
        x0_PSIV.write(w);
    }
    if (x4_PSVM)
    {
        w.writeBytes((atInt8*)"PSVM", 4);
        x4_PSVM.write(w);
    }
    if (x8_PSOV)
    {
        w.writeBytes((atInt8*)"PSOV", 4);
        x8_PSOV.write(w);
    }
    if (xc_PSLT)
    {
        w.writeBytes((atInt8*)"PSLT", 4);
        xc_PSLT.write(w);
    }
    if (x10_PSWT)
    {
        w.writeBytes((atInt8*)"PSWT", 4);
        x10_PSWT.write(w);
    }
    if (x14_PSTS)
    {
        w.writeBytes((atInt8*)"PSTS", 4);
        x14_PSTS.write(w);
    }
    if (x18_POFS)
    {
        w.writeBytes((atInt8*)"POFS", 4);
        x18_POFS.write(w);
    }
    if (x1c_SEED)
    {
        w.writeBytes((atInt8*)"SEED", 4);
        x1c_SEED.write(w);
    }
    if (x20_LENG)
    {
        w.writeBytes((atInt8*)"LENG", 4);
        x20_LENG.write(w);
    }
    if (x24_WIDT)
    {
        w.writeBytes((atInt8*)"WIDT", 4);
        x24_WIDT.write(w);
    }
    if (x28_MAXP)
    {
        w.writeBytes((atInt8*)"MAXP", 4);
        x28_MAXP.write(w);
    }
    if (x2c_GRTE)
    {
        w.writeBytes((atInt8*)"GRTE", 4);
        x2c_GRTE.write(w);
    }
    if (x30_COLR)
    {
        w.writeBytes((atInt8*)"COLR", 4);
        x30_COLR.write(w);
    }
    if (x34_LTME)
    {
        w.writeBytes((atInt8*)"LTME", 4);
        x34_LTME.write(w);
    }
    if (x38_ILOC)
    {
        w.writeBytes((atInt8*)"ILOC", 4);
        x38_ILOC.write(w);
    }
    if (x3c_IVEC)
    {
        w.writeBytes((atInt8*)"IVEC", 4);
        x3c_IVEC.write(w);
    }
    if (x40_EMTR)
    {
        w.writeBytes((atInt8*)"EMTR", 4);
        x40_EMTR.write(w);
    }
    if (x44_24_LINE)
    {
        w.writeBytes((atInt8*)"LINECNST\x01", 9);
    }
    if (x44_25_FXLL)
    {
        w.writeBytes((atInt8*)"FXLLCNST\x01", 9);
    }
    if (x44_26_AAPH)
    {
        w.writeBytes((atInt8*)"AAPHCNST\x01", 9);
    }
    if (x44_27_ZBUF)
    {
        w.writeBytes((atInt8*)"ZBUFCNST\x01", 9);
    }
    if (x44_28_SORT)
    {
        w.writeBytes((atInt8*)"SORTCNST\x01", 9);
    }
    if (x44_29_LIT_)
    {
        w.writeBytes((atInt8*)"LIT_CNST\x01", 9);
    }
    if (x44_30_MBLR)
    {
        w.writeBytes((atInt8*)"MBLRCNST\x01", 9);
    }
    if (x44_31_PMAB)
    {
        w.writeBytes((atInt8*)"PMABCNST\x01", 9);
    }
    if (x45_24_PMUS)
    {
        w.writeBytes((atInt8*)"PMUSCNST\x01", 9);
    }
    if (!x45_25_PMOO)
    {
        w.writeBytes((atInt8*)"PMOOCNST\x00", 9);
    }
    if (x45_26_VMD1)
    {
        w.writeBytes((atInt8*)"VMD1CNST\x01", 9);
    }
    if (x45_27_VMD2)
    {
        w.writeBytes((atInt8*)"VMD2CNST\x01", 9);
    }
    if (x45_28_VMD3)
    {
        w.writeBytes((atInt8*)"VMD3CNST\x01", 9);
    }
    if (x45_29_VMD4)
    {
        w.writeBytes((atInt8*)"VMD4CNST\x01", 9);
    }
    if (x45_30_CIND)
    {
        w.writeBytes((atInt8*)"CINDCNST\x01", 9);
    }
    if (x45_31_OPTS)
    {
        w.writeBytes((atInt8*)"OPTSCNST\x01", 9);
    }
    if (x30_30_ORNT)
    {
        w.writeBytes((atInt8*)"ORNTCNST\x01", 9);
    }
    if (x30_31_RSOP)
    {
        w.writeBytes((atInt8*)"RSOPCNST\x01", 9);
    }
    if (x48_MBSP)
    {
        w.writeBytes((atInt8*)"MBSP", 4);
        x48_MBSP.write(w);
    }
    if (x4c_SIZE)
    {
        w.writeBytes((atInt8*)"SIZE", 4);
        x4c_SIZE.write(w);
    }
    if (x50_ROTA)
    {
        w.writeBytes((atInt8*)"ROTA", 4);
        x50_ROTA.write(w);
    }
    if (x54_TEXR)
    {
        w.writeBytes((atInt8*)"TEXR", 4);
        x54_TEXR.write(w);
    }
    if (x58_TIND)
    {
        w.writeBytes((atInt8*)"TIND", 4);
        x58_TIND.write(w);
    }
    if (x5c_PMDL)
    {
        w.writeBytes((atInt8*)"PMDL", 4);
        x5c_PMDL.write(w);
    }
    if (x6c_PMOP)
    {
        w.writeBytes((atInt8*)"PMOP", 4);
        x6c_PMOP.write(w);
    }
    if (x70_PMRT)
    {
        w.writeBytes((atInt8*)"PMRT", 4);
        x70_PMRT.write(w);
    }
    if (x74_PMSC)
    {
        w.writeBytes((atInt8*)"PMSC", 4);
        x74_PMSC.write(w);
    }
    if (x78_PMCL)
    {
        w.writeBytes((atInt8*)"PMCL", 4);
        x78_PMCL.write(w);
    }
    if (x7c_VEL1)
    {
        w.writeBytes((atInt8*)"VEL1", 4);
        x7c_VEL1.write(w);
    }
    if (x80_VEL2)
    {
        w.writeBytes((atInt8*)"VEL2", 4);
        x80_VEL2.write(w);
    }
    if (x84_VEL3)
    {
        w.writeBytes((atInt8*)"VEL3", 4);
        x84_VEL3.write(w);
    }
    if (x88_VEL4)
    {
        w.writeBytes((atInt8*)"VEL4", 4);
        x88_VEL4.write(w);
    }
    if (x8c_ICTS)
    {
        w.writeBytes((atInt8*)"ICTS", 4);
        x8c_ICTS.write(w);
    }
    if (x9c_NCSY)
    {
        w.writeBytes((atInt8*)"NCSY", 4);
        x9c_NCSY.write(w);
    }
    if (xa0_CSSD)
    {
        w.writeBytes((atInt8*)"CSSD", 4);
        xa0_CSSD.write(w);
    }
    if (xa4_IDTS)
    {
        w.writeBytes((atInt8*)"IDTS", 4);
        xa4_IDTS.write(w);
    }
    if (xb4_NDSY)
    {
        w.writeBytes((atInt8*)"NDSY", 4);
        xb4_NDSY.write(w);
    }
    if (xb8_IITS)
    {
        w.writeBytes((atInt8*)"IITS", 4);
        xb8_IITS.write(w);
    }
    if (xc8_PISY)
    {
        w.writeBytes((atInt8*)"PISY", 4);
        xc8_PISY.write(w);
    }
    if (xcc_SISY)
    {
        w.writeBytes((atInt8*)"SISY", 4);
        xcc_SISY.write(w);
    }
    if (xd0_KSSM)
    {
        w.writeBytes((atInt8*)"KSSM", 4);
        xd0_KSSM.write(w);
    }
    if (xd4_SSWH)
    {
        w.writeBytes((atInt8*)"SSWH", 4);
        xd4_SSWH.write(w);
    }
    if (xd8_SELC)
    {
        w.writeBytes((atInt8*)"SELC", 4);
        xd8_SELC.write(w);
    }
    if (xe4_SSSD)
    {
        w.writeBytes((atInt8*)"SSSD", 4);
        xe4_SSSD.write(w);
    }
    if (xe8_SSPO)
    {
        w.writeBytes((atInt8*)"SSPO", 4);
        xe8_SSPO.write(w);
    }
    if (xf8_SESD)
    {
        w.writeBytes((atInt8*)"SESD", 4);
        xf8_SESD.write(w);
    }
    if (xfc_SEPO)
    {
        w.writeBytes((atInt8*)"SEPO", 4);
        xfc_SEPO.write(w);
    }
    if (xec_PMLC)
    {
        w.writeBytes((atInt8*)"PMLC", 4);
        xec_PMLC.write(w);
    }
    if (x100_LTYP)
    {
        w.writeBytes((atInt8*)"LTYP", 4);
        x100_LTYP.write(w);
    }
    if (x104_LCLR)
    {
        w.writeBytes((atInt8*)"LCLR", 4);
        x104_LCLR.write(w);
    }
    if (x108_LINT)
    {
        w.writeBytes((atInt8*)"LINT", 4);
        x108_LINT.write(w);
    }
    if (x10c_LOFF)
    {
        w.writeBytes((atInt8*)"LOFF", 4);
        x10c_LOFF.write(w);
    }
    if (x110_LDIR)
    {
        w.writeBytes((atInt8*)"LDIR", 4);
        x110_LDIR.write(w);
    }
    if (x114_LFOT)
    {
        w.writeBytes((atInt8*)"LFOT", 4);
        x114_LFOT.write(w);
    }
    if (x118_LFOR)
    {
        w.writeBytes((atInt8*)"LFOR", 4);
        x118_LFOR.write(w);
    }
    if (x11c_LSLA)
    {
        w.writeBytes((atInt8*)"LSLA", 4);
        x11c_LSLA.write(w);
    }
    if (x10c_ADV1)
    {
        w.writeBytes((atInt8*)"ADV1", 4);
        x10c_ADV1.write(w);
    }
    if (x110_ADV2)
    {
        w.writeBytes((atInt8*)"ADV2", 4);
        x110_ADV2.write(w);
    }
    if (x114_ADV3)
    {
        w.writeBytes((atInt8*)"ADV3", 4);
        x114_ADV3.write(w);
    }
    if (x118_ADV4)
    {
        w.writeBytes((atInt8*)"ADV4", 4);
        x118_ADV4.write(w);
    }
    if (x11c_ADV5)
    {
        w.writeBytes((atInt8*)"ADV5", 4);
        x11c_ADV5.write(w);
    }
    if (x120_ADV6)
    {
        w.writeBytes((atInt8*)"ADV6", 4);
        x120_ADV6.write(w);
    }
    if (x124_ADV7)
    {
        w.writeBytes((atInt8*)"ADV7", 4);
        x124_ADV7.write(w);
    }
    if (x128_ADV8)
    {
        w.writeBytes((atInt8*)"ADV8", 4);
        x128_ADV8.write(w);
    }
    w.writeBytes("_END", 4);
}

template <class IDType>
void GPSM<IDType>::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
{
    if (x54_TEXR.m_elem)
        x54_TEXR.m_elem->gatherDependencies(pathsOut);
    if (x58_TIND.m_elem)
        x58_TIND.m_elem->gatherDependencies(pathsOut);
    g_curSpec->flattenDependencies(x5c_PMDL.id, pathsOut);
    g_curSpec->flattenDependencies(x8c_ICTS.id, pathsOut);
    g_curSpec->flattenDependencies(xa4_IDTS.id, pathsOut);
    g_curSpec->flattenDependencies(xb8_IITS.id, pathsOut);
    xd0_KSSM.gatherDependencies(pathsOut);
    g_curSpec->flattenDependencies(xd4_SSWH.id, pathsOut);
    g_curSpec->flattenDependencies(xec_PMLC.id, pathsOut);
    g_curSpec->flattenDependencies(xd8_SELC.id, pathsOut);
}

template struct GPSM<UniqueID32>;
template struct GPSM<UniqueID64>;

template <class IDType>
bool ExtractGPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        GPSM<IDType> gpsm;
        gpsm.read(rs);
        gpsm.toYAMLStream(writer);
        return true;
    }
    return false;
}
template bool ExtractGPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractGPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteGPSM(const GPSM<IDType>& gpsm, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    gpsm.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}
template bool WriteGPSM<UniqueID32>(const GPSM<UniqueID32>& gpsm, const hecl::ProjectPath& outPath);
template bool WriteGPSM<UniqueID64>(const GPSM<UniqueID64>& gpsm, const hecl::ProjectPath& outPath);

}
