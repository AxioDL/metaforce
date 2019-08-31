#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Water : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> volume;
  DamageInfo damageInfo;
  Value<atVec3f> orientedForce;
  Value<atUint32> triggerFlags;
  Value<bool> thermalCold;
  Value<bool> displaySurface;
  UniqueID32 patternMap1;
  UniqueID32 patternMap2;
  UniqueID32 colorMap;
  UniqueID32 bumpMap;
  UniqueID32 envMap;
  UniqueID32 envBumpMap;
  Value<atVec3f> bumpLightDir;
  Value<float> bumpScale;
  Value<float> morphInTime;
  Value<float> morphOutTime;
  Value<bool> active;
  Value<atUint32> fluidType;
  Value<bool> unknownBool;
  Value<float> alpha;
  struct FluidUVMotion : BigDNA {
    AT_DECL_DNA
    struct FluidLayerMotion : BigDNA {
      AT_DECL_DNA
      Value<atUint32> motionType;
      Value<float> timeToWrap;
      Value<float> orientation;
      Value<float> magnitude;
      Value<float> uvMul;
    };

    FluidLayerMotion pattern1Layer;
    FluidLayerMotion pattern2Layer;
    FluidLayerMotion colorLayer;
    Value<float> timeToWrap;
    Value<float> orientation;
  } fluidUVMotion;

  Value<float> turbulenceSpeed;
  Value<float> turbulenceDistance;
  Value<float> turbulenceFrequencyMax;
  Value<float> turbulenceFrequencyMin;
  Value<float> turbulencePhaseMax;
  Value<float> turbulencePhaseMin;
  Value<float> turbulenceAmplitudeMax;
  Value<float> turbulenceAmplitudeMin;
  Value<atVec4f> splashColor;
  Value<atVec4f> insideFogColor;
  UniqueID32 splashParticle1;
  UniqueID32 splashParticle2;
  UniqueID32 splashParticle3;
  UniqueID32 visorRunoffParticle;
  UniqueID32 unmorphVisorRunoffParticle;
  Value<atUint32> visorRunoffSfx;
  Value<atUint32> unmorphVisorRunoffSfx;
  Value<atUint32> splashSfx1;
  Value<atUint32> splashSfx2;
  Value<atUint32> splashSfx3;
  Value<float> tileSize;
  Value<atUint32> tileSubdivisions;
  Value<float> specularMin;
  Value<float> specularMax;
  Value<float> reflectionSize;
  Value<float> rippleIntensity;
  Value<float> reflectionBlend;
  Value<float> fogBias;
  Value<float> fogMagnitude;
  Value<float> fogSpeed;
  Value<atVec4f> fogColor;
  UniqueID32 lightmap;
  Value<float> unitsPerLightmapTexel;
  Value<float> alphaInTime;
  Value<float> alphaOutTime;
  Value<atUint32> unusedInt1;
  Value<atUint32> unusedInt2;

  struct UnusedBitset : BigDNA{AT_DECL_EXPLICIT_DNA} unusedBitset;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (patternMap1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(patternMap1);
      ent->name = name + "_patternMap1";
    }
    if (patternMap2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(patternMap2);
      ent->name = name + "_patternMap2";
    }
    if (colorMap.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(colorMap);
      ent->name = name + "_colorMap";
    }
    if (bumpMap.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(bumpMap);
      ent->name = name + "_bumpMap";
    }
    if (envMap.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(envMap);
      ent->name = name + "_envMap";
    }
    if (envBumpMap.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(envBumpMap);
      ent->name = name + "_envBumpMap";
    }
    if (lightmap.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(lightmap);
      ent->name = name + "_lightmap";
    }
    if (splashParticle1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(splashParticle1);
      ent->name = name + "_splashParticle1";
    }
    if (splashParticle2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(splashParticle2);
      ent->name = name + "_splashParticle2";
    }
    if (splashParticle3.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(splashParticle3);
      ent->name = name + "_splashParticle3";
    }
    if (visorRunoffParticle.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(visorRunoffParticle);
      ent->name = name + "_visorRunoffParticle";
    }
    if (unmorphVisorRunoffParticle.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unmorphVisorRunoffParticle);
      ent->name = name + "_unmorphVisorRunoffParticle";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(patternMap1, pathsOut);
    g_curSpec->flattenDependencies(patternMap2, pathsOut);
    g_curSpec->flattenDependencies(colorMap, pathsOut);
    g_curSpec->flattenDependencies(bumpMap, pathsOut);
    g_curSpec->flattenDependencies(envMap, pathsOut);
    g_curSpec->flattenDependencies(envBumpMap, pathsOut);
    g_curSpec->flattenDependencies(lightmap, pathsOut);
    g_curSpec->flattenDependencies(splashParticle1, pathsOut);
    g_curSpec->flattenDependencies(splashParticle2, pathsOut);
    g_curSpec->flattenDependencies(splashParticle3, pathsOut);
    g_curSpec->flattenDependencies(visorRunoffParticle, pathsOut);
    g_curSpec->flattenDependencies(unmorphVisorRunoffParticle, pathsOut);
  }

  zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const override {
    zeus::CVector3f halfExtent = zeus::CVector3f(volume) / 2.f;
    zeus::CVector3f loc(location);
    return zeus::CAABox(loc - halfExtent, loc + halfExtent);
  }
};
} // namespace DataSpec::DNAMP1
