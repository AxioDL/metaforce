#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Effect : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  UniqueID32 part;
  UniqueID32 elsc;
  Value<bool> hotInThermal;
  Value<bool> noTimerUnlessAreaOccluded;
  Value<bool> rebuildSystemsOnActivate;
  Value<bool> active;
  Value<bool> useRateInverseCamDist;
  Value<float> rateInverseCamDist;
  Value<float> rateInverseCamDistRate;
  Value<float> duration;
  Value<float> durationResetWhileVisible;
  Value<bool> useRateCamDistRange;
  Value<float> rateCamDistRangeMin;
  Value<float> rateCamDistRangeMax;
  Value<float> rateCamDistRangeFarRate;
  Value<bool> combatVisorVisible;
  Value<bool> thermalVisorVisible;
  Value<bool> xrayVisorVisible;
  Value<bool> dieWhenSystemsDone;
  LightParameters lightParameters;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (part.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(part);
      ent->name = name + "_part";
    }
    if (elsc.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(elsc);
      ent->name = name + "_elsc";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(part, pathsOut);
    g_curSpec->flattenDependencies(elsc, pathsOut);
  }
};
} // namespace DataSpec::DNAMP1
