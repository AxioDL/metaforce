#pragma once
#include "../../DNACommon/DNACommon.hpp"
#include "../DNAMP1.hpp"
#include "../SAVW.hpp"
#include "zeus/CAABox.hpp"

#include <cstdio>

namespace DataSpec::DNAMP1 {

zeus::CTransform ConvertEditorEulerToTransform4f(const zeus::CVector3f& scale, const zeus::CVector3f& orientation,
                                                 const zeus::CVector3f& position);

struct IScriptObject : BigDNAVYaml {
  AT_DECL_DNA_YAMLV
  atUint32 type;
  Value<atUint32> id;
  struct Connection : BigDNA {
    AT_DECL_DNA_YAML
    Value<atUint32> state;
    Value<atUint32> msg;
    Value<atUint32> target;
  };

  Value<atUint32> connectionCount;
  Vector<Connection, AT_DNA_COUNT(connectionCount)> connections;
  Value<atUint32> propertyCount;
  ~IScriptObject() override = default;

  virtual void addCMDLRigPairs(PAKRouter<PAKBridge>&, CharacterAssociations<UniqueID32>& charAssoc) const {}
  virtual void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {}
  virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                  std::vector<hecl::ProjectPath>& lazyOut) const {}
  virtual void gatherScans(std::vector<Scan>& scansOut) const {}
  virtual zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const { return {}; }
};
} // namespace DataSpec::DNAMP1
