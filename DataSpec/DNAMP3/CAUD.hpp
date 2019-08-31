#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/MayaSpline.hpp"
#include "DataSpec/DNACommon/PAK.hpp"
namespace DataSpec::DNAMP3 {

struct CAUD : BigDNA {
  AT_DECL_DNA_YAML
  DNAFourCC magic;
  Value<atUint32> version;
  String<-1> name;
  Value<atUint32> volumeGroupCount;
  Vector<String<-1>, AT_DNA_COUNT(volumeGroupCount)> volumeGroups;
  Value<float> unknown1;
  Value<atUint32> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<atUint32> unknown5;

  struct CSMPInfo : BigDNA {
    AT_DECL_DNA_YAML
    Value<atUint32> dataLen;
    UniqueID64 csmpId;
    Value<float> unknown1;
    Value<atUint32> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<atInt32> unknown8;
    Value<atInt32> unknown9;
    Value<atUint32> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<bool> unknown14;
    Value<bool> unknown15;
    Value<bool> unknown16;
    Value<bool> unknown17;
    Value<bool> unknown18;
    Value<bool> unknown19;
    MayaSpline spline1;
    MayaSpline spline2;
    MayaSpline spline3;
    MayaSpline spline4;
    Value<atUint32> unkStructCount;
    struct UnknownStruct : BigDNA {
      AT_DECL_DNA_YAML
      Value<float> unknown1;
      Value<float> unknown2;
    };
    Vector<UnknownStruct, AT_DNA_COUNT(unkStructCount)> unkStructs;
    Value<atUint32> unknown20;
    Value<float> unknown21;
    Value<bool> unknown22;
    Value<bool> unknown23;
    MayaSpline spline5;
  };

  Value<atUint32> infoCount;
  Vector<CSMPInfo, AT_DNA_COUNT(infoCount)> info;

  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
    CAUD caud;
    caud.read(rs);
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    athena::io::ToYAMLStream(caud, writer);
    return true;
  }
};
} // namespace DataSpec::DNAMP3
