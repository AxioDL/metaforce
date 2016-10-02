#include "DPSC.hpp"

namespace DataSpec
{
namespace DNAParticle
{
template <class IDType>
void DPSM<IDType>::read(athena::io::YAMLDocReader& r)
{
    for (const auto& elem : r.getCurNode()->m_mapChildren)
    {
        if (elem.first.size() < 4)
        {
            LogModule.report(logvisor::Warning, "short FourCC in element '%s'", elem.first.c_str());
            continue;
        }

        r.enterSubRecord(elem.first.c_str());
        bool loadFirstDesc = false;
        uint32_t clsId = *reinterpret_cast<const uint32_t*>(elem.first.c_str());
        switch(clsId)
        {
        case SBIG('1SZE'):
        case SBIG('1LFT'):
        case SBIG('1ROT'):
        case SBIG('1OFF'):
        case SBIG('1CLR'):
        case SBIG('1TEX'):
        case SBIG('1ADD'):
            loadFirstDesc = true;
        case SBIG('2SZE'):
        case SBIG('2LFT'):
        case SBIG('2ROT'):
        case SBIG('2OFF'):
        case SBIG('2CLR'):
        case SBIG('2TEX'):
        case SBIG('2ADD'):
            if (loadFirstDesc)
                readQuadDecalInfo(r, clsId, x0_quad);
            else
                readQuadDecalInfo(r, clsId, x1c_quad);
        break;
        case SBIG('DMDL'):
            x38_DMDL.read(r);
        break;
        case SBIG('DLFT'):
            x48_DLFT.read(r);
        break;
        case SBIG('DMOP'):
            x4c_DMOP.read(r);
        break;
        case SBIG('DMRT'):
            x50_DMRT.read(r);
        break;
        case SBIG('DMSC'):
            x54_DMSC.read(r);
        break;
        case SBIG('DMCL'):
            x58_DMCL.read(r);
        break;
        case SBIG('DMAB'):
            x5c_24_DMAB = r.readBool(nullptr);
        break;
        case SBIG('DMOO'):
            x5c_25_DMOO = r.readBool(nullptr);
        break;
        }

        r.leaveSubRecord();
    }
}

template <class IDType>
void DPSM<IDType>::write(athena::io::YAMLDocWriter& w) const
{
    writeQuadDecalInfo(w, x0_quad, true);
    writeQuadDecalInfo(w, x1c_quad, false);

    if (x38_DMDL)
    {
        w.enterSubRecord("DMDL");
        x38_DMDL.write(w);
        w.leaveSubRecord();
    }
    if (x48_DLFT)
    {
        w.enterSubRecord("DLFT");
        x48_DLFT.write(w);
        w.leaveSubRecord();
    }
    if (x4c_DMOP)
    {
        w.enterSubRecord("DMOP");
        x4c_DMOP.write(w);
        w.leaveSubRecord();
    }
    if (x50_DMRT)
    {
        w.enterSubRecord("DMRT");
        x50_DMRT.write(w);
        w.leaveSubRecord();
    }
    if (x54_DMSC)
    {
        w.enterSubRecord("DMSC");
        x54_DMSC.write(w);
        w.leaveSubRecord();
    }
    if (x58_DMCL)
    {
        w.enterSubRecord("DMCL");
        x54_DMSC.write(w);
        w.leaveSubRecord();
    }
    if (x5c_24_DMAB)
        w.writeBool("DMAB", x5c_24_DMAB);
    if (x5c_25_DMOO)
        w.writeBool("DMOO", x5c_25_DMOO);
}

template <class IDType>
template <class Reader>
void DPSM<IDType>::readQuadDecalInfo(Reader& r, uint32_t clsId, typename DPSM<IDType>::SQuadDescr& quad)
{
    switch(clsId)
    {
    case SBIG('1LFT'):
    case SBIG('2LFT'):
        quad.x0_LFT.read(r);
    break;
    case SBIG('1SZE'):
    case SBIG('2SZE'):
        quad.x4_SZE.read(r);
    break;
    case SBIG('1ROT'):
    case SBIG('2ROT'):
        quad.x8_ROT.read(r);
    break;
    case SBIG('1OFF'):
    case SBIG('2OFF'):
        quad.xc_OFF.read(r);
    break;
    case SBIG('1CLR'):
    case SBIG('2CLR'):
        quad.x10_CLR.read(r);
    break;
    case SBIG('1TEX'):
    case SBIG('2TEX'):
        quad.x14_TEX.read(r);
    break;
    case SBIG('1ADD'):
    case SBIG('2ADD'):
        quad.x18_ADD.read(r);
    break;
    }
}

template <class IDType>
void DPSM<IDType>::writeQuadDecalInfo(athena::io::YAMLDocWriter& w,
                                      const typename DPSM<IDType>::SQuadDescr& quad, bool first) const
{
    if (quad.x0_LFT)
    {
        w.enterSubRecord((first ? "1LFT" : "2LFT"));
        quad.x0_LFT.write(w);
        w.leaveSubRecord();
    }

    if (quad.x4_SZE)
    {
        w.enterSubRecord((first ? "1SZE" : "2SZE"));
        quad.x4_SZE.write(w);
        w.leaveSubRecord();
    }
    if (quad.x8_ROT)
    {
        w.enterSubRecord((first ? "1ROT" : "2ROT"));
        quad.x8_ROT.write(w);
        w.leaveSubRecord();
    }
    if (quad.xc_OFF)
    {
        w.enterSubRecord((first ? "1OFF" : "2OFF"));
        quad.xc_OFF.write(w);
        w.leaveSubRecord();
    }
    if (quad.x10_CLR)
    {
        w.enterSubRecord((first ? "1CLR" : "2CLR"));
        quad.x10_CLR.write(w);
        w.leaveSubRecord();
    }
    if (quad.x14_TEX)
    {
        w.enterSubRecord((first ? "1TEX" : "2TEX"));
        quad.x14_TEX.write(w);
        w.leaveSubRecord();
    }
    if (quad.x18_ADD)
    {
        w.enterSubRecord((first ? "1ADD" : "2ADD"));
        quad.x18_ADD.write(w);
        w.leaveSubRecord();
    }
}

template <class IDType>
size_t DPSM<IDType>::binarySize(size_t __isz) const
{
    __isz += 4;
    __isz = getQuadDecalBinarySize(__isz, x0_quad);
    __isz = getQuadDecalBinarySize(__isz, x1c_quad);
    if (x38_DMDL)
        __isz = x38_DMDL.binarySize(__isz + 4);
    if (x48_DLFT)
        __isz = x48_DLFT.binarySize(__isz + 4);
    if (x4c_DMOP)
        __isz = x4c_DMOP.binarySize(__isz + 4);
    if (x50_DMRT)
        __isz = x50_DMRT.binarySize(__isz + 4);
    if (x54_DMSC)
        __isz = x54_DMSC.binarySize(__isz + 4);
    if (x58_DMCL)
        __isz = x58_DMCL.binarySize(__isz + 4);
    if (x5c_24_DMAB)
        __isz += 9;
    if (x5c_25_DMOO)
        __isz += 9;
    return __isz;
}

template <class IDType>
size_t DPSM<IDType>::getQuadDecalBinarySize(size_t __isz, const typename DPSM<IDType>::SQuadDescr& quad) const
{
    if (quad.x0_LFT)
        __isz = quad.x0_LFT.binarySize(__isz + 4);
    if (quad.x4_SZE)
        __isz = quad.x4_SZE.binarySize(__isz + 4);
    if (quad.x8_ROT)
        __isz = quad.x8_ROT.binarySize(__isz + 4);
    if (quad.xc_OFF)
        __isz = quad.xc_OFF.binarySize(__isz + 4);
    if (quad.x10_CLR)
        __isz = quad.x10_CLR.binarySize(__isz + 4);
    if (quad.x14_TEX)
        __isz = quad.x14_TEX.binarySize(__isz + 4);
    if (quad.x18_ADD)
        __isz = quad.x18_ADD.binarySize(__isz + 4);

    return __isz;
}

template <class IDType>
void DPSM<IDType>::read(athena::io::IStreamReader& r)
{
    uint32_t clsId;
    r.readBytesToBuf(&clsId, 4);
    if (clsId != SBIG('DPSM'))
    {
        LogModule.report(logvisor::Warning, "non DPSM provided to DPSM parser");
        return;
    }
    bool loadFirstDesc = false;
    r.readBytesToBuf(&clsId, 4);
    while (clsId != SBIG('_END'))
    {
        switch(clsId)
        {
        case SBIG('1SZE'):
        case SBIG('1LFT'):
        case SBIG('1ROT'):
        case SBIG('1OFF'):
        case SBIG('1CLR'):
        case SBIG('1TEX'):
        case SBIG('1ADD'):
            loadFirstDesc = true;
        case SBIG('2SZE'):
        case SBIG('2LFT'):
        case SBIG('2ROT'):
        case SBIG('2OFF'):
        case SBIG('2CLR'):
        case SBIG('2TEX'):
        case SBIG('2ADD'):
            if (loadFirstDesc)
                readQuadDecalInfo(r, clsId, x0_quad);
            else
                readQuadDecalInfo(r, clsId, x1c_quad);
        break;
        case SBIG('DMDL'):
            x38_DMDL.read(r);
        break;
        case SBIG('DLFT'):
            x48_DLFT.read(r);
        break;
        case SBIG('DMOP'):
            x4c_DMOP.read(r);
        break;
        case SBIG('DMRT'):
            x50_DMRT.read(r);
        break;
        case SBIG('DMSC'):
            x54_DMSC.read(r);
        break;
        case SBIG('DMCL'):
            x58_DMCL.read(r);
        break;
        case SBIG('DMAB'):
            r.readUint32();
            x5c_24_DMAB = r.readBool();
        break;
        case SBIG('DMOO'):
            r.readUint32();
            x5c_25_DMOO = r.readBool();
        break;
        default:
            LogModule.report(logvisor::Fatal, "Unknown DPSM class %.4s @%" PRIi64, &clsId, r.position());
        break;
        }
        r.readBytesToBuf(&clsId, 4);
    }
}

template <class IDType>
void DPSM<IDType>::write(athena::io::IStreamWriter& w) const
{
    w.writeBytes("DPSM", 4);
    writeQuadDecalInfo(w, x0_quad, true);
    writeQuadDecalInfo(w, x1c_quad, false);
    if (x38_DMDL)
    {
        w.writeBytes("DMDL", 4);
        x38_DMDL.write(w);
    }
    if (x48_DLFT)
    {
        w.writeBytes("DLFT", 4);
        x48_DLFT.write(w);
    }
    if (x4c_DMOP)
    {
        w.writeBytes("DMOP", 4);
        x4c_DMOP.write(w);
    }
    if (x50_DMRT)
    {
        w.writeBytes("DMRT", 4);
        x50_DMRT.write(w);
    }
    if (x54_DMSC)
    {
        w.writeBytes("DMSC", 4);
        x54_DMSC.write(w);
    }
    if (x58_DMCL)
    {
        w.writeBytes("DMCL", 4);
        x58_DMCL.write(w);
    }
    if (x5c_24_DMAB)
        w.writeBytes("DMABCNST\x01", 9);
    if (x5c_25_DMOO)
        w.writeBytes("DMOOCNST\x01", 9);
    w.writeBytes("_END", 4);
}

template <class IDType>
void DPSM<IDType>::writeQuadDecalInfo(athena::io::IStreamWriter& w,
                                      const typename DPSM<IDType>::SQuadDescr& quad, bool first) const
{
    if (quad.x0_LFT)
    {
        w.writeBytes((first ? "1LFT" : "2LFT"), 4);
        quad.x0_LFT.write(w);
    }
    if (quad.x4_SZE)
    {
        w.writeBytes((first ? "1SZE" : "2SZE"), 4);
        quad.x4_SZE.write(w);
    }
    if (quad.x8_ROT)
    {
        w.writeBytes((first ? "1ROT" : "2ROT"), 4);
        quad.x8_ROT.write(w);
    }
    if (quad.xc_OFF)
    {
        w.writeBytes((first ? "1OFF" : "2OFF"), 4);
        quad.xc_OFF.write(w);
    }
    if (quad.x10_CLR)
    {
        w.writeBytes((first ? "1CLR" : "2LCR"), 4);
        quad.x10_CLR.write(w);
    }
    if (quad.x14_TEX)
    {
        w.writeBytes((first ? "1TEX" : "2TEX"), 4);
        quad.x14_TEX.write(w);
    }
    if (quad.x18_ADD)
    {
        w.writeBytes((first ? "1ADD" : "2ADD"), 4);
        quad.x18_ADD.write(w);
    }
}

template <class IDType>
void DPSM<IDType>::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
{
    if (x0_quad.x14_TEX.m_elem)
        x0_quad.x14_TEX.m_elem->gatherDependencies(pathsOut);
    if (x1c_quad.x14_TEX.m_elem)
        x1c_quad.x14_TEX.m_elem->gatherDependencies(pathsOut);
    g_curSpec->flattenDependencies(x38_DMDL.id, pathsOut);
}

template struct DPSM<UniqueID32>;
template struct DPSM<UniqueID64>;

template <class IDType>
bool ExtractDPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        DPSM<IDType> dpsm;
        dpsm.read(rs);
        dpsm.toYAMLStream(writer);
        return true;
    }
    return false;
}
template bool ExtractDPSM<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractDPSM<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDPSM(const DPSM<IDType>& dpsm, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    dpsm.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeBytes((atInt8*)"\xff", 1);
    return true;
}
template bool WriteDPSM<UniqueID32>(const DPSM<UniqueID32>& dpsm, const hecl::ProjectPath& outPath);
template bool WriteDPSM<UniqueID64>(const DPSM<UniqueID64>& dpsm, const hecl::ProjectPath& outPath);


}
}
