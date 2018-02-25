#ifndef _DNAMP3_CAUD_HPP_
#define _DNAMP3_CAUD_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/PAK.hpp"

namespace DataSpec::DNAMP3
{

struct CAUD : BigDNA
{
    AT_DECL_DNA_YAML
    DNAFourCC magic;
    Value<atUint32> version;
    String<-1> name;
    Value<atUint32> nameCount;
    Vector<String<-1>, AT_DNA_COUNT(nameCount)> names;
    Value<float> unknown1;
    Value<atUint32> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atUint32> unknown5;

    struct CSMPInfo : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> dataLen;
        UniqueID64 csmpId;
        Value<float> unknown1;
        Value<atUint32> unknown2;
        Value<atUint32> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<atUint32> unknown7;
        Value<atUint32> unknown8;
        Value<atUint32> unknown9;
        Value<atUint32> unknown10;
        Value<float> unknown11;
        Value<float> unknown12;
        Value<float> unknown13;
        Value<atUint8> unknown14[8];
        struct UnknownStruct1 : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<float> unknown1;
            Value<float> unknown2;
            Value<atUint8> unknown3;
            Value<atUint8> unknown4;
        };
        Value<atUint32> unknown15;
        Vector<UnknownStruct1, AT_DNA_COUNT(unknown15)> unknown16;
        struct UnknownStruct2 : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint8> unknown1;
            Vector<atUint8, AT_DNA_COUNT(unknown1)> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
            Value<atUint16> unknown5;
            struct UnknownPair : BigDNA
            {
                AT_DECL_DNA_YAML
                Value<float> unknown1;
                Value<atInt32> unknown2;
            };
            Value<atUint16> unknown6;
            Vector<UnknownPair, AT_DNA_COUNT(unknown6)> unknown7;
            struct UnknownQuad: BigDNA
            {
                AT_DECL_EXPLICIT_DNA_YAML
                Value<float> unknown1;
                Value<float> unknown2;
                Value<atUint8> unknown3;
                Value<atUint8> unknown4;
                Value<float> unknown5[5];
            };

            Value<atUint16> unknown8;
            Vector<UnknownQuad, AT_DNA_COUNT(unknown8)> unknown9;
        };
        UnknownStruct2 unknown17[4];
        Value<atUint16> unknown18;
        Value<float> unknown19;
        Value<atUint8> unknown20;
        Value<atUint8> unknown21;
        Value<atUint8> unknown22;
        Value<atUint8> unknown23;
        Value<atUint8> unknown24;
        Value<atUint8> unknown25;
        Value<atUint8> unknown26;
        Value<atUint8> unknown27;
        Value<atUint8> unknown28;
        struct UnknownStruct3 : BigDNA
        {
            AT_DECL_DNA_YAML
            Value<atUint32> unknown1;
            Value<float> unknown2;
            Value<atUint8> unknown3;
            Value<atUint8> unknown4;
        };
        Vector<UnknownStruct3, AT_DNA_COUNT(unknown27)> unknown29;
        Value<float> unknown30;
        Value<float> unknown31;
    };

    Value<atUint32> infoCount;
    Vector<CSMPInfo, AT_DNA_COUNT(infoCount)> info;

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        CAUD caud;
        caud.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        athena::io::ToYAMLStream(caud, writer);
        return true;
    }
};
}

#endif // _DNAMP3_CAUD_HPP_
