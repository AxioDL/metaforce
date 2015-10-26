#include <Athena/FileWriter.hpp>
#include <lzo/lzo1x.h>
#include "MREA.hpp"
#include "../DNAMP1/MREA.hpp"
#include "DeafBabe.hpp"

namespace Retro
{
namespace DNAMP2
{

void MREA::StreamReader::nextBlock()
{
    if (m_nextBlk >= m_blkCount)
        Log.report(LogVisor::FatalError, "MREA stream overrun");

    BlockInfo& info = m_blockInfos[m_nextBlk++];

    /* Reallocate read buffer if needed */
    if (info.bufSize > m_compBufSz)
    {
        m_compBufSz = info.bufSize;
        m_compBuf.reset(new atUint8[m_compBufSz]);
    }

    /* Reallocate decompress buffer if needed */
    if (info.decompSize > m_decompBufSz)
    {
        m_decompBufSz = info.decompSize;
        m_decompBuf.reset(new atUint8[m_decompBufSz]);
    }

    if (info.compSize == 0)
    {
        /* Read uncompressed block */
        m_source.readUBytesToBuf(m_decompBuf.get(), info.decompSize);
    }
    else
    {
        /* Read compressed segments */
        atUint32 blockStart = ROUND_UP_32(info.compSize) - info.compSize;
        m_source.seek(blockStart);
        atUint32 rem = info.decompSize;
        atUint8* bufCur = m_decompBuf.get();
        while (rem)
        {
            atInt16 chunkSz = m_source.readInt16Big();
            if (chunkSz < 0)
            {
                chunkSz = -chunkSz;
                m_source.readUBytesToBuf(bufCur, chunkSz);
                bufCur += chunkSz;
                rem -= chunkSz;
            }
            else
            {
                m_source.readUBytesToBuf(m_compBuf.get(), chunkSz);
                lzo_uint dsz = rem;
                lzo1x_decompress(m_compBuf.get(), chunkSz, bufCur, &dsz, nullptr);
                bufCur += dsz;
                rem -= dsz;
            }
        }
    }

    m_posInBlk = 0;
    m_blkSz = info.decompSize;
}

MREA::StreamReader::StreamReader(Athena::io::IStreamReader& source, atUint32 blkCount)
: m_compBufSz(0x4120), m_compBuf(new atUint8[0x4120]),
  m_decompBufSz(0x4120), m_decompBuf(new atUint8[0x4120]),
  m_source(source), m_blkCount(blkCount)
{
    m_blockInfos.reserve(blkCount);
    for (atUint32 i=0 ; i<blkCount ; ++i)
    {
        m_blockInfos.emplace_back();
        BlockInfo& info = m_blockInfos.back();
        info.read(source);
        m_totalDecompLen += info.decompSize;
    }
    source.seekAlign32();
    m_blkBase = source.position();
    nextBlock();
}

void MREA::StreamReader::seek(atInt64 diff, Athena::SeekOrigin whence)
{
    atUint64 target = diff;
    if (whence == Athena::Current)
        target = m_pos + diff;
    else if (whence == Athena::End)
        target = m_totalDecompLen - diff;

    if (target >= m_totalDecompLen)
        Log.report(LogVisor::FatalError, "MREA stream seek overrun");

    /* Determine which block contains position */
    atUint32 dAccum = 0;
    atUint32 cAccum = 0;
    atUint32 bIdx = 0;
    for (BlockInfo& info : m_blockInfos)
    {
        atUint32 newAccum = dAccum + info.decompSize;
        if (newAccum > target)
            break;
        dAccum = newAccum;
        ++bIdx;
        if (info.compSize)
            cAccum += ROUND_UP_32(info.compSize);
        else
            cAccum += info.decompSize;
    }

    /* Seek source if needed */
    if (bIdx != m_nextBlk-1)
    {
        m_source.seek(m_blkBase + cAccum, Athena::Begin);
        m_nextBlk = bIdx;
        nextBlock();
    }

    m_pos = target;
    m_posInBlk = target - dAccum;
}

atUint64 MREA::StreamReader::readUBytesToBuf(void* buf, atUint64 len)
{
    atUint8* bufCur = reinterpret_cast<atUint8*>(buf);
    atUint64 rem = len;
    while (rem)
    {
        atUint64 lRem = rem;
        atUint64 blkRem = m_blkSz - m_posInBlk;
        if (lRem > blkRem)
            lRem = blkRem;
        memcpy(bufCur, &m_decompBuf[m_posInBlk], lRem);
        bufCur += lRem;
        rem -= lRem;
        m_posInBlk += lRem;
        m_pos += lRem;
        if (rem)
            nextBlock();
    }
    return len;
}

void MREA::StreamReader::writeDecompInfos(Athena::io::IStreamWriter& writer) const
{
    for (const BlockInfo& info : m_blockInfos)
    {
        BlockInfo modInfo = info;
        modInfo.compSize = 0;
        modInfo.write(writer);
    }
}

bool MREA::Extract(const SpecBase& dataSpec,
                   PAKEntryReadStream& rs,
                   const HECL::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter,
                   const DNAMP1::PAK::Entry& entry,
                   bool force,
                   std::function<void(const HECL::SystemChar*)>)
{
    using RigPair = std::pair<CSKR*, CINF*>;
    RigPair dummy(nullptr, nullptr);

    /* Rename MREA for consistency */
    HECL::ProjectPath mreaPath(outPath.getParentPath(), _S("!area.blend"));
    if (!force && mreaPath.getPathType() == HECL::ProjectPath::PT_FILE)
        return true;

    /* Do extract */
    Header head;
    head.read(rs);
    rs.seekAlign32();

    /* MREA decompression stream */
    StreamReader drs(rs, head.compressedBlockCount);
    Athena::io::FileWriter mreaDecompOut(pakRouter.getCooked(&entry).getWithExtension(_S(".decomp")).getAbsolutePath());
    head.write(mreaDecompOut);
    mreaDecompOut.seekAlign32();
    drs.writeDecompInfos(mreaDecompOut);
    mreaDecompOut.seekAlign32();
    atUint64 decompLen = drs.length();
    mreaDecompOut.writeBytes(drs.readBytes(decompLen).get(), decompLen);
    mreaDecompOut.close();
    drs.seek(0, Athena::Begin);

    /* Start up blender connection */
    HECL::BlenderConnection& conn = HECL::BlenderConnection::SharedConnection();
    if (!conn.createBlend(mreaPath, HECL::BlenderConnection::TypeArea))
        return false;

    /* Open Py Stream and read sections */
    HECL::BlenderConnection::PyOutStream os = conn.beginPythonOut(true);
    os.format("import bpy\n"
              "import bmesh\n"
              "from mathutils import Vector\n"
              "\n"
              "bpy.context.scene.name = '%s'\n",
              pakRouter.getBestEntryName(entry).c_str());
    DNACMDL::InitGeomBlenderContext(os, dataSpec.getMasterShaderPath());
    MaterialSet::RegisterMaterialProps(os);
    os << "# Clear Scene\n"
          "for ob in bpy.data.objects:\n"
          "    bpy.context.scene.objects.unlink(ob)\n"
          "    bpy.data.objects.remove(ob)\n"
          "bpy.types.Lamp.retro_layer = bpy.props.IntProperty(name='Retro: Light Layer')\n"
          "bpy.types.Lamp.retro_origtype = bpy.props.IntProperty(name='Retro: Original Type')\n"
          "bpy.types.Object.retro_disable_enviro_visor = bpy.props.BoolProperty(name='Retro: Disable in Combat/Scan Visor')\n"
          "bpy.types.Object.retro_disable_thermal_visor = bpy.props.BoolProperty(name='Retro: Disable in Thermal Visor')\n"
          "bpy.types.Object.retro_disable_xray_visor = bpy.props.BoolProperty(name='Retro: Disable in X-Ray Visor')\n"
          "bpy.types.Object.retro_thermal_level = bpy.props.EnumProperty(items=[('COOL', 'Cool', 'Cool Temperature'),"
                                                                               "('HOT', 'Hot', 'Hot Temperature'),"
                                                                               "('WARM', 'Warm', 'Warm Temperature')],"
                                                                        "name='Retro: Thermal Visor Level')\n"
          "\n";

    /* One shared material set for all meshes */
    os << "# Materials\n"
          "materials = []\n"
          "\n";
    MaterialSet matSet;
    atUint64 secStart = drs.position();
    matSet.read(drs);
    matSet.readToBlender(os, pakRouter, entry, 0);
    drs.seek(secStart + head.secSizes[0], Athena::Begin);
    std::vector<DNACMDL::VertexAttributes> vertAttribs;
    DNACMDL::GetVertexAttributes(matSet, vertAttribs);

    /* Read meshes */
    atUint32 curSec = 1;
    for (atUint32 m=0 ; m<head.meshCount ; ++m)
    {
        MeshHeader mHeader;
        secStart = drs.position();
        mHeader.read(drs);
        drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);
        curSec += DNACMDL::ReadGeomSectionsToBlender<PAKRouter<PAKBridge>, MaterialSet, RigPair, DNACMDL::SurfaceHeader_1_2>
                      (os, drs, pakRouter, entry, dummy, true,
                       true, vertAttribs, m, head.secCount, 0, &head.secSizes[curSec]);
        os.format("obj.retro_disable_enviro_visor = %s\n"
                  "obj.retro_disable_thermal_visor = %s\n"
                  "obj.retro_disable_xray_visor = %s\n"
                  "obj.retro_thermal_level = '%s'\n",
                  mHeader.visorFlags.disableEnviro() ? "True" : "False",
                  mHeader.visorFlags.disableThermal() ? "True" : "False",
                  mHeader.visorFlags.disableXray() ? "True" : "False",
                  mHeader.visorFlags.thermalLevelStr());

        /* Seek through AROT-relation sections */
        drs.seek(head.secSizes[curSec++], Athena::Current);
        drs.seek(head.secSizes[curSec++], Athena::Current);
    }

    /* Skip AROT */
    drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Skip BVH */
    drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Skip Bitmap */
    drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Skip SCLY (for now) */
    for (atUint32 l=0 ; l<head.sclyLayerCount ; ++l)
        drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Skip SCGN (for now) */
    drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Read collision meshes */
    DeafBabe collision;
    secStart = drs.position();
    collision.read(drs);
    DeafBabe::BlenderInit(os);
    collision.sendToBlender(os);
    drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

    /* Skip unknown section */
    drs.seek(head.secSizes[curSec++], Athena::Current);

    /* Read BABEDEAD Lights as Cycles emissives */
    secStart = drs.position();
    DNAMP1::MREA::ReadBabeDeadToBlender_1_2(os, drs);
    drs.seek(secStart + head.secSizes[curSec++], Athena::Begin);

    /* Origins to center of mass */
    os << "bpy.context.scene.layers[1] = True\n"
          "bpy.ops.object.select_by_type(type='MESH')\n"
          "bpy.ops.object.origin_set(type='ORIGIN_CENTER_OF_MASS')\n"
          "bpy.ops.object.select_all(action='DESELECT')\n"
          "bpy.context.scene.layers[1] = False\n";

    os.centerView();
    os.close();
    return conn.saveBlend();
}

}
}
