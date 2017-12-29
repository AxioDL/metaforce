#ifndef __COMMON_FSM2_HPP__
#define __COMMON_FSM2_HPP__

#include "PAK.hpp"
#include "DNACommon.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAFSM2
{
struct IFSM : BigYAML
{
    Delete _d;
};

template <class IDType>
struct FSM2 : BigYAML
{
    struct Header : BigYAML
    {
        DECL_YAML
        DNAFourCC magic;
        Value<atUint32> version;
    } header;

    struct CommonStruct : BigYAML
    {
        DECL_YAML
        String<-1> name;
        Value<atUint32> unknown;
    };

    struct FSMV1 : IFSM
    {
        DECL_YAML
        Value<atUint32> stateCount;
        Value<atUint32> unknown1Count;
        Value<atUint32> unknown2Count;
        Value<atUint32> unknown3Count;
        struct State : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknownCount;
            Vector<CommonStruct, DNA_COUNT(unknownCount)> unknown;
        };

        struct Unknown1 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<float> unknown1;
            Value<atUint32> unknown2Count;
            Vector<CommonStruct, DNA_COUNT(unknown2Count)> unknown2;
            Value<atUint8> unknown3;
        };

        struct Unknown2 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknownCount;
            Vector<CommonStruct, DNA_COUNT(unknownCount)> unknown;
        };

        struct Unknown3 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknownCount;
            Vector<CommonStruct, DNA_COUNT(unknownCount)> unknown;
            IDType fsmId;
        };

        Vector<State, DNA_COUNT(stateCount)> states;
        Vector<Unknown1, DNA_COUNT(unknown1Count)> unknown1;
        Vector<Unknown2, DNA_COUNT(unknown2Count)> unknown2;
        Vector<Unknown3, DNA_COUNT(unknown3Count)> unknown3;
    };

    struct FSMV2 : IFSM
    {
        DECL_YAML
        Value<atUint32> stateCount;
        Value<atUint32> unknown1Count;
        Value<atUint32> unknown2Count;
        Value<atUint32> unknown3Count;
        struct State : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknown1;
            Value<atUint32> unknown2;
            Value<atUint32> unknown3;
            Value<atUint32> unknown4;
            Value<atUint32> unknown5Count;
            Vector<CommonStruct, DNA_COUNT(unknown5Count)> unknown5;
        };

        struct Unknown1 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknown1;
            Value<atUint32> unknown2;
            Value<atUint32> unknown3;
            Value<atUint32> unknown4;
            Value<float> unknown5;
            Value<atUint32> unknown6Count;
            Vector<CommonStruct, DNA_COUNT(unknown6Count)> unknown6;
            Value<atUint8> unknown7;
        };

        struct Unknown2 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknown1;
            Value<atUint32> unknown2;
            Value<atUint32> unknown3;
            Value<atUint32> unknown4;
            Value<atUint32> unknown5Count;
            Vector<CommonStruct, DNA_COUNT(unknown5Count)> unknown5;
        };

        struct Unknown3 : BigYAML
        {
            DECL_YAML
            String<-1> name;
            Value<atUint32> unknown1;
            Value<atUint32> unknown2;
            Value<atUint32> unknown3;
            Value<atUint32> unknown4;
            Value<atUint32> unknown5Count;
            Vector<CommonStruct, DNA_COUNT(unknown5Count)> unknown5;
            IDType fsmId;
        };

        Vector<State, DNA_COUNT(stateCount)> states;
        Vector<Unknown1, DNA_COUNT(unknown1Count)> unknown1;
        Vector<Unknown2, DNA_COUNT(unknown2Count)> unknown2;
        Vector<Unknown3, DNA_COUNT(unknown3Count)> unknown3;
    };

    void read(athena::io::IStreamReader& in);
    void write(athena::io::IStreamWriter& out) const;
    void read(athena::io::YAMLDocReader& in);
    void write(athena::io::YAMLDocWriter& out) const;
    std::unique_ptr<IFSM> detail;
    size_t binarySize(size_t __isz) const;
    static const char* DNAType();
    const char* DNATypeV() const;
};

template <class IDType>
bool ExtractFSM2(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template <class IDType>
bool WriteFSM2(const FSM2<IDType>& fsm2, const hecl::ProjectPath& outPath);

}
#endif // __COMMON_FSM2_HPP__
