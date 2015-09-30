#ifndef _DNAMP1_RIDLEY_HPP_
#define _DNAMP1_RIDLEY_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Ridley : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 model1;
    UniqueID32 model2;
    UniqueID32 model3;
    UniqueID32 model4;
    UniqueID32 model5;
    UniqueID32 model6;
    UniqueID32 model7;
    UniqueID32 model8;
    UniqueID32 model9;
    UniqueID32 model10;
    UniqueID32 model11;
    UniqueID32 model12;
    UniqueID32 particle;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    struct RidleyStruct1 : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<atUint32> unknown2;
        UniqueID32 particle1;
        UniqueID32 particle2;
        UniqueID32 texture1;
        UniqueID32 texture2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<float> unknown11;
        Value<atVec4f> unknown12; //CColor
        Value<atVec4f> unknown13; //CColor
    } ridleyStruct1;

    Value<atUint32> soundID1;
    UniqueID32 wpsc2;
    DamageInfo damageInfo2;

    struct RidleyStruct2 : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<float> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<bool> unknown9;
    } ridleyStruct2_1;

    UniqueID32 wpsc3;
    DamageInfo damageInfo3;
    RidleyStruct2 ridleyStruct2_2;
    Value<atUint32> soundID2;
    DamageInfo damageInfo4;
    RidleyStruct2 ridleyStruct2_3;
    Value<float> unknown5;
    Value<float> unknown6;
    DamageInfo damageInfo5;
    Value<float> unknown7;
    DamageInfo damageInfo6;
    Value<float> unknown8;
    DamageInfo damageInfo7;
    Value<float> unknown9;
    UniqueID32 elsc;
    Value<float> unknown10;
    Value<atUint32> soundID3;
    DamageInfo damageInfo8;
};
}
}

#endif
