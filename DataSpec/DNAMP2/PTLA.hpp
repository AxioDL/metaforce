#ifndef __DNAMP2_PTLA_HPP__
#define __DNAMP2_PTLA_HPP__

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace DataSpec::DNAMP2
{
struct PTLA : BigDNA
{
    AT_DECL_DNA
    Value<atUint32> magic;
    struct UnknownStruct1 : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> count;
        struct Entry : BigDNA
        {
            AT_DECL_DNA
            Value<float> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
            Value<atUint16> unknown5;
            Value<atUint16> unknown6;
        };
        Vector<Entry, AT_DNA_COUNT(count)> entries;
        Value<atUint16> unknown1;
        Value<atVec3f> unknown2[2];
    };
    Value<atUint32> count1;
    Vector<UnknownStruct1, AT_DNA_COUNT(count1)> entries1;

    struct UnknownStruct2 : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> count;
        struct Entry : BigDNA
        {
            AT_DECL_DNA
            Value<float> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
        };
        Vector<Entry, AT_DNA_COUNT(count)> entries;
        Value<atUint16> unknown;
    };
    Value<atUint32> count2;
    Vector<UnknownStruct2, AT_DNA_COUNT(count2)> entries2;

    Value<atUint32> shortCount1;
    Vector<atUint16, AT_DNA_COUNT(shortCount1)> shorts1;

    Value<atUint32> shortCount2;
    Vector<atUint16, AT_DNA_COUNT(shortCount2)> shorts2;

    struct UnknownStruct3 : BigDNA
    {
        AT_DECL_DNA
        Value<atVec3f> unknown1[2];
        Value<atUint16> unknown2;
        Value<atUint16> unknown3;
        Value<atUint16> unknown4;
    };
    Value<atUint32> count3;
    Vector<UnknownStruct3, AT_DNA_COUNT(count3)> entries3;
};
}
#endif // __DNAMP2_PTLA_HPP__
