#ifndef _DNAMP1_PHAZONPOOL_HPP_
#define _DNAMP1_PHAZONPOOL_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PhazonPool : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<bool> unknown1;
    UniqueID32 model1;
    UniqueID32 model2;
    UniqueID32 particle1;
    UniqueID32 particle2;
    Value<atUint32> unknown2;
    DamageInfo damageInfo;
    Value<atVec3f> unknown3;
    Value<atUint32> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<bool> unknown8;
    Value<float> unknown9;
};
}
}

#endif
