#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {

struct WorldTeleporter : IScriptObject {
  AT_DECL_EXPLICIT_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<bool> active;
  UniqueID32 mlvl;
  UniqueID32 mrea;
  AnimationParameters animationParameters;
  Value<atVec3f> playerScale;
  UniqueID32 platformModel;
  Value<atVec3f> platformScale;
  UniqueID32 backgroundModel;
  Value<atVec3f> backgroundScale;
  Value<bool> upElevator;
  Value<atUint32> elevatorSound; // needs verifcation
  Value<atUint32> volume;
  Value<atUint32> panning;
  Value<bool> showText;
  UniqueID32 font;
  UniqueID32 strg;
  Value<bool> fadeWhite;
  Value<float> charFadeInTime;
  Value<float> charsPerSecond;
  Value<float> showDelay;

  /* Trilogy additions (property count 26) */
  String<-1> audioStream;
  Value<bool> unknown13;
  Value<float> unknown14;
  Value<float> unknown15;
  Value<float> unknown16;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (platformModel) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(platformModel);
      ent->name = name + "_model1";
    }
    if (backgroundModel) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(backgroundModel);
      ent->name = name + "_model2";
    }
    if (strg) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(strg);
      ent->name = name + "_strg";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(platformModel, pathsOut);
    g_curSpec->flattenDependencies(backgroundModel, pathsOut);
    g_curSpec->flattenDependencies(strg, pathsOut);
  }
};

} // namespace DataSpec::DNAMP1
