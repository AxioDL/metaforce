#include "ELSC.hpp"

namespace DataSpec
{
namespace DNAParticle
{
template <class IDType>
void ELSM<IDType>::read(athena::io::YAMLDocReader& r)
{
    for (const auto& elem : r.getCurNode()->m_mapChildren)
    {
        if (elem.first.size() < 4)
        {
            LogModule.report(logvisor::Warning, "short FourCC in element '%s'", elem.first.c_str());
            continue;
        }
        switch(*reinterpret_cast<const uint32_t*>(elem.first.data()))
        {
        case SBIG('LIFE'):
            x0_LIFE.read(r);
            break;
        case SBIG('SLIF'):
            x4_SLIF.read(r);
            break;
        case SBIG('GRAT'):
            x8_GRAT.read(r);
            break;
        case SBIG('SCNT'):
            xc_SCNT.read(r);
            break;
        case SBIG('SSEG'):
            x10_SSEG.read(r);
            break;
        case SBIG('COLR'):
            x14_COLR.read(r);
            break;
        case SBIG('IEMT'):
            x18_IEMT.read(r);
            break;
        case SBIG('FEMT'):
            x1c_FEMT.read(r);
            break;
        case SBIG('AMPL'):
            x20_AMPL.read(r);
            break;
        case SBIG('AMPD'):
            x24_AMPD.read(r);
            break;
        case SBIG('LWD1'):
            x28_LWD1.read(r);
            break;
        case SBIG('LWD2'):
            x2c_LWD2.read(r);
            break;
        case SBIG('LWD3'):
            x30_LWD3.read(r);
            break;
        case SBIG('LCL1'):
            x34_LCL1.read(r);
            break;
        case SBIG('LCL2'):
            x38_LCL2.read(r);
            break;
        case SBIG('LCL3'):
            x3c_LCL3.read(r);
            break;
        case SBIG('SSWH'):
            x40_SSWH.read(r);
            break;
        case SBIG('GPSM'):
            x50_GPSM.read(r);
            break;
        case SBIG('EPSM'):
            x60_EPSM.read(r);
            break;
        case SBIG('ZERY'):
            x70_ZERY.read(r);
            break;
        }
    }
}

template <class IDType>
void ELSM<IDType>::write(athena::io::YAMLDocWriter& w) const
{
    if (x0_LIFE)
    {
        w.enterSubRecord("LIFE");
        x0_LIFE.write(w);
        w.leaveSubRecord();
    }
    if (x4_SLIF)
    {
        w.enterSubRecord("SLIF");
        x4_SLIF.write(w);
        w.leaveSubRecord();
    }
    if (x8_GRAT)
    {
        w.enterSubRecord("GRAT");
        x8_GRAT.write(w);
        w.leaveSubRecord();
    }
    if (xc_SCNT)
    {
        w.enterSubRecord("SCNT");
        xc_SCNT.write(w);
        w.leaveSubRecord();
    }
    if (x10_SSEG)
    {
        w.enterSubRecord("SSEG");
        x10_SSEG.write(w);
        w.leaveSubRecord();
    }
    if (x14_COLR)
    {
        w.enterSubRecord("COLR");
        x14_COLR.write(w);
        w.leaveSubRecord();
    }
    if (x18_IEMT)
    {
        w.enterSubRecord("IEMT");
        x18_IEMT.write(w);
        w.leaveSubRecord();
    }
    if (x1c_FEMT)
    {
        w.enterSubRecord("FEMT");
        x1c_FEMT.write(w);
        w.leaveSubRecord();
    }
    if (x20_AMPL)
    {
        w.enterSubRecord("AMPL");
        x20_AMPL.write(w);
        w.leaveSubRecord();
    }
    if (x24_AMPD)
    {
        w.enterSubRecord("AMPD");
        x24_AMPD.write(w);
        w.leaveSubRecord();
    }
    if (x28_LWD1)
    {
        w.enterSubRecord("LWD1");
        x28_LWD1.write(w);
        w.leaveSubRecord();
    }
    if (x2c_LWD2)
    {
        w.enterSubRecord("LWD2");
        x2c_LWD2.write(w);
        w.leaveSubRecord();
    }
    if (x30_LWD3)
    {
        w.enterSubRecord("LWD3");
        x30_LWD3.write(w);
        w.leaveSubRecord();
    }
    if (x34_LCL1)
    {
        w.enterSubRecord("LCL1");
        x34_LCL1.write(w);
        w.leaveSubRecord();
    }
    if (x38_LCL2)
    {
        w.enterSubRecord("LCL2");
        x38_LCL2.write(w);
        w.leaveSubRecord();
    }
    if (x3c_LCL3)
    {
        w.enterSubRecord("LCL3");
        x3c_LCL3.write(w);
        w.leaveSubRecord();
    }
    if (x40_SSWH)
    {
        w.enterSubRecord("SSWH");
        x40_SSWH.write(w);
        w.leaveSubRecord();
    }
    if (x50_GPSM)
    {
        w.enterSubRecord("GPSM");
        x50_GPSM.write(w);
        w.leaveSubRecord();
    }
    if (x60_EPSM)
    {
        w.enterSubRecord("EPSM");
        x60_EPSM.write(w);
        w.leaveSubRecord();
    }
    if (x70_ZERY)
    {
        w.enterSubRecord("ZERY");
        x70_ZERY.write(w);
        w.leaveSubRecord();
    }
}

template <class IDType>
size_t ELSM<IDType>::binarySize(size_t __isz) const
{
    __isz += 4;
    if (x0_LIFE)
        __isz = x0_LIFE.binarySize(__isz + 4);
    if (x4_SLIF)
        __isz = x4_SLIF.binarySize(__isz + 4);
    if (x8_GRAT)
        __isz = x8_GRAT.binarySize(__isz + 4);
    if (xc_SCNT)
        __isz = xc_SCNT.binarySize(__isz + 4);
    if (x10_SSEG)
        __isz = x10_SSEG.binarySize(__isz + 4);
    if (x14_COLR)
        __isz = x14_COLR.binarySize(__isz + 4);
    if (x18_IEMT)
        __isz = x18_IEMT.binarySize(__isz + 4);
    if (x1c_FEMT)
        __isz = x1c_FEMT.binarySize(__isz + 4);
    if (x20_AMPL)
        __isz = x20_AMPL.binarySize(__isz + 4);
    if (x24_AMPD)
        __isz = x24_AMPD.binarySize(__isz + 4);
    if (x28_LWD1)
        __isz = x28_LWD1.binarySize(__isz + 4);
    if (x2c_LWD2)
        __isz = x2c_LWD2.binarySize(__isz + 4);
    if (x30_LWD3)
        __isz = x30_LWD3.binarySize(__isz + 4);
    if (x34_LCL1)
        __isz = x34_LCL1.binarySize(__isz + 4);
    if (x38_LCL2)
        __isz = x38_LCL2.binarySize(__isz + 4);
    if (x3c_LCL3)
        __isz = x3c_LCL3.binarySize(__isz + 4);
    if (x40_SSWH)
        __isz = x40_SSWH.binarySize(__isz + 4);
    if (x50_GPSM)
        __isz = x50_GPSM.binarySize(__isz + 4);
    if (x60_EPSM)
        __isz = x60_EPSM.binarySize(__isz + 4);
    if (x70_ZERY)
        __isz = x70_ZERY.binarySize(__isz + 4);

    return __isz;
}

template <class IDType>
void ELSM<IDType>::read(athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    if (clsId != SBIG('ELSM'))
    {
        LogModule.report(logvisor::Warning, "non ELSM provided to ELSM parser");
        return;
    }

    r.readBytesToBuf(&clsId, 4);
    while (clsId != SBIG('_END'))
    {
        switch(clsId)
        {
        case SBIG('LIFE'):
            x0_LIFE.read(r);
            break;
        case SBIG('SLIF'):
            x4_SLIF.read(r);
            break;
        case SBIG('GRAT'):
            x8_GRAT.read(r);
            break;
        case SBIG('SCNT'):
            xc_SCNT.read(r);
            break;
        case SBIG('SSEG'):
            x10_SSEG.read(r);
            break;
        case SBIG('COLR'):
            x14_COLR.read(r);
            break;
        case SBIG('IEMT'):
            x18_IEMT.read(r);
            break;
        case SBIG('FEMT'):
            x1c_FEMT.read(r);
            break;
        case SBIG('AMPL'):
            x20_AMPL.read(r);
            break;
        case SBIG('AMPD'):
            x24_AMPD.read(r);
            break;
        case SBIG('LWD1'):
            x28_LWD1.read(r);
            break;
        case SBIG('LWD2'):
            x2c_LWD2.read(r);
            break;
        case SBIG('LWD3'):
            x30_LWD3.read(r);
            break;
        case SBIG('LCL1'):
            x34_LCL1.read(r);
            break;
        case SBIG('LCL2'):
            x38_LCL2.read(r);
            break;
        case SBIG('LCL3'):
            x3c_LCL3.read(r);
            break;
        case SBIG('SSWH'):
            x40_SSWH.read(r);
            break;
        case SBIG('GPSM'):
            x50_GPSM.read(r);
            break;
        case SBIG('EPSM'):
            x60_EPSM.read(r);
            break;
        case SBIG('ZERY'):
            x70_ZERY.read(r);
            break;
        default:
            LogModule.report(logvisor::Fatal, "Unknown ELSM class %.4s @%" PRIi64, &clsId, r.position());
            break;
        }
        r.readBytesToBuf(&clsId, 4);
    }
}

template <class IDType>
void ELSM<IDType>::write(athena::io::IStreamWriter& w) const
{
    w.writeBytes((atInt8*)"ELSM", 4);
    if (x0_LIFE)
    {
        w.writeBytes((atInt8*)"LIFE", 4);
        x0_LIFE.write(w);
    }
    if (x4_SLIF)
    {
        w.writeBytes((atInt8*)"SLIF", 4);
        x4_SLIF.write(w);
    }
    if (x8_GRAT)
    {
        w.writeBytes((atInt8*)"GRAT", 4);
        x8_GRAT.write(w);
    }
    if (xc_SCNT)
    {
        w.writeBytes((atInt8*)"SCNT", 4);
        xc_SCNT.write(w);
    }
    if (x10_SSEG)
    {
        w.writeBytes((atInt8*)"SSEG", 4);
        x10_SSEG.write(w);
    }
    if (x14_COLR)
    {
        w.writeBytes((atInt8*)"COLR", 4);
        x14_COLR.write(w);
    }
    if (x18_IEMT)
    {
        w.writeBytes((atInt8*)"IEMT", 4);
        x18_IEMT.write(w);
    }
    if (x1c_FEMT)
    {
        w.writeBytes((atInt8*)"FEMT", 4);
        x1c_FEMT.write(w);
    }
    if (x20_AMPL)
    {
        w.writeBytes((atInt8*)"AMPL", 4);
        x20_AMPL.write(w);
    }
    if (x24_AMPD)
    {
        w.writeBytes((atInt8*)"AMPD", 4);
        x24_AMPD.write(w);
    }
    if (x28_LWD1)
    {
        w.writeBytes((atInt8*)"LWD1", 4);
        x28_LWD1.write(w);
    }
    if (x2c_LWD2)
    {
        w.writeBytes((atInt8*)"LWD2", 4);
        x2c_LWD2.write(w);
    }
    if (x30_LWD3)
    {
        w.writeBytes((atInt8*)"LWD3", 4);
        x30_LWD3.write(w);
    }
    if (x34_LCL1)
    {
        w.writeBytes((atInt8*)"LCL1", 4);
        x34_LCL1.write(w);
    }
    if (x38_LCL2)
    {
        w.writeBytes((atInt8*)"LCL2", 4);
        x38_LCL2.write(w);
    }
    if (x3c_LCL3)
    {
        w.writeBytes((atInt8*)"LCL3", 4);
        x3c_LCL3.write(w);
    }
    if (x40_SSWH)
    {
        w.writeBytes((atInt8*)"SSWH", 4);
        x40_SSWH.write(w);
    }
    if (x50_GPSM)
    {
        w.writeBytes((atInt8*)"GPSM", 4);
        x50_GPSM.write(w);
    }
    if (x60_EPSM)
    {
        w.writeBytes((atInt8*)"EPSM", 4);
        x60_EPSM.write(w);
    }
    if (x70_ZERY)
    {
        w.writeBytes((atInt8*)"ZERY", 4);
        x70_ZERY.write(w);
    }
    w.writeBytes("_END", 4);
}

template struct ELSM<UniqueID32>;
template struct ELSM<UniqueID64>;

template <class IDType>
bool ExtractELSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    FILE* fp = hecl::Fopen(outPath.getAbsolutePath().c_str(), _S("w"));
    if (fp)
    {
        ELSM<IDType> elsm;
        elsm.read(rs);
        elsm.toYAMLFile(fp);
        fclose(fp);
        return true;
    }
    return false;
}
template bool ExtractELSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractELSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteELSM(const ELSM<IDType>& elsm, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    elsm.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeBytes((atInt8*)"\xff", 1);
    return true;
}
template bool WriteELSM<UniqueID32>(const ELSM<UniqueID32>& gpsm, const hecl::ProjectPath& outPath);
template bool WriteELSM<UniqueID64>(const ELSM<UniqueID64>& gpsm, const hecl::ProjectPath& outPath);

}
}
