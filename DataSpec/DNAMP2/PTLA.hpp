#ifndef __DNAMP2_PTLA_HPP__
#define __DNAMP2_PTLA_HPP__

#include "../DNACommon/DNACommon.hpp"

namespace DataSpec
{
namespace DNAMP2
{
struct PTLA : BigDNA
{
    DECL_DNA
    Value<atUint32> magic;
    Value<atUint32> version;
    struct UnknownStruct1 : BigDNA
    {
        DECL_DNA
        Value<atUint32> count;
        struct Entry : BigDNA
        {
            DECL_DNA
            Value<float> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
            Value<atUint16> unknown5;
            Value<atUint16> unknown6;
        };
        Vector<Entry, DNA_COUNT(count)> entries;
        Value<atVec3f> unknown[2];
    };
    Value<atUint32> count1;
    Vector<UnknownStruct1, DNA_COUNT(count1)> entries1;

    struct UnknownStruct2 : BigDNA
    {
        DECL_DNA
        Value<atUint32> count;
        struct Entry : BigDNA
        {
            DECL_DNA
            Value<float> unknown1;
            Value<float> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
        };
        Vector<Entry, DNA_COUNT(count)> entries;
        Value<atUint16> unknown;
    };
    Value<atUint32> count2;
    Vector<UnknownStruct2, DNA_COUNT(count2)> entries2;

    Value<atUint32> shortCount1;
    Vector<atUint16, DNA_COUNT(shortCount1)> shorts1;

    Value<atUint32> shortCount2;
    Vector<atUint16, DNA_COUNT(shortCount2)> shorts2;

    struct UnknownStruct3 : BigDNA
    {
        DECL_DNA
        Value<atVec3f> unknown1[2];
        Value<atUint16> unknown2;
        Value<atUint16> unknown3;
        Value<atUint16> unknown4;
    };
    Value<atUint32> count3;
    Vector<UnknownStruct3, DNA_COUNT(count3)> entries3;
};
}
}
#endif // __DNAMP2_PTLA_HPP__
