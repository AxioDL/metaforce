#pragma once

#include <memory>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"

#include <logvisor/logvisor.hpp>

namespace DataSpec::DNAParticle {
extern logvisor::Module LogModule;

struct IElement : BigDNAVYaml {
  Delete _d;
  ~IElement() override = default;
  virtual std::string_view ClassID() const = 0;
  std::string_view DNATypeV() const override { return ClassID(); }
};

struct IRealElement : IElement {
  Delete _d2;
};
struct RealElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IRealElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IIntElement : IElement {
  Delete _d2;
};
struct IntElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IIntElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IVectorElement : IElement {
  Delete _d2;
};
struct VectorElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IVectorElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IColorElement : IElement {
  Delete _d2;
};
struct ColorElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IColorElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IModVectorElement : IElement {
  Delete _d2;
};
struct ModVectorElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IModVectorElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IEmitterElement : IElement {
  Delete _d2;
};
struct EmitterElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  std::unique_ptr<IEmitterElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

struct IUVElement : IElement {
  Delete _d2;
  virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const = 0;
};

struct BoolHelper : IElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  bool value = false;
  operator bool() const { return value; }
  BoolHelper& operator=(bool val) {
    value = val;
    return *this;
  }
  std::string_view ClassID() const override { return "BoolHelper"sv; }
};

struct RELifetimeTween : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "LFTW"sv; }
};

struct REConstant : IRealElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  Value<float> val;
  std::string_view ClassID() const override { return "CNST"sv; }
};

struct RETimeChain : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "CHAN"sv; }
};

struct REAdd : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "ADD_"sv; }
};

struct REClamp : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory min;
  RealElementFactory max;
  RealElementFactory val;
  std::string_view ClassID() const override { return "CLMP"sv; }
};

struct REKeyframeEmitter : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  Value<atUint32> percentageTween;
  Value<atUint32> unk1;
  Value<bool> loop;
  Value<atUint8> unk2;
  Value<atUint32> loopEnd;
  Value<atUint32> loopStart;
  Value<atUint32> count;
  Vector<float, AT_DNA_COUNT(count)> keys;
  std::string_view ClassID() const override { return percentageTween ? "KEYP"sv : "KEYE"sv; }
};

struct REInitialRandom : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "IRND"sv; }
};

struct RERandom : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "RAND"sv; }
};

struct REMultiply : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "MULT"sv; }
};

struct REPulse : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "PULS"sv; }
};

struct RETimeScale : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory dv;
  std::string_view ClassID() const override { return "SCAL"sv; }
};

struct RELifetimePercent : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory percent;
  std::string_view ClassID() const override { return "RLPT"sv; }
};

struct RESineWave : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory frequency;
  RealElementFactory amplitude;
  RealElementFactory phase;
  std::string_view ClassID() const override { return "SINE"sv; }
};

struct REInitialSwitch : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "ISWT"sv; }
};

struct RECompareLessThan : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory ca;
  RealElementFactory cb;
  RealElementFactory pass;
  RealElementFactory fail;
  std::string_view ClassID() const override { return "CLTN"sv; }
};

struct RECompareEquals : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory ca;
  RealElementFactory cb;
  RealElementFactory pass;
  RealElementFactory fail;
  std::string_view ClassID() const override { return "CEQL"sv; }
};

struct REParticleAdvanceParam1 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP1"sv; }
};

struct REParticleAdvanceParam2 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP2"sv; }
};

struct REParticleAdvanceParam3 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP3"sv; }
};

struct REParticleAdvanceParam4 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP4"sv; }
};

struct REParticleAdvanceParam5 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP5"sv; }
};

struct REParticleAdvanceParam6 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP6"sv; }
};

struct REParticleAdvanceParam7 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP7"sv; }
};

struct REParticleAdvanceParam8 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PAP8"sv; }
};

struct REParticleSizeOrLineLength : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PSLL"sv; }
};

struct REParticleRotationOrLineWidth : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PRLW"sv; }
};

struct RESubtract : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "SUB_"sv; }
};

struct REVectorMagnitude : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  std::string_view ClassID() const override { return "VMAG"sv; }
};

struct REVectorXToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  std::string_view ClassID() const override { return "VXTR"sv; }
};

struct REVectorYToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  std::string_view ClassID() const override { return "VYTR"sv; }
};

struct REVectorZToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  std::string_view ClassID() const override { return "VZTR"sv; }
};

struct RECEXT : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory index;
  std::string_view ClassID() const override { return "CEXT"sv; }
};

struct REIntTimesReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "ITRL"sv; }
};

struct IEKeyframeEmitter : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  Value<atUint32> percentageTween;
  Value<atUint32> unk1;
  Value<bool> loop;
  Value<atUint8> unk2;
  Value<atUint32> loopEnd;
  Value<atUint32> loopStart;
  Value<atUint32> count;
  Vector<atUint32, AT_DNA_COUNT(count)> keys;
  std::string_view ClassID() const override { return percentageTween ? "KEYP"sv : "KEYE"sv; }
};

