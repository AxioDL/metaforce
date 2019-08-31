#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace DataSpec {
struct MayaSpline : public BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint8> preInf;
  Value<atUint8> postInf;
  Value<atUint32> knotCount;
  struct Knot : BigDNA {
    AT_DECL_DNA_YAML
    Value<float> time;
    Value<float> amplitude;
    Value<atUint8> unk1;
    Value<atUint8> unk2;
  };

  Vector<Knot, AT_DNA_COUNT(knotCount)> knots;
  Value<atUint8> clampMode;
  Value<float> minAmp;
  Value<float> maxAmp;
};
}