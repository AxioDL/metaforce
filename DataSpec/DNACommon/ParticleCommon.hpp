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
  virtual const char* ClassID() const = 0;
  const char* DNATypeV() const override { return ClassID(); }
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
  const char* ClassID() const override { return "BoolHelper"; }
};

struct RELifetimeTween : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "LFTW"; }
};

struct REConstant : IRealElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  Value<float> val;
  const char* ClassID() const override { return "CNST"; }
};

struct RETimeChain : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "CHAN"; }
};

struct REAdd : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "ADD_"; }
};

struct REClamp : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory min;
  RealElementFactory max;
  RealElementFactory val;
  const char* ClassID() const override { return "CLMP"; }
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
  const char* ClassID() const override { return percentageTween ? "KEYP" : "KEYE"; }
};

struct REInitialRandom : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "IRND"; }
};

struct RERandom : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "RAND"; }
};

struct REMultiply : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "MULT"; }
};

struct REPulse : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "PULS"; }
};

struct RETimeScale : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory dv;
  const char* ClassID() const override { return "SCAL"; }
};

struct RELifetimePercent : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory percent;
  const char* ClassID() const override { return "RLPT"; }
};

struct RESineWave : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory frequency;
  RealElementFactory amplitude;
  RealElementFactory phase;
  const char* ClassID() const override { return "SINE"; }
};

struct REInitialSwitch : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "ISWT"; }
};

struct RECompareLessThan : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory ca;
  RealElementFactory cb;
  RealElementFactory pass;
  RealElementFactory fail;
  const char* ClassID() const override { return "CLTN"; }
};

struct RECompareEquals : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory ca;
  RealElementFactory cb;
  RealElementFactory pass;
  RealElementFactory fail;
  const char* ClassID() const override { return "CEQL"; }
};

struct REParticleAdvanceParam1 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP1"; }
};

struct REParticleAdvanceParam2 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP2"; }
};

struct REParticleAdvanceParam3 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP3"; }
};

struct REParticleAdvanceParam4 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP4"; }
};

struct REParticleAdvanceParam5 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP5"; }
};

struct REParticleAdvanceParam6 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP6"; }
};

struct REParticleAdvanceParam7 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP7"; }
};

struct REParticleAdvanceParam8 : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PAP8"; }
};

struct REParticleSizeOrLineLength : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PSLL"; }
};

struct REParticleRotationOrLineWidth : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PRLW"; }
};

struct RESubtract : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "SUB_"; }
};

struct REVectorMagnitude : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  const char* ClassID() const override { return "VMAG"; }
};

struct REVectorXToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  const char* ClassID() const override { return "VXTR"; }
};

struct REVectorYToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  const char* ClassID() const override { return "VYTR"; }
};

struct REVectorZToReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory vec;
  const char* ClassID() const override { return "VZTR"; }
};

struct RECEXT : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory index;
  const char* ClassID() const override { return "CEXT"; }
};

struct REIntTimesReal : IRealElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "ITRL"; }
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
  const char* ClassID() const override { return percentageTween ? "KEYP" : "KEYE"; }
};

struct IEDeath : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory passthrough;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "DETH"; }
};

struct IEClamp : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory min;
  IntElementFactory max;
  IntElementFactory val;
  const char* ClassID() const override { return "CLMP"; }
};

struct IETimeChain : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "CHAN"; }
};

struct IEAdd : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "ADD_"; }
};

struct IEConstant : IIntElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  Value<atUint32> val;
  const char* ClassID() const override { return "CNST"; }
};

struct IEImpulse : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory val;
  const char* ClassID() const override { return "IMPL"; }
};

struct IELifetimePercent : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory percent;
  const char* ClassID() const override { return "ILPT"; }
};

struct IEInitialRandom : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "IRND"; }
};

struct IEPulse : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "PULS"; }
};

struct IEMultiply : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "MULT"; }
};

struct IESampleAndHold : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory val;
  IntElementFactory waitMin;
  IntElementFactory waitMax;
  const char* ClassID() const override { return "SPAH"; }
};

struct IERandom : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "RAND"; }
};

struct IETimeScale : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory dv;
  const char* ClassID() const override { return "TSCL"; }
};

struct IEGTCP : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "GTCP"; }
};

struct IEModulo : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory a;
  IntElementFactory b;
  const char* ClassID() const override { return "MODU"; }
};

struct IESubtract : IIntElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory direction;
  IntElementFactory baseRadius;
  const char* ClassID() const override { return "SUB_"; }
};

struct VECone : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  RealElementFactory b;
  const char* ClassID() const override { return "CONE"; }
};

struct VETimeChain : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "CHAN"; }
};

struct VEAngleCone : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory angleXBias;
  RealElementFactory angleYBias;
  RealElementFactory angleXRange;
  RealElementFactory angleYRange;
  RealElementFactory magnitude;
  const char* ClassID() const override { return "ANGC"; }
};

struct VEAdd : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  const char* ClassID() const override { return "ADD_"; }
};

