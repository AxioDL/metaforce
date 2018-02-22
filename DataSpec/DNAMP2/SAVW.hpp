#ifndef __DNAMP2_SAVW_HPP__
#define __DNAMP2_SAVW_HPP__

#include "../DNAMP1/SAVW.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2
{
struct SAVW : DNAMP1::SAVW
{
    AT_DECL_DNA_YAML
    Value<atUint32> systemVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(systemVarCount)> systemVars;
    Value<atUint32> gameVarCount;
    Vector<SAVWCommon::EnvironmentVariable, DNA_COUNT(gameVarCount)> gameVars;
    Value<atUint32> gameObjectCount;
    Vector<atUint32, DNA_COUNT(gameObjectCount)> gameObjects;
};
}

#endif // __DNAMP2_SAVW_HPP__
