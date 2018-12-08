#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DNAMP2.hpp"

namespace DataSpec::DNAMP2 {

class AGSC {
public:
  struct Header : BigDNA {
    AT_DECL_DNA
    Value<atUint32> unk;
    String<-1> groupName;
    Value<atUint16> groupId = -1;
    Value<atUint32> poolSz = 0;
    Value<atUint32> projSz = 0;
    Value<atUint32> sdirSz = 0;
    Value<atUint32> sampSz = 0;
  };
  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
  static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

} // namespace DataSpec::DNAMP2
