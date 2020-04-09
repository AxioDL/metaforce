#pragma once

#include <memory>
#include <vector>

#include "DataSpec/DNACommon/DNACommon.hpp"

#include <logvisor/logvisor.hpp>

namespace DataSpec::DNAParticle {
extern logvisor::Module LogModule;

enum class ParticleType {
  GPSM = SBIG('GPSM'),
  SWSH = SBIG('SWSH'),
  ELSM = SBIG('ELSM'),
  DPSM = SBIG('DPSM'),
  CRSM = SBIG('CRSM'),
  WPSM = SBIG('WPSM')
};

/*
 * The particle property (PP) metaclass system provides common compile-time utilities
 * for storing, enumerating, and streaming particle scripts.
 */

template <class _Basis>
struct PPImpl : BigDNA, _Basis {
  AT_DECL_EXPLICIT_DNA_YAML

  template<typename T>
  static constexpr bool _shouldStore(T& p, bool defaultBool) {
    if constexpr (std::is_same_v<T, bool>) {
      return p != defaultBool;
    } else if constexpr (std::is_same_v<T, uint32_t>) {
      return p != 0xffffffff;
    } else if constexpr (std::is_same_v<T, float>) {
      return true;
    } else {
      return p.operator bool();
    }
  }

  constexpr void _read(athena::io::IStreamReader& r) {
    constexpr FourCC RefType = uint32_t(_Basis::Type);
    DNAFourCC clsId(r);
    if (clsId != RefType) {
      LogModule.report(logvisor::Warning, fmt("non {} provided to {} parser"), RefType, RefType);
      return;
    }
    clsId.read(r);
    while (clsId != SBIG('_END')) {
      if (!_Basis::Lookup(clsId, [&](auto& p) {
        using Tp = std::decay_t<decltype(p)>;
        if constexpr (std::is_same_v<Tp, bool>) {
          DNAFourCC tp(r);
          if (tp == SBIG('CNST'))
            p = r.readBool();
        } else if constexpr (std::is_same_v<Tp, uint32_t>) {
          DNAFourCC tp(r);
          if (tp == SBIG('CNST'))
            p = r.readUint32Big();
        } else if constexpr (std::is_same_v<Tp, float>) {
          DNAFourCC tp(r);
          if (tp == SBIG('CNST'))
            p = r.readFloatBig();
        } else {
          p.read(r);
        }
      })) {
        LogModule.report(logvisor::Fatal, fmt("Unknown {} class {} @{}"), RefType, clsId, r.position());
      }
      clsId.read(r);
    }
  }

  constexpr void _write(athena::io::IStreamWriter& w) {
    constexpr DNAFourCC RefType = uint32_t(_Basis::Type);
    RefType.write(w);
    _Basis::Enumerate([&](FourCC fcc, auto& p, bool defaultBool = false) {
      if (_shouldStore(p, defaultBool)) {
        using Tp = std::decay_t<decltype(p)>;
        DNAFourCC(fcc).write(w);
        if constexpr (std::is_same_v<Tp, bool>) {
          w.writeBytes("CNST", 4);
          w.writeBool(p);
        } else if constexpr (std::is_same_v<Tp, uint32_t>) {
          w.writeBytes("CNST", 4);
          w.writeUint32Big(p);
        } else if constexpr (std::is_same_v<Tp, float>) {
          w.writeBytes("CNST", 4);
          w.writeFloatBig(p);
        } else {
          p.write(w);
        }
      }
    });
    w.writeBytes("_END", 4);
  }

  constexpr void _binarySize(std::size_t& s) {
    constexpr DNAFourCC RefType = uint32_t(_Basis::Type);
    RefType.binarySize(s);
    _Basis::Enumerate([&](FourCC fcc, auto& p, bool defaultBool = false) {
      if (_shouldStore(p, defaultBool)) {
        using Tp = std::decay_t<decltype(p)>;
        DNAFourCC(fcc).binarySize(s);
        if constexpr (std::is_same_v<Tp, bool>) {
          s += 5;
        } else if constexpr (std::is_same_v<Tp, uint32_t> || std::is_same_v<Tp, float>) {
          s += 8;
        } else {
          p.binarySize(s);
        }
      }
    });
    s += 4;
  }

