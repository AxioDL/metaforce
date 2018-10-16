#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct AreaAttributes : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    enum class EWeatherType : atUint32
    {
        None,
        Snow,
        Rain
    };

    Value<atUint32>     unknown1;
    Value<bool>         skyboxEnabled;
    Value<EWeatherType> weather;
    Value<float>        unknown2;
    Value<float>        unknown3;
    Value<float>        unknown4;
    Value<float>        unknown5;
    UniqueID32          skybox;
    Value<atUint32>     unknown6;

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(skybox, pathsOut);
    }
};
}

