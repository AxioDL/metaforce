#ifndef _DNAMP3_CAUD_HPP_
#define _DNAMP3_CAUD_HPP_

#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/PAK.hpp"

namespace DataSpec
{
namespace DNAMP3
{

struct CAUD : BigYAML
{
    DECL_YAML
    DNAFourCC magic;
    Value<atUint32> version;
    String<-1> name;
    Value<atUint32> nameCount;
    Vector<String<-1>, DNA_COUNT(nameCount)> names;
    Value<float> unknown1;
    Value<atUint32> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atUint32> unknown5;

    struct CSMPInfo : BigYAML
    {
        DECL_YAML
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
        struct UnknownStruct1 : BigYAML
        {
            DECL_YAML
            Value<float> unknown1;
            Value<float> unknown2;
            Value<atUint8> unknown3;
            Value<atUint8> unknown4;
        };
        Value<atUint32> unknown15;
        Vector<UnknownStruct1, DNA_COUNT(unknown15)> unknown16;
        struct UnknownStruct2 : BigYAML
        {
            DECL_YAML
            Value<atUint8> unknown1;
            Vector<atUint8, DNA_COUNT(unknown1)> unknown2;
            Value<float> unknown3;
            Value<float> unknown4;
            Value<atUint16> unknown5;
            struct UnknownPair : BigYAML
            {
                DECL_YAML
                Value<float> unknown1;
                Value<atInt32> unknown2;
            };
            Value<atUint16> unknown6;
            Vector<UnknownPair, DNA_COUNT(unknown6)> unknown7;
            struct UnknownQuad: BigYAML
            {
                Delete __dna_delete;
                Value<float> unknown1;
                Value<float> unknown2;
                Value<atUint8> unknown3;
                Value<atUint8> unknown4;
                Value<float> unknown5[5];

                void read(athena::io::IStreamReader& __dna_reader)
                {
                    /* unknown1 */
                    unknown1 = __dna_reader.readFloatBig();
                    /* unknown2 */
                    unknown2 = __dna_reader.readFloatBig();
                    /* unknown3 */
                    unknown3 = __dna_reader.readUByte();
                    /* unknown4 */
                    unknown4 = __dna_reader.readUByte();
                    if (unknown4 == 5)
                    {
                        unknown5[0] = __dna_reader.readFloatBig();
                        unknown5[1] = __dna_reader.readFloatBig();
                        unknown5[2] = __dna_reader.readFloatBig();
                        unknown5[3] = __dna_reader.readFloatBig();
                    }
                }

                void write(athena::io::IStreamWriter& __dna_writer) const
                {
                    /* unknown1 */
                    __dna_writer.writeFloatBig(unknown1);
                    /* unknown2 */
                    __dna_writer.writeFloatBig(unknown2);
                    /* unknown3 */
                    __dna_writer.writeUByte(unknown3);
                    /* unknown4 */
                    __dna_writer.writeUByte(unknown4);
                    if (unknown4 == 5)
                    {
                        __dna_writer.writeFloatBig(unknown5[0]);
                        __dna_writer.writeFloatBig(unknown5[1]);
                        __dna_writer.writeFloatBig(unknown5[2]);
                        __dna_writer.writeFloatBig(unknown5[3]);
                    }
                }

                void read(athena::io::YAMLDocReader& __dna_docin)
                {
                    /* unknown1 */
                    unknown1 = __dna_docin.readFloat("unknown1");
                    /* unknown2 */
                    unknown2 = __dna_docin.readFloat("unknown2");
                    /* unknown3 */
                    unknown3 = __dna_docin.readUByte("unknown3");
                    /* unknown4 */
                    unknown4 = __dna_docin.readUByte("unknown4");
                    if (unknown4 == 5)
                    {
                        unknown5[0] = __dna_docin.readFloat("unknown5");
                        unknown5[1] = __dna_docin.readFloat("unknown5");
                        unknown5[2] = __dna_docin.readFloat("unknown5");
                        unknown5[3] = __dna_docin.readFloat("unknown5");
                    }
                }

                void write(athena::io::YAMLDocWriter& __dna_docout) const
                {
                    /* unknown1 */
                    __dna_docout.writeFloat("unknown1", unknown1);
                    /* unknown2 */
                    __dna_docout.writeFloat("unknown2", unknown2);
                    /* unknown3 */
                    __dna_docout.writeUByte("unknown3", unknown3);
                    /* unknown4 */
                    __dna_docout.writeUByte("unknown4", unknown4);
                    if (unknown4 == 5)
                    {
                        __dna_docout.writeFloat("unknown5", unknown5[0]);
                        __dna_docout.writeFloat("unknown5", unknown5[1]);
                        __dna_docout.writeFloat("unknown5", unknown5[2]);
                        __dna_docout.writeFloat("unknown5", unknown5[3]);
                    }
                }

                const char* DNAType()
                {
                    return "DataSpec::DNAMP3::CAUD::CSMPInfo::UnknownStruct2::UnknownQuad";
                }

                size_t binarySize(size_t __isz) const
                {
                    return __isz + 10 + (unknown4 == 5 ? sizeof(float) * 4 : 0);
                }
            };

            Value<atUint16> unknown8;
            Vector<UnknownQuad, DNA_COUNT(unknown8)> unknown9;
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
        struct UnknownStruct3 : BigYAML
        {
            DECL_YAML
            Value<atUint32> unknown1;
            Value<float> unknown2;
            Value<atUint8> unknown3;
            Value<atUint8> unknown4;
        };
        Vector<UnknownStruct3, DNA_COUNT(unknown27)> unknown29;
        Value<float> unknown30;
        Value<float> unknown31;
    };

    Value<atUint32> infoCount;
    Vector<CSMPInfo, DNA_COUNT(infoCount)> info;

    static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
    {
        CAUD caud;
        caud.read(rs);
        athena::io::FileWriter writer(outPath.getAbsolutePath());
        caud.toYAMLStream(writer);
        return true;
    }
};
}
}

#endif // _DNAMP3_CAUD_HPP_
