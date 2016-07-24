#ifndef __DNAMP1_SAVW_HPP__
#define __DNAMP1_SAVW_HPP__

#include "../DNACommon/SAVWCommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Scan : BigYAML
{
    DECL_YAML
    UniqueID32 scanId;
    Value<SAVWCommon::EScanCategory> category;
};

struct SAVW : BigYAML
{
    DECL_YAML
    SAVWCommon::Header header;
    Value<atUint32> skippableCutsceneCount;
    Vector<atUint32, DNA_COUNT(skippableCutsceneCount)> skippableCutscenes;
    Value<atUint32> relayCount;
    Vector<atUint32, DNA_COUNT(relayCount)> relays;
    Value<atUint32> layerCount;
    Vector<SAVWCommon::Layer, DNA_COUNT(layerCount)> layers;
    Value<atUint32> doorCount;
    Vector<atUint32, DNA_COUNT(doorCount)> doors;
    Value<atUint32> scanCount;
    Vector<Scan, DNA_COUNT(scanCount)> scans;
};

static bool ExtractSAVW(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    SAVW savw;
    savw.read(rs);
    FILE* fp = hecl::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
    savw.toYAMLFile(fp);
    fclose(fp);
    return true;
}
}
}

#endif // __DNAMP1_SAVW_HPP__
