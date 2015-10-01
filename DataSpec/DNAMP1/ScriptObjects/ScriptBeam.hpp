#ifndef _DNAMP1_SCRIPTBEAM_HPP_
#define _DNAMP1_SCRIPTBEAM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct ScriptBeam : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    UniqueID32 wpsc;
    struct ScriptBeamParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<atUint32> unknown1;
        UniqueID32 particel1;
        UniqueID32 particel2;
        UniqueID32 texture1;
        UniqueID32 texture2;
        Value<float> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<atVec4f> unknown11; // CColor
        Value<atVec4f> unknown12; // CColor
    } beamParametrs;
    DamageInfo damageInfo;
};
}
}

#endif
