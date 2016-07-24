#ifndef __DNAMP2_SAVW_HPP__
#define __DNAMP2_SAVW_HPP__

#include "../DNAMP1/SAVW.hpp"
#include "DNAMP2.hpp"

namespace DataSpec
{
namespace DNAMP2
{
struct SAVW : DNAMP1::SAVW
{
    DECL_YAML
    Value<atUint32> systemVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(systemVarCount)> systemVars;
    Value<atUint32> gameVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(gameVarCount)> gameVars;
    Value<atUint32> gameObjectCount;
    Vector<atUint32, DNA_COUNT(gameObjectCount)> gameObjects;
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

#endif // __DNAMP2_SAVW_HPP__