  void _read(athena::io::YAMLDocReader& r) {
    constexpr DNAFourCC RefType = uint32_t(_Basis::Type);

    for (const auto& [key, value] : r.getCurNode()->m_mapChildren) {
      if (key == "DNAType"sv)
        continue;
      if (key.size() < 4) {
        LogModule.report(logvisor::Warning, fmt("short FourCC in element '{}'"), key);
        continue;
      }

      if (auto rec = r.enterSubRecord(key)) {
        const DNAFourCC clsId = key.c_str();
        if (!_Basis::Lookup(clsId, [&](auto& p) {
          using Tp = std::decay_t<decltype(p)>;
          if constexpr (std::is_same_v<Tp, bool>) {
            p = r.readBool();
          } else if constexpr (std::is_same_v<Tp, uint32_t>) {
            p = r.readUint32();
          } else if constexpr (std::is_same_v<Tp, float>) {
            p = r.readFloat();
          } else {
            p.read(r);
          }
        })) {
          LogModule.report(logvisor::Fatal, fmt("Unknown {} class {}"), RefType, clsId);
        }
      }
    }
  }

  constexpr void _write(athena::io::YAMLDocWriter& w) {
    _Basis::Enumerate([&](FourCC fcc, auto& p, bool defaultBool = false) {
      if (_shouldStore(p, defaultBool)) {
        using Tp = std::decay_t<decltype(p)>;
        if (auto rec = w.enterSubRecord(fcc.toStringView())) {
          if constexpr (std::is_same_v<Tp, bool>) {
            w.writeBool(p);
          } else if constexpr (std::is_same_v<Tp, uint32_t>) {
            w.writeUint32(p);
          } else if constexpr (std::is_same_v<Tp, float>) {
            w.writeFloat(p);
          } else {
            p.write(w);
          }
        }
      }
    });
  }

  constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& deps) {
    _Basis::Enumerate([&](FourCC fcc, auto& p, bool defaultBool = false) {
      using Tp = std::decay_t<decltype(p)>;
      if constexpr (!std::is_same_v<Tp, bool> && !std::is_same_v<Tp, uint32_t> && !std::is_same_v<Tp, float>)
        p.gatherDependencies(deps);
    });
  }

  constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& deps) const {
    const_cast<PPImpl&>(*this).gatherDependencies(deps);
  }
};

template <typename _Type>
struct PEType {
  using Type = _Type;
};

template <class _Basis>
struct PEImpl : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML
  using _PtrType = typename _Basis::PtrType;

  void _read(athena::io::IStreamReader& r) {
    DNAFourCC clsId(r);
    if (clsId == FOURCC('NONE')) {
      m_elem.reset();
      return;
    }
    if (!_Basis::Lookup(clsId, [&](auto&& p) {
      using Tp = std::decay_t<decltype(p)>;
      m_elem = std::make_unique<typename Tp::Type>();
      m_elem->read(r);
    })) {
      LogModule.report(logvisor::Fatal, fmt("Unknown {} class {} @{}"), _PtrType::TypeName, clsId, r.position());
    }
  }

  void _write(athena::io::IStreamWriter& w) {
    if (m_elem) {
      w.writeBytes(m_elem->ClassID().data(), 4);
      m_elem->write(w);
    } else {
      w.writeBytes("NONE", 4);
    }
  }

  void _binarySize(std::size_t& s) {
    if (m_elem)
      m_elem->binarySize(s);
    s += 4;
  }

  void _read(athena::io::YAMLDocReader& r) {
    const auto& mapChildren = r.getCurNode()->m_mapChildren;
    if (mapChildren.empty()) {
      m_elem.reset();
      return;
    }

    const auto& [key, value] = mapChildren[0];
    if (key.size() < 4)
      LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), key);

    if (auto rec = r.enterSubRecord(key)) {
      const DNAFourCC clsId = key.c_str();
      if (!_Basis::Lookup(clsId, [&](auto&& p) {
        using Tp = std::decay_t<decltype(p)>;
        m_elem = std::make_unique<typename Tp::Type>();
        m_elem->read(r);
      })) {
        LogModule.report(logvisor::Fatal, fmt("Unknown {} class {}"), _PtrType::TypeName, clsId);
      }
    }
  }

  void _write(athena::io::YAMLDocWriter& w) {
    if (m_elem)
      if (auto rec = w.enterSubRecord(m_elem->ClassID()))
        m_elem->write(w);
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& deps) const {
    _Basis::gatherDependencies(deps, m_elem);
  }

  operator bool() const { return m_elem.operator bool(); }
  auto* get() const { return m_elem.get(); }
  auto* operator->() const { return get(); }
  void reset() { m_elem.reset(); }
