#include "PART.hpp"

namespace DataSpec::DNAParticle {

template <>
const char* GPSM<UniqueID32>::DNAType() {
  return "GPSM<UniqueID32>";
}

template <>
const char* GPSM<UniqueID64>::DNAType() {
  return "GPSM<UniqueID64>";
}

template <class IDType>
void GPSM<IDType>::_read(typename ReadYaml::StreamT& r) {
  for (const auto& elem : r.getCurNode()->m_mapChildren) {
    if (elem.first.size() < 4) {
      LogModule.report(logvisor::Warning, fmt("short FourCC in element '{}'"), elem.first);
      continue;
    }

    if (auto rec = r.enterSubRecord(elem.first.c_str())) {
      switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
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
void GPSM<IDType>::_write(typename WriteYaml::StreamT& w) const {
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
    if (auto rec = w.enterSubRecord("MBSP"))
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
void GPSM<IDType>::_binarySize(typename BinarySize::StreamT& s) const {
  s += 4;
  if (x0_PSIV) {
    s += 4;
    x0_PSIV.binarySize(s);
  }
  if (x4_PSVM) {
    s += 4;
    x4_PSVM.binarySize(s);
  }
  if (x8_PSOV) {
    s += 4;
    x8_PSOV.binarySize(s);
  }
  if (xc_PSLT) {
    s += 4;
    xc_PSLT.binarySize(s);
  }
  if (x10_PSWT) {
    s += 4;
    x10_PSWT.binarySize(s);
  }
  if (x14_PSTS) {
    s += 4;
    x14_PSTS.binarySize(s);
  }
  if (x18_POFS) {
    s += 4;
    x18_POFS.binarySize(s);
  }
  if (x1c_SEED) {
    s += 4;
    x1c_SEED.binarySize(s);
  }
  if (x20_LENG) {
    s += 4;
    x20_LENG.binarySize(s);
  }
  if (x24_WIDT) {
    s += 4;
    x24_WIDT.binarySize(s);
  }
  if (x28_MAXP) {
    s += 4;
    x28_MAXP.binarySize(s);
  }
  if (x2c_GRTE) {
    s += 4;
    x2c_GRTE.binarySize(s);
  }
  if (x30_COLR) {
    s += 4;
    x30_COLR.binarySize(s);
  }
  if (x34_LTME) {
    s += 4;
    x34_LTME.binarySize(s);
  }
  if (x38_ILOC) {
    s += 4;
    x38_ILOC.binarySize(s);
  }
  if (x3c_IVEC) {
    s += 4;
    x3c_IVEC.binarySize(s);
  }
  if (x40_EMTR) {
    s += 4;
    x40_EMTR.binarySize(s);
  }
  if (x44_24_LINE)
    s += 9;
  if (x44_25_FXLL)
    s += 9;
  if (x44_26_AAPH)
    s += 9;
  if (x44_27_ZBUF)
    s += 9;
  if (x44_28_SORT)
    s += 9;
  if (x44_29_LIT_)
    s += 9;
  if (x44_30_MBLR)
    s += 9;
  if (x44_31_PMAB)
    s += 9;
  if (x45_24_PMUS)
    s += 9;
  if (!x45_25_PMOO)
    s += 9;
  if (x45_26_VMD1)
    s += 9;
  if (x45_27_VMD2)
    s += 9;
  if (x45_28_VMD3)
    s += 9;
  if (x45_29_VMD4)
    s += 9;
  if (x45_30_CIND)
    s += 9;
  if (x45_31_OPTS)
    s += 9;
  if (x30_30_ORNT)
    s += 9;
  if (x30_31_RSOP)
    s += 9;
  if (x48_MBSP) {
    s += 4;
    x48_MBSP.binarySize(s);
  }
  if (x4c_SIZE) {
    s += 4;
    x4c_SIZE.binarySize(s);
  }
  if (x50_ROTA) {
    s += 4;
    x50_ROTA.binarySize(s);
  }
  if (x54_TEXR) {
    s += 4;
    x54_TEXR.binarySize(s);
  }
  if (x58_TIND) {
    s += 4;
    x58_TIND.binarySize(s);
  }
  if (x5c_PMDL) {
    s += 4;
    x5c_PMDL.binarySize(s);
  }
  if (x6c_PMOP) {
    s += 4;
    x6c_PMOP.binarySize(s);
  }
  if (x70_PMRT) {
    s += 4;
    x70_PMRT.binarySize(s);
  }
  if (x74_PMSC) {
    s += 4;
    x74_PMSC.binarySize(s);
  }
  if (x78_PMCL) {
    s += 4;
    x78_PMCL.binarySize(s);
  }
  if (x7c_VEL1) {
    s += 4;
    x7c_VEL1.binarySize(s);
  }
  if (x80_VEL2) {
    s += 4;
    x80_VEL2.binarySize(s);
  }
  if (x84_VEL3) {
    s += 4;
    x84_VEL3.binarySize(s);
  }
  if (x88_VEL4) {
    s += 4;
    x88_VEL4.binarySize(s);
  }
  if (x8c_ICTS) {
    s += 4;
    x8c_ICTS.binarySize(s);
  }
  if (x9c_NCSY) {
    s += 4;
    x9c_NCSY.binarySize(s);
  }
  if (xa0_CSSD) {
    s += 4;
    xa0_CSSD.binarySize(s);
  }
  if (xa4_IDTS) {
    s += 4;
    xa4_IDTS.binarySize(s);
  }
  if (xb4_NDSY) {
    s += 4;
    xb4_NDSY.binarySize(s);
  }
  if (xb8_IITS) {
    s += 4;
    xb8_IITS.binarySize(s);
  }
  if (xc8_PISY) {
    s += 4;
    xc8_PISY.binarySize(s);
  }
  if (xcc_SISY) {
    s += 4;
    xcc_SISY.binarySize(s);
  }
  if (xd0_KSSM) {
    s += 4;
    xd0_KSSM.binarySize(s);
  }
  if (xd4_SSWH) {
    s += 4;
    xd4_SSWH.binarySize(s);
  }
  if (xd8_SELC) {
    s += 4;
    xd8_SELC.binarySize(s);
  }
  if (xe4_SSSD) {
    s += 4;
    xe4_SSSD.binarySize(s);
  }
  if (xe8_SSPO) {
    s += 4;
    xe8_SSPO.binarySize(s);
  }
  if (xf8_SESD) {
    s += 4;
    xf8_SESD.binarySize(s);
  }
  if (xfc_SEPO) {
    s += 4;
    xfc_SEPO.binarySize(s);
  }
  if (xec_PMLC) {
    s += 4;
    xec_PMLC.binarySize(s);
  }
  if (x100_LTYP) {
    s += 4;
    x100_LTYP.binarySize(s);
  }
  if (x104_LCLR) {
    s += 4;
    x104_LCLR.binarySize(s);
  }
  if (x108_LINT) {
    s += 4;
    x108_LINT.binarySize(s);
  }
  if (x10c_LOFF) {
    s += 4;
    x10c_LOFF.binarySize(s);
  }
  if (x110_LDIR) {
    s += 4;
    x110_LDIR.binarySize(s);
  }
  if (x114_LFOT) {
    s += 4;
    x114_LFOT.binarySize(s);
  }
  if (x118_LFOR) {
    s += 4;
    x118_LFOR.binarySize(s);
  }
  if (x11c_LSLA) {
    s += 4;
    x11c_LSLA.binarySize(s);
  }
  if (x10c_ADV1) {
    s += 4;
    x10c_ADV1.binarySize(s);
  }
  if (x110_ADV2) {
    s += 4;
    x110_ADV2.binarySize(s);
  }
  if (x114_ADV3) {
    s += 4;
    x114_ADV3.binarySize(s);
  }
  if (x118_ADV4) {
    s += 4;
    x118_ADV4.binarySize(s);
  }
  if (x11c_ADV5) {
    s += 4;
    x11c_ADV5.binarySize(s);
  }
  if (x120_ADV6) {
    s += 4;
    x120_ADV6.binarySize(s);
  }
  if (x124_ADV7) {
    s += 4;
    x124_ADV7.binarySize(s);
  }
  if (x128_ADV8) {
    s += 4;
    x128_ADV8.binarySize(s);
  }
}

template <class IDType>
void GPSM<IDType>::_read(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  if (clsId != SBIG('GPSM')) {
    LogModule.report(logvisor::Warning, fmt("non GPSM provided to GPSM parser"));
    return;
  }
  clsId.read(r);
  while (clsId != SBIG('_END')) {
    switch (clsId.toUint32()) {
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
      LogModule.report(logvisor::Fatal, fmt("Unknown GPSM class {} @{}"), clsId, r.position());
      break;
    }
    clsId.read(r);
  }
}

template <class IDType>
void GPSM<IDType>::_write(typename Write::StreamT& w) const {
  w.writeBytes((atInt8*)"GPSM", 4);
  if (x0_PSIV) {
    w.writeBytes((atInt8*)"PSIV", 4);
    x0_PSIV.write(w);
  }
  if (x4_PSVM) {
    w.writeBytes((atInt8*)"PSVM", 4);
    x4_PSVM.write(w);
  }
  if (x8_PSOV) {
    w.writeBytes((atInt8*)"PSOV", 4);
    x8_PSOV.write(w);
  }
  if (xc_PSLT) {
    w.writeBytes((atInt8*)"PSLT", 4);
    xc_PSLT.write(w);
  }
  if (x10_PSWT) {
    w.writeBytes((atInt8*)"PSWT", 4);
    x10_PSWT.write(w);
  }
  if (x14_PSTS) {
    w.writeBytes((atInt8*)"PSTS", 4);
    x14_PSTS.write(w);
  }
  if (x18_POFS) {
    w.writeBytes((atInt8*)"POFS", 4);
    x18_POFS.write(w);
  }
  if (x1c_SEED) {
    w.writeBytes((atInt8*)"SEED", 4);
    x1c_SEED.write(w);
  }
  if (x20_LENG) {
    w.writeBytes((atInt8*)"LENG", 4);
    x20_LENG.write(w);
  }
  if (x24_WIDT) {
    w.writeBytes((atInt8*)"WIDT", 4);
    x24_WIDT.write(w);
  }
  if (x28_MAXP) {
    w.writeBytes((atInt8*)"MAXP", 4);
    x28_MAXP.write(w);
  }
  if (x2c_GRTE) {
    w.writeBytes((atInt8*)"GRTE", 4);
    x2c_GRTE.write(w);
  }
  if (x30_COLR) {
    w.writeBytes((atInt8*)"COLR", 4);
    x30_COLR.write(w);
  }
  if (x34_LTME) {
    w.writeBytes((atInt8*)"LTME", 4);
    x34_LTME.write(w);
  }
  if (x38_ILOC) {
    w.writeBytes((atInt8*)"ILOC", 4);
    x38_ILOC.write(w);
  }
  if (x3c_IVEC) {
    w.writeBytes((atInt8*)"IVEC", 4);
    x3c_IVEC.write(w);
  }
  if (x40_EMTR) {
    w.writeBytes((atInt8*)"EMTR", 4);
    x40_EMTR.write(w);
  }
  if (x44_24_LINE) {
    w.writeBytes((atInt8*)"LINECNST\x01", 9);
  }
  if (x44_25_FXLL) {
    w.writeBytes((atInt8*)"FXLLCNST\x01", 9);
  }
  if (x44_26_AAPH) {
    w.writeBytes((atInt8*)"AAPHCNST\x01", 9);
  }
  if (x44_27_ZBUF) {
    w.writeBytes((atInt8*)"ZBUFCNST\x01", 9);
  }
  if (x44_28_SORT) {
    w.writeBytes((atInt8*)"SORTCNST\x01", 9);
  }
  if (x44_29_LIT_) {
    w.writeBytes((atInt8*)"LIT_CNST\x01", 9);
  }
  if (x44_30_MBLR) {
    w.writeBytes((atInt8*)"MBLRCNST\x01", 9);
  }
  if (x44_31_PMAB) {
    w.writeBytes((atInt8*)"PMABCNST\x01", 9);
  }
  if (x45_24_PMUS) {
    w.writeBytes((atInt8*)"PMUSCNST\x01", 9);
  }
  if (!x45_25_PMOO) {
    w.writeBytes((atInt8*)"PMOOCNST\x00", 9);
  }
  if (x45_26_VMD1) {
    w.writeBytes((atInt8*)"VMD1CNST\x01", 9);
  }
  if (x45_27_VMD2) {
    w.writeBytes((atInt8*)"VMD2CNST\x01", 9);
  }
  if (x45_28_VMD3) {
    w.writeBytes((atInt8*)"VMD3CNST\x01", 9);
  }
  if (x45_29_VMD4) {
    w.writeBytes((atInt8*)"VMD4CNST\x01", 9);
  }
  if (x45_30_CIND) {
    w.writeBytes((atInt8*)"CINDCNST\x01", 9);
  }
  if (x45_31_OPTS) {
    w.writeBytes((atInt8*)"OPTSCNST\x01", 9);
  }
  if (x30_30_ORNT) {
    w.writeBytes((atInt8*)"ORNTCNST\x01", 9);
  }
  if (x30_31_RSOP) {
    w.writeBytes((atInt8*)"RSOPCNST\x01", 9);
  }
  if (x48_MBSP) {
    w.writeBytes((atInt8*)"MBSP", 4);
    x48_MBSP.write(w);
  }
  if (x4c_SIZE) {
    w.writeBytes((atInt8*)"SIZE", 4);
    x4c_SIZE.write(w);
  }
  if (x50_ROTA) {
    w.writeBytes((atInt8*)"ROTA", 4);
    x50_ROTA.write(w);
  }
  if (x54_TEXR) {
    w.writeBytes((atInt8*)"TEXR", 4);
    x54_TEXR.write(w);
  }
  if (x58_TIND) {
    w.writeBytes((atInt8*)"TIND", 4);
    x58_TIND.write(w);
  }
  if (x5c_PMDL) {
    w.writeBytes((atInt8*)"PMDL", 4);
    x5c_PMDL.write(w);
  }
  if (x6c_PMOP) {
    w.writeBytes((atInt8*)"PMOP", 4);
    x6c_PMOP.write(w);
  }
  if (x70_PMRT) {
    w.writeBytes((atInt8*)"PMRT", 4);
    x70_PMRT.write(w);
  }
  if (x74_PMSC) {
    w.writeBytes((atInt8*)"PMSC", 4);
    x74_PMSC.write(w);
  }
  if (x78_PMCL) {
    w.writeBytes((atInt8*)"PMCL", 4);
    x78_PMCL.write(w);
  }
  if (x7c_VEL1) {
    w.writeBytes((atInt8*)"VEL1", 4);
    x7c_VEL1.write(w);
  }
  if (x80_VEL2) {
    w.writeBytes((atInt8*)"VEL2", 4);
    x80_VEL2.write(w);
  }
  if (x84_VEL3) {
    w.writeBytes((atInt8*)"VEL3", 4);
    x84_VEL3.write(w);
  }
  if (x88_VEL4) {
    w.writeBytes((atInt8*)"VEL4", 4);
    x88_VEL4.write(w);
  }
  if (x8c_ICTS) {
    w.writeBytes((atInt8*)"ICTS", 4);
    x8c_ICTS.write(w);
  }
  if (x9c_NCSY) {
    w.writeBytes((atInt8*)"NCSY", 4);
    x9c_NCSY.write(w);
  }
  if (xa0_CSSD) {
    w.writeBytes((atInt8*)"CSSD", 4);
    xa0_CSSD.write(w);
  }
  if (xa4_IDTS) {
    w.writeBytes((atInt8*)"IDTS", 4);
    xa4_IDTS.write(w);
  }
  if (xb4_NDSY) {
    w.writeBytes((atInt8*)"NDSY", 4);
    xb4_NDSY.write(w);
  }
  if (xb8_IITS) {
    w.writeBytes((atInt8*)"IITS", 4);
    xb8_IITS.write(w);
  }
  if (xc8_PISY) {
    w.writeBytes((atInt8*)"PISY", 4);
    xc8_PISY.write(w);
  }
  if (xcc_SISY) {
    w.writeBytes((atInt8*)"SISY", 4);
    xcc_SISY.write(w);
  }
  if (xd0_KSSM) {
    w.writeBytes((atInt8*)"KSSM", 4);
    xd0_KSSM.write(w);
  }
  if (xd4_SSWH) {
    w.writeBytes((atInt8*)"SSWH", 4);
    xd4_SSWH.write(w);
  }
  if (xd8_SELC) {
    w.writeBytes((atInt8*)"SELC", 4);
    xd8_SELC.write(w);
  }
  if (xe4_SSSD) {
    w.writeBytes((atInt8*)"SSSD", 4);
    xe4_SSSD.write(w);
  }
  if (xe8_SSPO) {
    w.writeBytes((atInt8*)"SSPO", 4);
    xe8_SSPO.write(w);
  }
  if (xf8_SESD) {
    w.writeBytes((atInt8*)"SESD", 4);
    xf8_SESD.write(w);
  }
  if (xfc_SEPO) {
    w.writeBytes((atInt8*)"SEPO", 4);
    xfc_SEPO.write(w);
  }
  if (xec_PMLC) {
    w.writeBytes((atInt8*)"PMLC", 4);
    xec_PMLC.write(w);
  }
  if (x100_LTYP) {
    w.writeBytes((atInt8*)"LTYP", 4);
    x100_LTYP.write(w);
  }
  if (x104_LCLR) {
    w.writeBytes((atInt8*)"LCLR", 4);
    x104_LCLR.write(w);
  }
  if (x108_LINT) {
    w.writeBytes((atInt8*)"LINT", 4);
    x108_LINT.write(w);
  }
  if (x10c_LOFF) {
    w.writeBytes((atInt8*)"LOFF", 4);
    x10c_LOFF.write(w);
  }
  if (x110_LDIR) {
    w.writeBytes((atInt8*)"LDIR", 4);
    x110_LDIR.write(w);
  }
  if (x114_LFOT) {
    w.writeBytes((atInt8*)"LFOT", 4);
    x114_LFOT.write(w);
  }
  if (x118_LFOR) {
    w.writeBytes((atInt8*)"LFOR", 4);
    x118_LFOR.write(w);
  }
  if (x11c_LSLA) {
    w.writeBytes((atInt8*)"LSLA", 4);
    x11c_LSLA.write(w);
  }
  if (x10c_ADV1) {
    w.writeBytes((atInt8*)"ADV1", 4);
    x10c_ADV1.write(w);
  }
  if (x110_ADV2) {
    w.writeBytes((atInt8*)"ADV2", 4);
    x110_ADV2.write(w);
  }
  if (x114_ADV3) {
    w.writeBytes((atInt8*)"ADV3", 4);
    x114_ADV3.write(w);
  }
  if (x118_ADV4) {
    w.writeBytes((atInt8*)"ADV4", 4);
    x118_ADV4.write(w);
  }
  if (x11c_ADV5) {
    w.writeBytes((atInt8*)"ADV5", 4);
    x11c_ADV5.write(w);
  }
  if (x120_ADV6) {
    w.writeBytes((atInt8*)"ADV6", 4);
    x120_ADV6.write(w);
  }
  if (x124_ADV7) {
    w.writeBytes((atInt8*)"ADV7", 4);
    x124_ADV7.write(w);
  }
  if (x128_ADV8) {
    w.writeBytes((atInt8*)"ADV8", 4);
    x128_ADV8.write(w);
  }
  w.writeBytes("_END", 4);
}

AT_SUBSPECIALIZE_DNA_YAML(GPSM<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(GPSM<UniqueID64>)

template <class IDType>
void GPSM<IDType>::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
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
bool ExtractGPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter writer(outPath.getAbsolutePath());
  if (writer.isOpen()) {
    GPSM<IDType> gpsm;
    gpsm.read(rs);
    athena::io::ToYAMLStream(gpsm, writer);
    return true;
  }
  return false;
}
template bool ExtractGPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractGPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteGPSM(const GPSM<IDType>& gpsm, const hecl::ProjectPath& outPath) {
  athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
  if (w.hasError())
    return false;
  gpsm.write(w);
  int64_t rem = w.position() % 32;
  if (rem)
    for (int64_t i = 0; i < 32 - rem; ++i)
      w.writeUByte(0xff);
  return true;
}
template bool WriteGPSM<UniqueID32>(const GPSM<UniqueID32>& gpsm, const hecl::ProjectPath& outPath);
template bool WriteGPSM<UniqueID64>(const GPSM<UniqueID64>& gpsm, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAParticle
