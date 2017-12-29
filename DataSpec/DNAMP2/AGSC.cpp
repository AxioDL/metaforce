#include "AGSC.hpp"

namespace DataSpec::DNAMP2
{

bool AGSC::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    Header head;
    head.read(rs);

    {
        hecl::ProjectPath poolPath = outPath.getWithExtension(_S(".pool"), true);
        athena::io::FileWriter w(poolPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(head.poolSz).get(), head.poolSz);
    }

    {
        hecl::ProjectPath projPath = outPath.getWithExtension(_S(".proj"), true);
        athena::io::FileWriter w(projPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(head.projSz).get(), head.projSz);
    }

    {
        hecl::ProjectPath sdirPath = outPath.getWithExtension(_S(".sdir"), true);
        athena::io::FileWriter w(sdirPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(head.sdirSz).get(), head.sdirSz);
    }

    {
        hecl::ProjectPath sampPath = outPath.getWithExtension(_S(".samp"), true);
        athena::io::FileWriter w(sampPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(head.sampSz).get(), head.sampSz);
    }

    return true;
}

bool AGSC::Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath());
    if (w.hasError())
        return false;

    hecl::ProjectPath woExt = inPath.getWithExtension(nullptr, true);
    std::string lastComp = std::string(woExt.getLastComponentUTF8());
    if (hecl::StringUtils::EndsWith(lastComp, "_AGSC"))
        lastComp.assign(lastComp.cbegin(), lastComp.cend() - 5);

    hecl::ProjectPath poolPath = inPath.getWithExtension(_S(".pool"), true);
    athena::io::FileReader poolR(poolPath.getAbsolutePath());
    if (poolR.hasError())
        return false;
    uint32_t poolLen = poolR.length();

    hecl::ProjectPath projPath = inPath.getWithExtension(_S(".proj"), true);
    athena::io::FileReader projR(projPath.getAbsolutePath());
    if (projR.hasError())
        return false;
    uint32_t projLen = projR.length();

    hecl::ProjectPath sdirPath = inPath.getWithExtension(_S(".sdir"), true);
    athena::io::FileReader sdirR(sdirPath.getAbsolutePath());
    if (sdirR.hasError())
        return false;
    uint32_t sdirLen = sdirR.length();

    hecl::ProjectPath sampPath = inPath.getWithExtension(_S(".samp"), true);
    athena::io::FileReader sampR(sampPath.getAbsolutePath());
    if (sampR.hasError())
        return false;
    uint32_t sampLen = sampR.length();

    projR.seek(4, athena::SeekOrigin::Begin);
    uint16_t groupId = projR.readUint16Big();
    projR.seek(0, athena::SeekOrigin::Begin);

    Header head;
    head.groupName = lastComp;
    head.groupId = groupId;
    head.poolSz = poolLen;
    head.projSz = projLen;
    head.sdirSz = sdirLen;
    head.sampSz = sampLen;
    head.write(w);

    w.writeBytes(poolR.readBytes(poolLen).get(), poolLen);
    w.writeBytes(projR.readBytes(projLen).get(), projLen);
    w.writeBytes(sdirR.readBytes(sdirLen).get(), sdirLen);
    w.writeBytes(sampR.readBytes(sampLen).get(), sampLen);

    return true;
}

}
