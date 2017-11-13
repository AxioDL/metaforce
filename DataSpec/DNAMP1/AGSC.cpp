#include "AGSC.hpp"

namespace DataSpec
{
namespace DNAMP1
{

using namespace std::literals;

bool AGSC::Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    Header head;
    head.read(rs);

    {
        hecl::ProjectPath poolPath = outPath.getWithExtension(_S(".pool"), true);
        uint32_t poolLen = rs.readUint32Big();
        athena::io::FileWriter w(poolPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(poolLen).get(), poolLen);
    }

    {
        hecl::ProjectPath projPath = outPath.getWithExtension(_S(".proj"), true);
        uint32_t projLen = rs.readUint32Big();
        athena::io::FileWriter w(projPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(projLen).get(), projLen);
    }

    {
        hecl::ProjectPath sampPath = outPath.getWithExtension(_S(".samp"), true);
        uint32_t sampLen = rs.readUint32Big();
        athena::io::FileWriter w(sampPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(sampLen).get(), sampLen);
    }

    {
        hecl::ProjectPath sdirPath = outPath.getWithExtension(_S(".sdir"), true);
        uint32_t sdirLen = rs.readUint32Big();
        athena::io::FileWriter w(sdirPath.getAbsolutePath());
        w.writeBytes(rs.readBytes(sdirLen).get(), sdirLen);
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

    Header head;
    head.audioDir = "Audio/"sv;
    head.groupName = lastComp;
    head.write(w);

    {
        hecl::ProjectPath poolPath = inPath.getWithExtension(_S(".pool"), true);
        athena::io::FileReader r(poolPath.getAbsolutePath());
        if (r.hasError())
            return false;
        uint32_t poolLen = r.length();
        w.writeUint32Big(poolLen);
        w.writeBytes(r.readBytes(poolLen).get(), poolLen);
    }

    {
        hecl::ProjectPath projPath = inPath.getWithExtension(_S(".proj"), true);
        athena::io::FileReader r(projPath.getAbsolutePath());
        if (r.hasError())
            return false;
        uint32_t projLen = r.length();
        w.writeUint32Big(projLen);
        w.writeBytes(r.readBytes(projLen).get(), projLen);
    }

    {
        hecl::ProjectPath sampPath = inPath.getWithExtension(_S(".samp"), true);
        athena::io::FileReader r(sampPath.getAbsolutePath());
        if (r.hasError())
            return false;
        uint32_t sampLen = r.length();
        w.writeUint32Big(sampLen);
        w.writeBytes(r.readBytes(sampLen).get(), sampLen);
    }

    {
        hecl::ProjectPath sdirPath = inPath.getWithExtension(_S(".sdir"), true);
        athena::io::FileReader r(sdirPath.getAbsolutePath());
        if (r.hasError())
            return false;
        uint32_t sdirLen = r.length();
        w.writeUint32Big(sdirLen);
        w.writeBytes(r.readBytes(sdirLen).get(), sdirLen);
    }

    return true;
}

}
}