private:
  std::unique_ptr<_PtrType> m_elem;
};

struct IElement : BigDNAVYaml {
  Delete _d;
  ~IElement() override = default;
  virtual std::string_view ClassID() const = 0;
  std::string_view DNATypeV() const override { return ClassID(); }
};

struct IRealElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "RealElement"sv;
};
struct RELifetimeTween;
struct REConstant;
struct RETimeChain;
struct REAdd;
struct REClamp;
struct REKeyframeEmitter;
struct REKeyframeEmitter;
struct REInitialRandom;
struct RERandom;
struct REMultiply;
struct REPulse;
struct RETimeScale;
struct RELifetimePercent;
struct RESineWave;
struct REInitialSwitch;
struct RECompareLessThan;
struct RECompareEquals;
struct REParticleAdvanceParam1;
struct REParticleAdvanceParam2;
struct REParticleAdvanceParam3;
struct REParticleAdvanceParam4;
struct REParticleAdvanceParam5;
struct REParticleAdvanceParam6;
struct REParticleAdvanceParam7;
struct REParticleAdvanceParam8;
struct REParticleSizeOrLineLength;
struct REParticleRotationOrLineWidth;
struct RESubtract;
struct REVectorMagnitude;
struct REVectorXToReal;
struct REVectorYToReal;
struct REVectorZToReal;
struct RECEXT;
struct REIntTimesReal;
struct _RealElementFactory {
  using PtrType = IRealElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('LFTW'): f(PEType<RELifetimeTween>{}); return true;
    case SBIG('CNST'): f(PEType<REConstant>{}); return true;
    case SBIG('CHAN'): f(PEType<RETimeChain>{}); return true;
    case SBIG('ADD_'): f(PEType<REAdd>{}); return true;
    case SBIG('CLMP'): f(PEType<REClamp>{}); return true;
    case SBIG('KEYE'): f(PEType<REKeyframeEmitter>{}); return true;
    case SBIG('KEYP'): f(PEType<REKeyframeEmitter>{}); return true;
    case SBIG('IRND'): f(PEType<REInitialRandom>{}); return true;
    case SBIG('RAND'): f(PEType<RERandom>{}); return true;
    case SBIG('MULT'): f(PEType<REMultiply>{}); return true;
    case SBIG('PULS'): f(PEType<REPulse>{}); return true;
    case SBIG('SCAL'): f(PEType<RETimeScale>{}); return true;
    case SBIG('RLPT'): f(PEType<RELifetimePercent>{}); return true;
    case SBIG('SINE'): f(PEType<RESineWave>{}); return true;
    case SBIG('ISWT'): f(PEType<REInitialSwitch>{}); return true;
    case SBIG('CLTN'): f(PEType<RECompareLessThan>{}); return true;
    case SBIG('CEQL'): f(PEType<RECompareEquals>{}); return true;
    case SBIG('PAP1'): f(PEType<REParticleAdvanceParam1>{}); return true;
    case SBIG('PAP2'): f(PEType<REParticleAdvanceParam2>{}); return true;
    case SBIG('PAP3'): f(PEType<REParticleAdvanceParam3>{}); return true;
    case SBIG('PAP4'): f(PEType<REParticleAdvanceParam4>{}); return true;
    case SBIG('PAP5'): f(PEType<REParticleAdvanceParam5>{}); return true;
    case SBIG('PAP6'): f(PEType<REParticleAdvanceParam6>{}); return true;
    case SBIG('PAP7'): f(PEType<REParticleAdvanceParam7>{}); return true;
    case SBIG('PAP8'): f(PEType<REParticleAdvanceParam8>{}); return true;
    case SBIG('PSLL'): f(PEType<REParticleSizeOrLineLength>{}); return true;
    case SBIG('PRLW'): f(PEType<REParticleRotationOrLineWidth>{}); return true;
    case SBIG('SUB_'): f(PEType<RESubtract>{}); return true;
    case SBIG('VMAG'): f(PEType<REVectorMagnitude>{}); return true;
    case SBIG('VXTR'): f(PEType<REVectorXToReal>{}); return true;
    case SBIG('VYTR'): f(PEType<REVectorYToReal>{}); return true;
    case SBIG('VZTR'): f(PEType<REVectorZToReal>{}); return true;
    case SBIG('CEXT'): f(PEType<RECEXT>{}); return true;
    case SBIG('ITRL'): f(PEType<REIntTimesReal>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IRealElement>& elemPtr) {}
};
using RealElementFactory = PEImpl<_RealElementFactory>;

