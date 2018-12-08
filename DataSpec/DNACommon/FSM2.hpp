#pragma once

#include "PAK.hpp"
#include "DNACommon.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAFSM2 {
struct IFSM : BigDNAVYaml {
  Delete _d;
};

template <class IDType>
struct AT_SPECIALIZE_PARMS(DataSpec::UniqueID32, DataSpec::UniqueID64) FSM2 : BigDNA {
  struct Header : BigDNA {
    AT_DECL_DNA_YAML
    DNAFourCC magic = FOURCC('FSM2');
    Value<atUint32> version;
  } header;

  struct CommonStruct : BigDNA {
    AT_DECL_DNA_YAML
    String<-1> name;
    Value<atUint32> unknown;
  };

  struct FSMV1 : IFSM {
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    Value<atUint32> stateCount;
    Value<atUint32> unknown1Count;
    Value<atUint32> unknown2Count;
    Value<atUint32> unknown3Count;
    struct State : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknownCount;
      Vector<CommonStruct, AT_DNA_COUNT(unknownCount)> unknown;
    };

    struct Unknown1 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<float> unknown1;
      Value<atUint32> unknown2Count;
      Vector<CommonStruct, AT_DNA_COUNT(unknown2Count)> unknown2;
      Value<atUint8> unknown3;
    };

    struct Unknown2 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknownCount;
      Vector<CommonStruct, AT_DNA_COUNT(unknownCount)> unknown;
    };

    struct Unknown3 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknownCount;
      Vector<CommonStruct, AT_DNA_COUNT(unknownCount)> unknown;
      Value<IDType> fsmId;
    };

    Vector<State, AT_DNA_COUNT(stateCount)> states;
    Vector<Unknown1, AT_DNA_COUNT(unknown1Count)> unknown1;
    Vector<Unknown2, AT_DNA_COUNT(unknown2Count)> unknown2;
    Vector<Unknown3, AT_DNA_COUNT(unknown3Count)> unknown3;
  };

  struct FSMV2 : IFSM {
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    Value<atUint32> stateCount;
    Value<atUint32> unknown1Count;
    Value<atUint32> unknown2Count;
    Value<atUint32> unknown3Count;
    struct State : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknown1;
      Value<atUint32> unknown2;
      Value<atUint32> unknown3;
      Value<atUint32> unknown4;
      Value<atUint32> unknown5Count;
      Vector<CommonStruct, AT_DNA_COUNT(unknown5Count)> unknown5;
    };

    struct Unknown1 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknown1;
      Value<atUint32> unknown2;
      Value<atUint32> unknown3;
      Value<atUint32> unknown4;
      Value<float> unknown5;
      Value<atUint32> unknown6Count;
      Vector<CommonStruct, AT_DNA_COUNT(unknown6Count)> unknown6;
      Value<atUint8> unknown7;
    };

    struct Unknown2 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknown1;
      Value<atUint32> unknown2;
      Value<atUint32> unknown3;
      Value<atUint32> unknown4;
      Value<atUint32> unknown5Count;
      Vector<CommonStruct, AT_DNA_COUNT(unknown5Count)> unknown5;
    };

    struct Unknown3 : BigDNA {
      AT_DECL_DNA_YAML
      String<-1> name;
      Value<atUint32> unknown1;
      Value<atUint32> unknown2;
      Value<atUint32> unknown3;
      Value<atUint32> unknown4;
      Value<atUint32> unknown5Count;
      Vector<CommonStruct, AT_DNA_COUNT(unknown5Count)> unknown5;
      Value<IDType> fsmId;
    };

    Vector<State, AT_DNA_COUNT(stateCount)> states;
    Vector<Unknown1, AT_DNA_COUNT(unknown1Count)> unknown1;
    Vector<Unknown2, AT_DNA_COUNT(unknown2Count)> unknown2;
    Vector<Unknown3, AT_DNA_COUNT(unknown3Count)> unknown3;
  };

  std::unique_ptr<IFSM> detail;
  AT_DECL_EXPLICIT_DNA_YAML
};

template <class IDType>
bool ExtractFSM2(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template <class IDType>
bool WriteFSM2(const FSM2<IDType>& fsm2, const hecl::ProjectPath& outPath);

} // namespace DataSpec::DNAFSM2
