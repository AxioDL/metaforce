#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Steam : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> volume;
  DamageInfo damageInfo;
  Value<atVec3f> unknown1;
  Value<atUint32> unknown2;
  Value<bool> unknown3;
  UniqueID32 texture;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
  Value<bool> unknown8;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (texture.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
      ent->name = name + "_texture";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(texture, lazyOut);
  }
};
} // namespace DataSpec::DNAMP1