struct IIntElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "IntElement"sv;
};
struct IEKeyframeEmitter;
struct IEKeyframeEmitter;
struct IEDeath;
struct IEClamp;
struct IETimeChain;
struct IEAdd;
struct IEConstant;
struct IEImpulse;
struct IELifetimePercent;
struct IEInitialRandom;
struct IEPulse;
struct IEMultiply;
struct IESampleAndHold;
struct IERandom;
struct IETimeScale;
struct IEGTCP;
struct IEModulo;
struct IESubtract;
struct _IntElementFactory {
  using PtrType = IIntElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('KEYE'): f(PEType<IEKeyframeEmitter>{}); return true;
    case SBIG('KEYP'): f(PEType<IEKeyframeEmitter>{}); return true;
    case SBIG('DETH'): f(PEType<IEDeath>{}); return true;
    case SBIG('CLMP'): f(PEType<IEClamp>{}); return true;
    case SBIG('CHAN'): f(PEType<IETimeChain>{}); return true;
    case SBIG('ADD_'): f(PEType<IEAdd>{}); return true;
    case SBIG('CNST'): f(PEType<IEConstant>{}); return true;
    case SBIG('IMPL'): f(PEType<IEImpulse>{}); return true;
    case SBIG('ILPT'): f(PEType<IELifetimePercent>{}); return true;
    case SBIG('IRND'): f(PEType<IEInitialRandom>{}); return true;
    case SBIG('PULS'): f(PEType<IEPulse>{}); return true;
    case SBIG('MULT'): f(PEType<IEMultiply>{}); return true;
    case SBIG('SPAH'): f(PEType<IESampleAndHold>{}); return true;
    case SBIG('RAND'): f(PEType<IERandom>{}); return true;
    case SBIG('TSCL'): f(PEType<IETimeScale>{}); return true;
    case SBIG('GTCP'): f(PEType<IEGTCP>{}); return true;
    case SBIG('MODU'): f(PEType<IEModulo>{}); return true;
    case SBIG('SUB_'): f(PEType<IESubtract>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IIntElement>& elemPtr) {}
};
using IntElementFactory = PEImpl<_IntElementFactory>;

