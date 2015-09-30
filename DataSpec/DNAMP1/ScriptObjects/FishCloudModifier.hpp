#ifndef _DNAMP1_FISHCLOUDMODIFIER_HPP_
#define _DNAMP1_FISHCLOUDMODIFIER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct FishCloudModifier : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> position;
    Value<bool> unknown1;
    Value<bool> unknown2;
    Value<bool> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
};
}
}

#endif