struct VECircleCluster : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory circleOffset;
  VectorElementFactory circleNormal;
  IntElementFactory cycleFrames;
  RealElementFactory randomFactor;
  const char* ClassID() const override { return "CCLU"; }
};

struct VEConstant : IVectorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[3];
  const char* ClassID() const override { return "CNST"; }
};

struct VECircle : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory circleOffset;
  VectorElementFactory circleNormal;
  RealElementFactory angleConstant;
  RealElementFactory angleLinear;
  RealElementFactory circleRadius;
  const char* ClassID() const override { return "CIRC"; }
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
  const char* ClassID() const override { return percentageTween ? "KEYP" : "KEYE"; }
};

struct VEMultiply : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  VectorElementFactory b;
  const char* ClassID() const override { return "MULT"; }
};

struct VERealToVector : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory a;
  const char* ClassID() const override { return "RTOV"; }
};

struct VEPulse : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  VectorElementFactory a;
  VectorElementFactory b;
  const char* ClassID() const override { return "PULS"; }
};

struct VEParticleVelocity : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PVEL"; }
};

struct VESPOS : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory a;
  const char* ClassID() const override { return "SPOS"; }
};

struct VEPLCO : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PLCO"; }
};

struct VEPLOC : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PLOC"; }
};

struct VEPSOR : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PSOR"; }
};

struct VEPSOF : IVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "PSOF"; }
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
  const char* ClassID() const override { return percentageTween ? "KEYP" : "KEYE"; }
};

struct CEConstant : IColorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[4];
  const char* ClassID() const override { return "CNST"; }
};

struct CETimeChain : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "CHAN"; }
};

struct CEFadeEnd : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  RealElementFactory startFrame;
  RealElementFactory endFrame;
  const char* ClassID() const override { return "CFDE"; }
};

struct CEFade : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ColorElementFactory a;
  ColorElementFactory b;
  RealElementFactory endFrame;
  const char* ClassID() const override { return "FADE"; }
};

struct CEPulse : IColorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  ColorElementFactory a;
  ColorElementFactory b;
  const char* ClassID() const override { return "PULS"; }
};

struct MVEImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  const char* ClassID() const override { return "IMPL"; }
};

struct MVEExponentialImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  const char* ClassID() const override { return "EMPL"; }
};

struct MVETimeChain : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  ModVectorElementFactory a;
  ModVectorElementFactory b;
  IntElementFactory thresholdFrame;
  const char* ClassID() const override { return "CHAN"; }
};

struct MVEBounce : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory planePoint;
  VectorElementFactory planeNormal;
  RealElementFactory friction;
  RealElementFactory restitution;
  BoolHelper dieOnPenetrate;
  const char* ClassID() const override { return "BNCE"; }
};

struct MVEConstant : IModVectorElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  RealElementFactory comps[3];
  const char* ClassID() const override { return "CNST"; }
};

struct MVEGravity : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory acceleration;
  const char* ClassID() const override { return "GRAV"; }
};

struct MVEExplode : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  RealElementFactory impulseMagnitude;
  RealElementFactory falloffFactor;
  const char* ClassID() const override { return "EXPL"; }
};

struct MVESetPosition : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory position;
  const char* ClassID() const override { return "SPOS"; }
};

struct MVELinearImplosion : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory implodePoint;
  RealElementFactory velocityScale;
  RealElementFactory maxRadius;
  RealElementFactory minRadius;
  BoolHelper enableMinRadius;
  const char* ClassID() const override { return "LMPL"; }
};

struct MVEPulse : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  IntElementFactory aDuration;
  IntElementFactory bDuration;
  ModVectorElementFactory a;
  ModVectorElementFactory b;
  const char* ClassID() const override { return "PULS"; }
};

struct MVEWind : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory windVelocity;
  RealElementFactory factor;
  const char* ClassID() const override { return "WIND"; }
};

struct MVESwirl : IModVectorElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory helixPoint;
  VectorElementFactory curveBinormal;
  RealElementFactory filterGain;
  RealElementFactory tangentialVelocity;
  const char* ClassID() const override { return "SWRL"; }
};

struct EESimpleEmitter : IEmitterElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory position;
  VectorElementFactory velocity;
  const char* ClassID() const override { return "SEMR"; }
};

struct VESphere : IEmitterElement {
  AT_DECL_DNA_YAMLV_NO_TYPE
  VectorElementFactory sphereOrigin;
  RealElementFactory sphereRadius;
  RealElementFactory magnitude;
  const char* ClassID() const override { return "SPHE"; }
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
  const char* ClassID() const override { return "ASPH"; }
};

struct EESimpleEmitterTR : EESimpleEmitter {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  const char* ClassID() const override { return "SETR"; }
};

template <class IDType>
struct UVEConstant : IUVElement {
  AT_DECL_EXPLICIT_DNA_YAMLV_NO_TYPE
  AT_SUBDECL_DNA
  CastIDToZero<IDType> tex;
  const char* ClassID() const override { return "CNST"; }

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
  const char* ClassID() const override { return "ATEX"; }

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
