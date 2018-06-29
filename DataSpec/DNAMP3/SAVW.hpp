#ifndef __DNAMP3_SAVW_HPP__
#define __DNAMP3_SAVW_HPP__

#include "DataSpec/DNACommon/SAVWCommon.hpp"
#include "DNAMP3.hpp"

namespace DataSpec::DNAMP3
{
struct Scan : BigDNA
{
    AT_DECL_DNA_YAML
    UniqueID64 scanId;
    Value<SAVWCommon::EScanCategory> category;
};

struct SavedState : BigDNA
{
    AT_DECL_DNA_YAML
    struct ID : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint64> id[2];
    };
    ID id;
    Value<atUint32> instance;
};

struct SAVW : BigDNA
{
    AT_DECL_DNA_YAML
    SAVWCommon::Header header;
    Value<atUint32> skippableCutsceneCount;
    Vector<SavedState, AT_DNA_COUNT(skippableCutsceneCount)> skippableCutscenes;
    Value<atUint32> relayCount;
    Vector<SavedState, AT_DNA_COUNT(relayCount)> relays;
    Value<atUint32> doorCount;
    Vector<SavedState, AT_DNA_COUNT(doorCount)> doors;
    Value<atUint32> scanCount;
    Vector<Scan, AT_DNA_COUNT(scanCount)> scans;
    Value<atUint32> systemVarCount;
    Vector<SAVWCommon::EnvironmentVariable, AT_DNA_COUNT(systemVarCount)> systemVars;
    Value<atUint32> gameVarCount;
    Vector<SAVWCommon::EnvironmentVariable, AT_DNA_COUNT(gameVarCount)> gameVars;
    Value<atUint32> gameObjectCount;
    Vector<SavedState, AT_DNA_COUNT(gameObjectCount)> gameObjects;
};
}

#endif // __DNAMP3_SAVW_HPP__