struct IVectorElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "VectorElement"sv;
};
struct VECone;
struct VETimeChain;
struct VEAngleCone;
struct VEAdd;
struct VECircleCluster;
struct VEConstant;
struct VECircle;
struct VEKeyframeEmitter;
struct VEKeyframeEmitter;
struct VEMultiply;
struct VERealToVector;
struct VEPulse;
struct VEParticleVelocity;
struct VESPOS;
struct VEPLCO;
struct VEPLOC;
struct VEPSOR;
struct VEPSOF;
struct _VectorElementFactory {
  using PtrType = IVectorElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('CONE'): f(PEType<VECone>{}); return true;
    case SBIG('CHAN'): f(PEType<VETimeChain>{}); return true;
    case SBIG('ANGC'): f(PEType<VEAngleCone>{}); return true;
    case SBIG('ADD_'): f(PEType<VEAdd>{}); return true;
    case SBIG('CCLU'): f(PEType<VECircleCluster>{}); return true;
    case SBIG('CNST'): f(PEType<VEConstant>{}); return true;
    case SBIG('CIRC'): f(PEType<VECircle>{}); return true;
    case SBIG('KEYE'): f(PEType<VEKeyframeEmitter>{}); return true;
    case SBIG('KEYP'): f(PEType<VEKeyframeEmitter>{}); return true;
    case SBIG('MULT'): f(PEType<VEMultiply>{}); return true;
    case SBIG('RTOV'): f(PEType<VERealToVector>{}); return true;
    case SBIG('PULS'): f(PEType<VEPulse>{}); return true;
    case SBIG('PVEL'): f(PEType<VEParticleVelocity>{}); return true;
    case SBIG('SPOS'): f(PEType<VESPOS>{}); return true;
    case SBIG('PLCO'): f(PEType<VEPLCO>{}); return true;
    case SBIG('PLOC'): f(PEType<VEPLOC>{}); return true;
    case SBIG('PSOR'): f(PEType<VEPSOR>{}); return true;
    case SBIG('PSOF'): f(PEType<VEPSOF>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IVectorElement>& elemPtr) {}
};
using VectorElementFactory = PEImpl<_VectorElementFactory>;

struct IColorElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "ColorElement"sv;
};
struct CEKeyframeEmitter;
struct CEKeyframeEmitter;
struct CEConstant;
struct CETimeChain;
struct CEFadeEnd;
struct CEFade;
struct CEPulse;
struct _ColorElementFactory {
  using PtrType = IColorElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('KEYE'): f(PEType<CEKeyframeEmitter>{}); return true;
    case SBIG('KEYP'): f(PEType<CEKeyframeEmitter>{}); return true;
    case SBIG('CNST'): f(PEType<CEConstant>{}); return true;
    case SBIG('CHAN'): f(PEType<CETimeChain>{}); return true;
    case SBIG('CFDE'): f(PEType<CEFadeEnd>{}); return true;
    case SBIG('FADE'): f(PEType<CEFade>{}); return true;
    case SBIG('PULS'): f(PEType<CEPulse>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IColorElement>& elemPtr) {}
};
using ColorElementFactory = PEImpl<_ColorElementFactory>;

struct IModVectorElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "ModVectorElement"sv;
};
struct MVEImplosion;
struct MVEExponentialImplosion;
struct MVETimeChain;
struct MVEBounce;
struct MVEConstant;
struct MVEGravity;
struct MVEExplode;
struct MVESetPosition;
struct MVELinearImplosion;
struct MVEPulse;
struct MVEWind;
struct MVESwirl;
struct _ModVectorElementFactory {
  using PtrType = IModVectorElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('IMPL'): f(PEType<MVEImplosion>{}); return true;
    case SBIG('EMPL'): f(PEType<MVEExponentialImplosion>{}); return true;
    case SBIG('CHAN'): f(PEType<MVETimeChain>{}); return true;
    case SBIG('BNCE'): f(PEType<MVEBounce>{}); return true;
    case SBIG('CNST'): f(PEType<MVEConstant>{}); return true;
    case SBIG('GRAV'): f(PEType<MVEGravity>{}); return true;
    case SBIG('EXPL'): f(PEType<MVEExplode>{}); return true;
    case SBIG('SPOS'): f(PEType<MVESetPosition>{}); return true;
    case SBIG('LMPL'): f(PEType<MVELinearImplosion>{}); return true;
    case SBIG('PULS'): f(PEType<MVEPulse>{}); return true;
    case SBIG('WIND'): f(PEType<MVEWind>{}); return true;
    case SBIG('SWRL'): f(PEType<MVESwirl>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IModVectorElement>& elemPtr) {}
};
using ModVectorElementFactory = PEImpl<_ModVectorElementFactory>;

