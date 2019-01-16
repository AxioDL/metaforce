#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Warwasp : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atUint32> flavor;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<atUint8> colliderType;
  DamageInfo damageInfo1;
  UniqueID32 projectileWeapon;
  DamageInfo projectileDamage;
  UniqueID32 projectileVisorParticle;
  Value<atUint32> projectileVisorSfx;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (projectileWeapon) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(projectileWeapon);
      ent->name = name + "_wpsc";
    }
    if (projectileVisorParticle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(projectileVisorParticle);
      ent->name = name + "_part";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(projectileWeapon, pathsOut);
    g_curSpec->flattenDependencies(projectileVisorParticle, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
