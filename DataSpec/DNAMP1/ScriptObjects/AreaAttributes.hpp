#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct AreaAttributes : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  enum class EWeatherType : atUint32 { None, Snow, Rain };

  Value<atUint32> load; /* 0 causes the loader to bail and return null */
  Value<bool> skyboxEnabled;
  Value<EWeatherType> weather;
  Value<float> envFxDensity;
  Value<float> thermalHeat;
  Value<float> xrayFogDistance;
  Value<float> worldLightingLevel;
  UniqueID32 skybox;
  Value<atUint32> phazonType;

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(skybox, pathsOut);
  }
};
} // namespace DataSpec::DNAMP1