struct IEDeath : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory passthrough;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "DETH"sv; }
};

struct IEClamp : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory min;
  IntElementFactory max;
  IntElementFactory val;
  std::string_view ClassID() const override { return "CLMP"sv; }
};

struct IETimeChain : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "CHAN"sv; }
};

struct IEAdd : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "ADD_"sv; }
};

struct IEConstant : IIntElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  Value<atUint32> val;
  std::string_view ClassID() const override { return "CNST"sv; }
};

struct IEImpulse : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory val;
  std::string_view ClassID() const override { return "IMPL"sv; }
};

struct IELifetimePercent : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory percent;
  std::string_view ClassID() const override { return "ILPT"sv; }
};

struct IEInitialRandom : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "IRND"sv; }
};

struct IEPulse : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "PULS"sv; }
};

struct IEMultiply : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "MULT"sv; }
};

struct IESampleAndHold : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory val;
  IntElementFactory waitMin;
  IntElementFactory waitMax;
  std::string_view ClassID() const override { return "SPAH"sv; }
};

struct IERandom : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "RAND"sv; }
};

struct IETimeScale : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory dv;
  std::string_view ClassID() const override { return "TSCL"sv; }
};

struct IEGTCP : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "GTCP"sv; }
};

struct IEModulo : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  std::string_view ClassID() const override { return "MODU"sv; }
};

struct IESubtract : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory direction;
  IntElementFactory baseRadius;
  std::string_view ClassID() const override { return "SUB_"sv; }
};

struct VECone : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  RealElementFactory b;
  std::string_view ClassID() const override { return "CONE"sv; }
};

struct VETimeChain : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "CHAN"sv; }
};

struct VEAngleCone : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory angleXBias;
  RealElementFactory angleYBias;
  RealElementFactory angleXRange;
  RealElementFactory angleYRange;
  RealElementFactory magnitude;
  std::string_view ClassID() const override { return "ANGC"sv; }
};

struct VEAdd : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  std::string_view ClassID() const override { return "ADD_"sv; }
};

struct VECircleCluster : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory circleOffset;
  VectorElementFactory circleNormal;
  IntElementFactory cycleFrames;
  RealElementFactory randomFactor;
  std::string_view ClassID() const override { return "CCLU"sv; }
};

struct VEConstant : IVectorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[3];
  std::string_view ClassID() const override { return "CNST"sv; }
};

struct VECircle : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory circleOffset;
  VectorElementFactory circleNormal;
  RealElementFactory angleConstant;
  RealElementFactory angleLinear;
  RealElementFactory circleRadius;
  std::string_view ClassID() const override { return "CIRC"sv; }
};

struct VEKeyframeEmitter : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  Value<atUint32> percentageTween;
  Value<atUint32> unk1;
  Value<bool> loop;
  Value<atUint8> unk2;
  Value<atUint32> loopEnd;
  Value<atUint32> loopStart;
  Value<atUint32> count;
  Vector<atVec3f, AT_DNA_COUNT(count)> keys;
  std::string_view ClassID() const override { return percentageTween ? "KEYP"sv : "KEYE"sv; }
};

struct VEMultiply : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  std::string_view ClassID() const override { return "MULT"sv; }
};

struct VERealToVector : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  std::string_view ClassID() const override { return "RTOV"sv; }
};

struct VEPulse : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  VectorElementFactory a;
  VectorElementFactory b;
  std::string_view ClassID() const override { return "PULS"sv; }
};

struct VEParticleVelocity : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PVEL"sv; }
};

struct VESPOS : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  std::string_view ClassID() const override { return "SPOS"sv; }
};

struct VEPLCO : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PLCO"sv; }
};

struct VEPLOC : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PLOC"sv; }
};

struct VEPSOR : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PSOR"sv; }
};

struct VEPSOF : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "PSOF"sv; }
};

struct CEKeyframeEmitter : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  Value<atUint32> percentageTween;
  Value<atUint32> unk1;
  Value<bool> loop;
  Value<atUint8> unk2;
  Value<atUint32> loopEnd;
  Value<atUint32> loopStart;
  Value<atUint32> count;
  Vector<atVec4f, AT_DNA_COUNT(count)> keys;
  std::string_view ClassID() const override { return percentageTween ? "KEYP"sv : "KEYE"sv; }
};

struct CEConstant : IColorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[4];
  std::string_view ClassID() const override { return "CNST"sv; }
};

struct CETimeChain : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "CHAN"sv; }
};

struct CEFadeEnd : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  RealElementFactory startFrame;
  RealElementFactory endFrame;
  std::string_view ClassID() const override { return "CFDE"sv; }
};

struct CEFade : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  RealElementFactory endFrame;
  std::string_view ClassID() const override { return "FADE"sv; }
};

