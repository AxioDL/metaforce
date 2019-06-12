#pragma once
#include "hecl/hecl.hpp"
#include "hecl/FourCC.hpp"
#include "athena/DNA.hpp"
#include "athena/MemoryReader.hpp"

namespace hecl::blender {

struct SDNABlock : public athena::io::DNA<athena::Little> {
  AT_DECL_DNA
  DNAFourCC magic;
  DNAFourCC nameMagic;
  Value<atUint32> numNames;
  Vector<String<-1>, AT_DNA_COUNT(numNames)> names;
  Align<4> align1;
  DNAFourCC typeMagic;
  Value<atUint32> numTypes;
  Vector<String<-1>, AT_DNA_COUNT(numTypes)> types;
  Align<4> align2;
  DNAFourCC tlenMagic;
  Vector<Value<atUint16>, AT_DNA_COUNT(numTypes)> tlens;
  Align<4> align3;
  DNAFourCC strcMagic;
  Value<atUint32> numStrcs;
  struct SDNAStruct : public athena::io::DNA<athena::Little> {
    AT_DECL_DNA
    Value<atUint16> type;
    Value<atUint16> numFields;
    struct SDNAField : public athena::io::DNA<athena::Little> {
      AT_DECL_DNA
      Value<atUint16> type;
      Value<atUint16> name;
      atUint32 offset;
    };
    Vector<SDNAField, AT_DNA_COUNT(numFields)> fields;

    void computeOffsets(const SDNABlock& block);
    const SDNAField* lookupField(const SDNABlock& block, const char* n) const;
  };
  Vector<SDNAStruct, AT_DNA_COUNT(numStrcs)> strcs;

  const SDNAStruct* lookupStruct(const char* n, atUint32& idx) const;
};

struct FileBlock : public athena::io::DNA<athena::Little> {
  AT_DECL_DNA
  DNAFourCC type;
  Value<atUint32> size;
  Value<atUint64> ptr;
  Value<atUint32> sdnaIdx;
  Value<atUint32> count;
};

class SDNARead {
  std::vector<uint8_t> m_data;
  SDNABlock m_sdnaBlock;

public:
  explicit SDNARead(SystemStringView path);
  operator bool() const { return !m_data.empty(); }
  const SDNABlock& sdnaBlock() const { return m_sdnaBlock; }
  void enumerate(const std::function<bool(const FileBlock& block, athena::io::MemoryReader& r)>& func) const;
};

BlendType GetBlendType(SystemStringView path);

} // namespace hecl::blender
