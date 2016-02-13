#ifndef _DNAMP1_AREAATTRIBUTES_HPP_
#define _DNAMP1_AREAATTRIBUTES_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct AreaAttributes : IScriptObject
{
    DECL_YAML
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
};
}
}

#endif