struct CEPulse : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  ColorElementFactory a;
  ColorElementFactory b;
  std::string_view ClassID() const override { return "PULS"sv; }
};

struct MVEImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  std::string_view ClassID() const override { return "IMPL"sv; }
};

struct MVEExponentialImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  std::string_view ClassID() const override { return "EMPL"sv; }
};

struct MVETimeChain : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ModVectorElementFactory a;
  ModVectorElementFactory b;
  IntElementFactory thresholdFrame;
  std::string_view ClassID() const override { return "CHAN"sv; }
};

struct MVEBounce : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory planePoint;
  VectorElementFactory planeNormal;
  RealElementFactory friction;
  RealElementFactory restitution;
  BoolHelper dieOnPenetrate;
  std::string_view ClassID() const override { return "BNCE"sv; }
};

struct MVEConstant : IModVectorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[3];
  std::string_view ClassID() const override { return "CNST"sv; }
};

struct MVEGravity : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory acceleration;
  std::string_view ClassID() const override { return "GRAV"sv; }
};

struct MVEExplode : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory impulseMagnitude;
  RealElementFactory falloffFactor;
  std::string_view ClassID() const override { return "EXPL"sv; }
};

struct MVESetPosition : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory position;
  std::string_view ClassID() const override { return "SPOS"sv; }
};

struct MVELinearImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  std::string_view ClassID() const override { return "LMPL"sv; }
};

struct MVEPulse : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  ModVectorElementFactory a;
  ModVectorElementFactory b;
  std::string_view ClassID() const override { return "PULS"sv; }
};

struct MVEWind : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory windVelocity;
  RealElementFactory factor;
  std::string_view ClassID() const override { return "WIND"sv; }
};

struct MVESwirl : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory helixPoint;
  VectorElementFactory curveBinormal;
  RealElementFactory filterGain;
  RealElementFactory tangentialVelocity;
  std::string_view ClassID() const override { return "SWRL"sv; }
};

struct EESimpleEmitter : IEmitterElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory position;
  VectorElementFactory velocity;
  std::string_view ClassID() const override { return "SEMR"sv; }
};

struct VESphere : IEmitterElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory sphereOrigin;
  RealElementFactory sphereRadius;
  RealElementFactory magnitude;
  std::string_view ClassID() const override { return "SPHE"sv; }
};

struct VEAngleSphere : IEmitterElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory sphereOrigin;
  RealElementFactory sphereRadius;
  RealElementFactory magnitude;
  RealElementFactory angleXBias;
  RealElementFactory angleYBias;
  RealElementFactory angleXRange;
  RealElementFactory angleYRange;
  std::string_view ClassID() const override { return "ASPH"sv; }
};

struct EESimpleEmitterTR : EESimpleEmitter {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  std::string_view ClassID() const override { return "SETR"sv; }
};

template <class IDType>
struct UVEConstant : IUVElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  AT_SUBDECL_DNA
  CastIDToZero<IDType> tex;
  std::string_view ClassID() const override { return "CNST"sv; }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const override {
    g_curSpec->flattenDependencies(tex, pathsOut);
  }
};

template <class IDType>
struct UVEAnimTexture : IUVElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  AT_SUBDECL_DNA
  CastIDToZero<IDType> tex;
  IntElementFactory tileW;
  IntElementFactory tileH;
  IntElementFactory strideW;
  IntElementFactory strideH;
  IntElementFactory cycleFrames;
  Value<bool> loop = false;
  std::string_view ClassID() const override { return "ATEX"sv; }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const override {
    g_curSpec->flattenDependencies(tex, pathsOut);
  }
};

template <class IDType>
struct UVElementFactory : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA
  DNAFourCC m_type;
  std::unique_ptr<IUVElement> m_elem;
  operator bool() const { return m_elem.operator bool(); }
};

template <class IDType>
struct SpawnSystemKeyframeData : BigDNA {
  Value<atUint32> a;
  Value<atUint32> b;
  Value<atUint32> endFrame;
  Value<atUint32> d;

  struct SpawnSystemKeyframeInfo : BigDNA {
    IDType id;
    Value<atUint32> a;
    Value<atUint32> b;
    Value<atUint32> c;
    AT_DECL_EXPLICIT_DNA_YAML
  };

  std::vector<std::pair<atUint32, std::vector<SpawnSystemKeyframeInfo>>> spawns;

  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA

  operator bool() const { return spawns.size() != 0; }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
    for (const auto& p : spawns)
      for (const SpawnSystemKeyframeInfo& info : p.second)
        g_curSpec->flattenDependencies(info.id, pathsOut);
  }
};

template <class IDType>
struct ChildResourceFactory : BigDNA {
  IDType id;
  AT_DECL_EXPLICIT_DNA_YAML
  AT_SUBDECL_DNA
  operator bool() const { return id.isValid(); }
};

} // namespace DataSpec::DNAParticle
