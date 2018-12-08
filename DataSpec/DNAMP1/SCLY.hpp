#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "ScriptObjects/IScriptObject.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1 {
struct SCLY : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  Value<FourCC> fourCC;
  Value<atUint32> version;
  Value<atUint32> layerCount;

  Vector<atUint32, AT_DNA_COUNT(layerCount)> layerSizes;

  struct ScriptLayer : BigDNA {
    AT_DECL_EXPLICIT_DNA_YAML
    Value<atUint8> unknown;
    Value<atUint32> objectCount;
    Vector<std::unique_ptr<IScriptObject>, AT_DNA_COUNT(objectCount)> objects;
    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const;
    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
  };
  Vector<ScriptLayer, AT_DNA_COUNT(layerCount)> layers;

  void exportToLayerDirectories(const PAK::Entry&, PAKRouter<PAKBridge>&, bool) const;
  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const;
  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const;
};
} // namespace DataSpec::DNAMP1