struct IEmitterElement : IElement {
  Delete _d2;
  static constexpr std::string_view TypeName = "EmitterElement"sv;
};
struct EESimpleEmitterTR;
struct EESimpleEmitter;
struct VESphere;
struct VEAngleSphere;
struct _EmitterElementFactory {
  using PtrType = IEmitterElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('SETR'): f(PEType<EESimpleEmitterTR>{}); return true;
    case SBIG('SEMR'): f(PEType<EESimpleEmitter>{}); return true;
    case SBIG('SPHE'): f(PEType<VESphere>{}); return true;
    case SBIG('ASPH'): f(PEType<VEAngleSphere>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IEmitterElement>& elemPtr) {}
};
using EmitterElementFactory = PEImpl<_EmitterElementFactory>;

struct IUVElement : IElement {
  Delete _d2;
  virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const = 0;
  static constexpr std::string_view TypeName = "UVElement"sv;
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

template <typename Tp>
struct ValueHelper : BigDNA {
  AT_DECL_EXPLICIT_DNA_YAML

  void _read(athena::io::IStreamReader& r) {
    hecl::DNAFourCC ValueType;
    ValueType.read(r);
    if (ValueType == FOURCC('CNST'))
      athena::io::Read<athena::io::PropType::None>::Do<Tp, athena::Endian::Big>({}, value.emplace(), r);
    else
      value = std::nullopt;
  }
  void _write(athena::io::IStreamWriter& w) {
    if (value) {
      w.writeBytes("CNST", 4);
      athena::io::Write<athena::io::PropType::None>::Do<Tp, athena::Endian::Big>({}, *value, w);
    } else {
      w.writeBytes("NONE", 4);
    }
  }
  void _binarySize(std::size_t& s) {
    s += 4;
    if (value)
      athena::io::BinarySize<athena::io::PropType::None>::Do<Tp, athena::Endian::Big>({}, *value, s);
  }
  void _read(athena::io::YAMLDocReader& r) {
    athena::io::ReadYaml<athena::io::PropType::None>::Do<Tp, athena::Endian::Big>({}, value.emplace(), r);
  }
  void _write(athena::io::YAMLDocWriter& w) {
    athena::io::WriteYaml<athena::io::PropType::None>::Do<Tp, athena::Endian::Big>({}, *value, w);
  }

  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) {}

  std::optional<Tp> value = {};
  void emplace(Tp val) { value.emplace(val); }
  Tp operator*() const { return *value; }
  operator bool() const { return value.operator bool(); }
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
    if (tex.isValid())
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
    if (tex.isValid())
      g_curSpec->flattenDependencies(tex, pathsOut);
  }
};

template <class IDType>
struct _UVElementFactory {
  using PtrType = IUVElement;
  template<typename _Func>
  static bool constexpr Lookup(FourCC fcc, _Func f) {
    switch (fcc.toUint32()) {
    case SBIG('CNST'): f(PEType<UVEConstant<IDType>>{}); return true;
    case SBIG('ATEX'): f(PEType<UVEAnimTexture<IDType>>{}); return true;
    default: return false;
    }
  }
  static constexpr void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                                           const std::unique_ptr<IUVElement>& elemPtr) {
    if (elemPtr)
      elemPtr->gatherDependencies(pathsOut);
  }
};
template <class IDType>
using UVElementFactory = PEImpl<_UVElementFactory<IDType>>;

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
  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
    if (id.isValid())
      g_curSpec->flattenDependencies(id, pathsOut);
  }
};

} // namespace DataSpec::DNAParticle
