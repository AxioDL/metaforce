#ifndef _DNAMP1_METROIDPRIMESTAGE1_HPP_
#define _DNAMP1_METROIDPRIMESTAGE1_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MetroidPrimeStage1 : IScriptObject
{
    DECL_YAML
    Value<atUint32> unknown1;
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<bool> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<atUint32> unknown6;
    Value<bool> unknown7;
    Value<atUint32> unknown8;
    HealthInfo healthInfo1;
    HealthInfo healthInfo2;
    Value<atUint32> unknown9;

    struct PrimeParameters1 : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<float> unknown1;
        Value<float> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<float> unknown11;
        Value<float> unknown12;
        Value<float> unknown13;
        Value<float> unknown14;
    } primeStruct1_1, primeStruct1_2, primeStruct1_3, primeStruct1_4;

    Value<atUint32> unknown10;
    Value<atUint32> unknown11;

    struct MassivePrimeStruct : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        PatternedInfo patternedInfo;
        ActorParameters actorParameters;
        Value<atUint32> unknown2;
        struct PrimeStruct2 : BigYAML
        {
            DECL_YAML
            Value<bool> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            struct PrimeStruct3 : BigYAML
            {
                DECL_YAML
                Value<bool> unknown1;
                Value<float> unknown2;
                Value<float> unknown3;
                Value<float> unknown4;
                Value<float> unknown5;
                Value<float> unknown6;
                Value<float> unknown7;
                Value<float> unknown8;
                Value<float> unknown9;
            } primeStruct3_1, primeStruct3_2, primeStruct3_3;
        } primeStruct2_1, primeStruct2_2, primeStruct2_3;

        Value<atUint32> unknown4;
        UniqueID32 particle1;
        UniqueID32 particle2;
        UniqueID32 particle3;
        DamageInfo damageInfo1;
        Value<float> unknown5;
        Value<float> unknown6;
        UniqueID32 texture1;
        Value<atUint32> unknown7;
        Value<atUint32> unknown8;
        UniqueID32 particle4;

        struct PrimeStruct4 : BigYAML
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
            Value<atVec4f> unknown12; // CColor
            Value<atVec4f> unknown13; // CColor
            UniqueID32 wpsc;
            DamageInfo damageInfo1;
            struct PrimeStruct5 : BigYAML
            {
                DECL_YAML
                Value<atUint32> propertyCount;
                UniqueID32 unknown1;
                Value<atUint32> unknown2;
                UniqueID32 unknown3;
                UniqueID32 unknown4;
                Value<atUint32> unknown5;
                Value<atUint32> unknown6;
                Value<atUint32> unknown7;
                Value<atUint32> unknown8;
            } primeStruct5;
            Value<float> unknown14;
            DamageInfo damageInfo2;
        } primeStruct4_1, primeStruct4_2, primeStruct4_3, primeStruct4_4;

        UniqueID32 wpsc1;
        DamageInfo damageInfo2;
        PrimeStruct2 primeStruct2_4;
        UniqueID32 wpsc2;
        DamageInfo damageInfo3;
        PrimeStruct2 primeStruct2_5;
        Value<atUint32> unknown3;
        UniqueID32 particle5;
        DamageInfo damageInfo4;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<float> unknown11;
        UniqueID32 texture2;
        Value<bool> unknown12;
        Value<bool> unknown13;
        Value<bool> unknown14;
        Value<bool> unknown15;
        DamageInfo damageInfo5;
        PrimeStruct2 primeStruct2_6;
        UniqueID32 particle6;
        UniqueID32 swhc;
        UniqueID32 particle7;
        UniqueID32 particle8;

        struct PrimeStruct6 : BigYAML
        {
            DECL_YAML
            Value<atUint32> propertyCount;
            DamageVulnerability damageVulnerability;
            Value<atVec4f> unknown1;
            Value<atUint32> unknown2;
            Value<atUint32> unknown3;
        } primeStruct6_1, primeStruct6_2, primeStruct6_3, primeStruct6_4;
    } massivePrimeStruct;
};
}
}

#endif
