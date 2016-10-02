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

    Scan() = default;
    Scan(const UniqueID32& id) : scanId(id), category(SAVWCommon::EScanCategory::None) {}
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
}
}

#endif // __DNAMP1_SAVW_HPP__
