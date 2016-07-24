#ifndef __DNAMP3_SAVW_HPP__
#define __DNAMP3_SAVW_HPP__

#include "../DNACommon/SAVWCommon.hpp"
#include "DNAMP3.hpp"

namespace DataSpec
{
namespace DNAMP3
{
struct Scan : BigYAML
{
    DECL_YAML
    UniqueID64 scanId;
    Value<SAVWCommon::EScanCategory> category;
};

struct SavedState : BigYAML
{
    DECL_YAML
    struct ID : BigYAML
    {
        DECL_YAML
        Value<atUint64> id[2];
    };
    ID id;
    Value<atUint32> instance;
};

struct SAVW : BigYAML
{
    DECL_YAML
    SAVWCommon::Header header;
    Value<atUint32> skippableCutsceneCount;
    Vector<SavedState, DNA_COUNT(skippableCutsceneCount)> skippableCutscenes;
    Value<atUint32> relayCount;
    Vector<SavedState, DNA_COUNT(relayCount)> relays;
    Value<atUint32> layerCount;
    Vector<SAVWCommon::Layer, DNA_COUNT(layerCount)> layers;
    Value<atUint32> doorCount;
    Vector<SavedState, DNA_COUNT(doorCount)> doors;
    Value<atUint32> scanCount;
    Vector<Scan, DNA_COUNT(scanCount)> scans;
    Value<atUint32> systemVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(systemVarCount)> systemVars;
    Value<atUint32> gameVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(gameVarCount)> gameVars;
    Value<atUint32> gameObjectCount;
    Vector<SavedState, DNA_COUNT(gameObjectCount)> gameObjects;
};
}
}

#endif // __DNAMP3_SAVW_HPP__
