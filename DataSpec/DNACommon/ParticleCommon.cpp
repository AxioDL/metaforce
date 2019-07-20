#include "ParticleCommon.hpp"

namespace DataSpec::DNAParticle {
logvisor::Module LogModule("urde::DNAParticle");

template <>
void REConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  val = r.readFloat(nullptr);
}
template <>
void REConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeFloat(nullptr, val);
}
template <>
void REConstant::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
}
template <>
void REConstant::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  val = r.readFloatBig();
}
template <>
void REConstant::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeFloatBig(val);
}

template <>
void IEConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  val = r.readUint32(nullptr);
}
template <>
void IEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeUint32(nullptr, val);
}
template <>
void IEConstant::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
}
template <>
void IEConstant::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  val = r.readUint32Big();
}
template <>
void IEConstant::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeUint32Big(val);
}

template <>
void VEConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  size_t elemCount;
  if (auto v = r.enterSubVector(nullptr, elemCount)) {
    for (size_t i = 0; i < 3 && i < elemCount; ++i) {
      if (auto rec = r.enterSubRecord(nullptr))
        comps[i].read(r);
    }
  }
}
template <>
void VEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector(nullptr))
    for (int i = 0; i < 3; ++i)
      if (auto rec = w.enterSubRecord(nullptr))
        comps[i].write(w);
}
template <>
void VEConstant::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  comps[0].binarySize(s);
  comps[1].binarySize(s);
  comps[2].binarySize(s);
}
template <>
void VEConstant::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  comps[0].read(r);
  comps[1].read(r);
  comps[2].read(r);
}
template <>
void VEConstant::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  comps[0].write(w);
  comps[1].write(w);
  comps[2].write(w);
}

template <>
void CEConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  for (int i = 0; i < 4; ++i)
    if (auto rec = r.enterSubRecord(nullptr))
      comps[i].read(r);
}
template <>
void CEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector(nullptr))
    for (int i = 0; i < 4; ++i)
      if (auto rec = w.enterSubRecord(nullptr))
        comps[i].write(w);
}
template <>
void CEConstant::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  comps[0].binarySize(s);
  comps[1].binarySize(s);
  comps[2].binarySize(s);
  comps[3].binarySize(s);
}
template <>
void CEConstant::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  comps[0].read(r);
  comps[1].read(r);
  comps[2].read(r);
  comps[3].read(r);
}
template <>
void CEConstant::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  comps[0].write(w);
  comps[1].write(w);
  comps[2].write(w);
  comps[3].write(w);
}

template <>
void MVEConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  for (int i = 0; i < 3; ++i)
    if (auto rec = r.enterSubRecord(nullptr))
      comps[i].read(r);
}
template <>
void MVEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector(nullptr))
    for (int i = 0; i < 3; ++i)
      if (auto rec = w.enterSubRecord(nullptr))
        comps[i].write(w);
}
template <>
void MVEConstant::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  comps[0].binarySize(s);
  comps[1].binarySize(s);
  comps[2].binarySize(s);
}
template <>
void MVEConstant::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  comps[0].read(r);
  comps[1].read(r);
  comps[2].read(r);
}
template <>
void MVEConstant::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  comps[0].write(w);
  comps[1].write(w);
  comps[2].write(w);
}

template <>
void RealElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('LFTW'):
    m_elem.reset(new struct RELifetimeTween);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct REConstant);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct RETimeChain);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct REAdd);
    break;
  case SBIG('CLMP'):
    m_elem.reset(new struct REClamp);
    break;
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct REKeyframeEmitter);
    break;
  case SBIG('IRND'):
    m_elem.reset(new struct REInitialRandom);
    break;
  case SBIG('RAND'):
    m_elem.reset(new struct RERandom);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct REMultiply);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct REPulse);
    break;
  case SBIG('SCAL'):
    m_elem.reset(new struct RETimeScale);
    break;
  case SBIG('RLPT'):
    m_elem.reset(new struct RELifetimePercent);
    break;
  case SBIG('SINE'):
    m_elem.reset(new struct RESineWave);
    break;
  case SBIG('ISWT'):
    m_elem.reset(new struct REInitialSwitch);
    break;
  case SBIG('CLTN'):
    m_elem.reset(new struct RECompareLessThan);
    break;
  case SBIG('CEQL'):
    m_elem.reset(new struct RECompareEquals);
    break;
  case SBIG('PAP1'):
    m_elem.reset(new struct REParticleAdvanceParam1);
    break;
  case SBIG('PAP2'):
    m_elem.reset(new struct REParticleAdvanceParam2);
    break;
  case SBIG('PAP3'):
    m_elem.reset(new struct REParticleAdvanceParam3);
    break;
  case SBIG('PAP4'):
    m_elem.reset(new struct REParticleAdvanceParam4);
    break;
  case SBIG('PAP5'):
    m_elem.reset(new struct REParticleAdvanceParam5);
    break;
  case SBIG('PAP6'):
    m_elem.reset(new struct REParticleAdvanceParam6);
    break;
  case SBIG('PAP7'):
    m_elem.reset(new struct REParticleAdvanceParam7);
    break;
  case SBIG('PAP8'):
    m_elem.reset(new struct REParticleAdvanceParam8);
    break;
  case SBIG('PSLL'):
    m_elem.reset(new struct REParticleSizeOrLineLength);
    break;
  case SBIG('PRLW'):
    m_elem.reset(new struct REParticleRotationOrLineWidth);
    break;
  case SBIG('SUB_'):
    m_elem.reset(new struct RESubtract);
    break;
  case SBIG('VMAG'):
    m_elem.reset(new struct REVectorMagnitude);
    break;
  case SBIG('VXTR'):
    m_elem.reset(new struct REVectorXToReal);
    break;
  case SBIG('VYTR'):
    m_elem.reset(new struct REVectorYToReal);
    break;
  case SBIG('VZTR'):
    m_elem.reset(new struct REVectorZToReal);
    break;
  case SBIG('CEXT'):
    m_elem.reset(new struct RECEXT);
    break;
  case SBIG('ITRL'):
    m_elem.reset(new struct REIntTimesReal);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void RealElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void RealElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void RealElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('LFTW'):
    m_elem.reset(new struct RELifetimeTween);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct REConstant);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct RETimeChain);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct REAdd);
    break;
  case SBIG('CLMP'):
    m_elem.reset(new struct REClamp);
    break;
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct REKeyframeEmitter);
    break;
  case SBIG('IRND'):
    m_elem.reset(new struct REInitialRandom);
    break;
  case SBIG('RAND'):
    m_elem.reset(new struct RERandom);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct REMultiply);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct REPulse);
    break;
  case SBIG('SCAL'):
    m_elem.reset(new struct RETimeScale);
    break;
  case SBIG('RLPT'):
    m_elem.reset(new struct RELifetimePercent);
    break;
  case SBIG('SINE'):
    m_elem.reset(new struct RESineWave);
    break;
  case SBIG('ISWT'):
    m_elem.reset(new struct REInitialSwitch);
    break;
  case SBIG('CLTN'):
    m_elem.reset(new struct RECompareLessThan);
    break;
  case SBIG('CEQL'):
    m_elem.reset(new struct RECompareEquals);
    break;
  case SBIG('PAP1'):
    m_elem.reset(new struct REParticleAdvanceParam1);
    break;
  case SBIG('PAP2'):
    m_elem.reset(new struct REParticleAdvanceParam2);
    break;
  case SBIG('PAP3'):
    m_elem.reset(new struct REParticleAdvanceParam3);
    break;
  case SBIG('PAP4'):
    m_elem.reset(new struct REParticleAdvanceParam4);
    break;
  case SBIG('PAP5'):
    m_elem.reset(new struct REParticleAdvanceParam5);
    break;
  case SBIG('PAP6'):
    m_elem.reset(new struct REParticleAdvanceParam6);
    break;
  case SBIG('PAP7'):
    m_elem.reset(new struct REParticleAdvanceParam7);
    break;
  case SBIG('PAP8'):
    m_elem.reset(new struct REParticleAdvanceParam8);
    break;
  case SBIG('PSLL'):
    m_elem.reset(new struct REParticleSizeOrLineLength);
    break;
  case SBIG('PRLW'):
    m_elem.reset(new struct REParticleRotationOrLineWidth);
    break;
  case SBIG('SUB_'):
    m_elem.reset(new struct RESubtract);
    break;
  case SBIG('VMAG'):
    m_elem.reset(new struct REVectorMagnitude);
    break;
  case SBIG('VXTR'):
    m_elem.reset(new struct REVectorXToReal);
    break;
  case SBIG('VYTR'):
    m_elem.reset(new struct REVectorYToReal);
    break;
  case SBIG('VZTR'):
    m_elem.reset(new struct REVectorZToReal);
    break;
  case SBIG('CEXT'):
    m_elem.reset(new struct RECEXT);
    break;
  case SBIG('ITRL'):
    m_elem.reset(new struct REIntTimesReal);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown RealElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void RealElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void IntElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct IEKeyframeEmitter);
    break;
  case SBIG('DETH'):
    m_elem.reset(new struct IEDeath);
    break;
  case SBIG('CLMP'):
    m_elem.reset(new struct IEClamp);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct IETimeChain);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct IEAdd);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct IEConstant);
    break;
  case SBIG('IMPL'):
    m_elem.reset(new struct IEImpulse);
    break;
  case SBIG('ILPT'):
    m_elem.reset(new struct IELifetimePercent);
    break;
  case SBIG('IRND'):
    m_elem.reset(new struct IEInitialRandom);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct IEPulse);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct IEMultiply);
    break;
  case SBIG('SPAH'):
    m_elem.reset(new struct IESampleAndHold);
    break;
  case SBIG('RAND'):
    m_elem.reset(new struct IERandom);
    break;
  case SBIG('TSCL'):
    m_elem.reset(new struct IETimeScale);
    break;
  case SBIG('GTCP'):
    m_elem.reset(new struct IEGTCP);
    break;
  case SBIG('MODU'):
    m_elem.reset(new struct IEModulo);
    break;
  case SBIG('SUB_'):
    m_elem.reset(new struct IESubtract);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void IntElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void IntElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void IntElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct IEKeyframeEmitter);
    break;
  case SBIG('DETH'):
    m_elem.reset(new struct IEDeath);
    break;
  case SBIG('CLMP'):
    m_elem.reset(new struct IEClamp);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct IETimeChain);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct IEAdd);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct IEConstant);
    break;
  case SBIG('IMPL'):
    m_elem.reset(new struct IEImpulse);
    break;
  case SBIG('ILPT'):
    m_elem.reset(new struct IELifetimePercent);
    break;
  case SBIG('IRND'):
    m_elem.reset(new struct IEInitialRandom);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct IEPulse);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct IEMultiply);
    break;
  case SBIG('SPAH'):
    m_elem.reset(new struct IESampleAndHold);
    break;
  case SBIG('RAND'):
    m_elem.reset(new struct IERandom);
    break;
  case SBIG('TSCL'):
    m_elem.reset(new struct IETimeScale);
    break;
  case SBIG('GTCP'):
    m_elem.reset(new struct IEGTCP);
    break;
  case SBIG('MODU'):
    m_elem.reset(new struct IEModulo);
    break;
  case SBIG('SUB_'):
    m_elem.reset(new struct IESubtract);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown IntElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void IntElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void VectorElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('CONE'):
    m_elem.reset(new struct VECone);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct VETimeChain);
    break;
  case SBIG('ANGC'):
    m_elem.reset(new struct VEAngleCone);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct VEAdd);
    break;
  case SBIG('CCLU'):
    m_elem.reset(new struct VECircleCluster);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct VEConstant);
    break;
  case SBIG('CIRC'):
    m_elem.reset(new struct VECircle);
    break;
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct VEKeyframeEmitter);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct VEMultiply);
    break;
  case SBIG('RTOV'):
    m_elem.reset(new struct VERealToVector);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct VEPulse);
    break;
  case SBIG('PVEL'):
    m_elem.reset(new struct VEParticleVelocity);
    break;
  case SBIG('SPOS'):
    m_elem.reset(new struct VESPOS);
    break;
  case SBIG('PLCO'):
    m_elem.reset(new struct VEPLCO);
    break;
  case SBIG('PLOC'):
    m_elem.reset(new struct VEPLOC);
    break;
  case SBIG('PSOR'):
    m_elem.reset(new struct VEPSOR);
    break;
  case SBIG('PSOF'):
    m_elem.reset(new struct VEPSOF);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void VectorElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void VectorElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void VectorElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('CONE'):
    m_elem.reset(new struct VECone);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct VETimeChain);
    break;
  case SBIG('ANGC'):
    m_elem.reset(new struct VEAngleCone);
    break;
  case SBIG('ADD_'):
    m_elem.reset(new struct VEAdd);
    break;
  case SBIG('CCLU'):
    m_elem.reset(new struct VECircleCluster);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct VEConstant);
    break;
  case SBIG('CIRC'):
    m_elem.reset(new struct VECircle);
    break;
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct VEKeyframeEmitter);
    break;
  case SBIG('MULT'):
    m_elem.reset(new struct VEMultiply);
    break;
  case SBIG('RTOV'):
    m_elem.reset(new struct VERealToVector);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct VEPulse);
    break;
  case SBIG('PVEL'):
    m_elem.reset(new struct VEParticleVelocity);
    break;
  case SBIG('SPOS'):
    m_elem.reset(new struct VESPOS);
    break;
  case SBIG('PLCO'):
    m_elem.reset(new struct VEPLCO);
    break;
  case SBIG('PLOC'):
    m_elem.reset(new struct VEPLOC);
    break;
  case SBIG('PSOR'):
    m_elem.reset(new struct VEPSOR);
    break;
  case SBIG('PSOF'):
    m_elem.reset(new struct VEPSOF);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown VectorElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void VectorElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void ColorElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct CEKeyframeEmitter);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct CEConstant);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct CETimeChain);
    break;
  case SBIG('CFDE'):
    m_elem.reset(new struct CEFadeEnd);
    break;
  case SBIG('FADE'):
    m_elem.reset(new struct CEFade);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct CEPulse);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void ColorElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void ColorElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void ColorElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('KEYE'):
  case SBIG('KEYP'):
    m_elem.reset(new struct CEKeyframeEmitter);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct CEConstant);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct CETimeChain);
    break;
  case SBIG('CFDE'):
    m_elem.reset(new struct CEFadeEnd);
    break;
  case SBIG('FADE'):
    m_elem.reset(new struct CEFade);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct CEPulse);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown ColorElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void ColorElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void ModVectorElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('IMPL'):
    m_elem.reset(new struct MVEImplosion);
    break;
  case SBIG('EMPL'):
    m_elem.reset(new struct MVEExponentialImplosion);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct MVETimeChain);
    break;
  case SBIG('BNCE'):
    m_elem.reset(new struct MVEBounce);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct MVEConstant);
    break;
  case SBIG('GRAV'):
    m_elem.reset(new struct MVEGravity);
    break;
  case SBIG('EXPL'):
    m_elem.reset(new struct MVEExplode);
    break;
  case SBIG('SPOS'):
    m_elem.reset(new struct MVESetPosition);
    break;
  case SBIG('LMPL'):
    m_elem.reset(new struct MVELinearImplosion);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct MVEPulse);
    break;
  case SBIG('WIND'):
    m_elem.reset(new struct MVEWind);
    break;
  case SBIG('SWRL'):
    m_elem.reset(new struct MVESwirl);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void ModVectorElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void ModVectorElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void ModVectorElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('IMPL'):
    m_elem.reset(new struct MVEImplosion);
    break;
  case SBIG('EMPL'):
    m_elem.reset(new struct MVEExponentialImplosion);
    break;
  case SBIG('CHAN'):
    m_elem.reset(new struct MVETimeChain);
    break;
  case SBIG('BNCE'):
    m_elem.reset(new struct MVEBounce);
    break;
  case SBIG('CNST'):
    m_elem.reset(new struct MVEConstant);
    break;
  case SBIG('GRAV'):
    m_elem.reset(new struct MVEGravity);
    break;
  case SBIG('EXPL'):
    m_elem.reset(new struct MVEExplode);
    break;
  case SBIG('SPOS'):
    m_elem.reset(new struct MVESetPosition);
    break;
  case SBIG('LMPL'):
    m_elem.reset(new struct MVELinearImplosion);
    break;
  case SBIG('PULS'):
    m_elem.reset(new struct MVEPulse);
    break;
  case SBIG('WIND'):
    m_elem.reset(new struct MVEWind);
    break;
  case SBIG('SWRL'):
    m_elem.reset(new struct MVESwirl);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown ModVectorElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void ModVectorElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void EmitterElementFactory::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  const auto& mapChildren = r.getCurNode()->m_mapChildren;
  if (mapChildren.empty()) {
    m_elem.reset();
    return;
  }

  const auto& elem = mapChildren[0];
  if (elem.first.size() < 4)
    LogModule.report(logvisor::Fatal, fmt("short FourCC in element '{}'"), elem.first);

  switch (*reinterpret_cast<const uint32_t*>(elem.first.data())) {
  case SBIG('SETR'):
    m_elem.reset(new struct EESimpleEmitterTR);
    break;
  case SBIG('SEMR'):
    m_elem.reset(new struct EESimpleEmitter);
    break;
  case SBIG('SPHE'):
    m_elem.reset(new struct VESphere);
    break;
  case SBIG('ASPH'):
    m_elem.reset(new struct VEAngleSphere);
    break;
  default:
    m_elem.reset();
    return;
  }
  if (auto rec = r.enterSubRecord(elem.first.c_str()))
    m_elem->read(r);
}

template <>
void EmitterElementFactory::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}

template <>
void EmitterElementFactory::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  if (m_elem)
    m_elem->binarySize(s);
}

template <>
void EmitterElementFactory::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  DNAFourCC clsId;
  clsId.read(r);
  switch (clsId.toUint32()) {
  case SBIG('SETR'):
    m_elem.reset(new struct EESimpleEmitterTR);
    break;
  case SBIG('SEMR'):
    m_elem.reset(new struct EESimpleEmitter);
    break;
  case SBIG('SPHE'):
    m_elem.reset(new struct VESphere);
    break;
  case SBIG('ASPH'):
    m_elem.reset(new struct VEAngleSphere);
    break;
  case SBIG('NONE'):
    m_elem.reset();
    return;
  default:
    m_elem.reset();
    LogModule.report(logvisor::Fatal, fmt("Unknown EmitterElement class {} @{}"), clsId, r.position());
    return;
  }
  m_elem->read(r);
}

template <>
void EmitterElementFactory::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

template <>
void BoolHelper::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  value = r.readBool(nullptr);
}
template <>
void BoolHelper::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeBool(nullptr, value);
}
template <>
void BoolHelper::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 5;
}
template <>
void BoolHelper::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('CNST'))
    value = r.readBool();
  else
    value = false;
}
template <>
void BoolHelper::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeBytes((atInt8*)"CNST", 4);
  w.writeBool(value);
}

template <>
void EESimpleEmitterTR::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  position.m_elem.reset();
  velocity.m_elem.reset();
  if (auto rec = r.enterSubRecord("ILOC"))
    position.read(r);
  if (auto rec = r.enterSubRecord("IVEC"))
    velocity.read(r);
}
template <>
void EESimpleEmitterTR::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto rec = w.enterSubRecord("ILOC"))
    position.write(w);
  if (auto rec = w.enterSubRecord("IVEC"))
    velocity.write(w);
}
template <>
void EESimpleEmitterTR::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 8;
  position.binarySize(s);
  velocity.binarySize(s);
}
template <>
void EESimpleEmitterTR::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  position.m_elem.reset();
  velocity.m_elem.reset();
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('ILOC')) {
    position.read(r);
    r.readBytesToBuf(&clsId, 4);
    if (clsId == SBIG('IVEC'))
      velocity.read(r);
  }
}
template <>
void EESimpleEmitterTR::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeBytes((atInt8*)"ILOC", 4);
  position.write(w);
  w.writeBytes((atInt8*)"IVEC", 4);
  velocity.write(w);
}

template <>
const char* UVEConstant<UniqueID32>::DNAType() {
  return "UVEConstant<UniqueID32>";
}
template <>
const char* UVEConstant<UniqueID64>::DNAType() {
  return "UVEConstant<UniqueID64>";
}

template <class IDType>
void UVEConstant<IDType>::_read(typename ReadYaml::StreamT& r) {
  tex.clear();
  if (auto rec = r.enterSubRecord("tex"))
    tex.read(r);
}
template <class IDType>
void UVEConstant<IDType>::_write(typename WriteYaml::StreamT& w) const {
  if (auto rec = w.enterSubRecord("tex"))
    tex.write(w);
}
template <class IDType>
void UVEConstant<IDType>::_binarySize(typename BinarySize::StreamT& _s) const {
  _s += 4;
  tex.binarySize(_s);
}
template <class IDType>
void UVEConstant<IDType>::_read(typename Read::StreamT& r) {
  tex.clear();
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('CNST'))
    tex.read(r);
}
template <class IDType>
void UVEConstant<IDType>::_write(typename Write::StreamT& w) const {
  w.writeBytes((atInt8*)"CNST", 4);
  tex.write(w);
}

AT_SUBSPECIALIZE_DNA_YAML(UVEConstant<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(UVEConstant<UniqueID64>)

template struct UVEConstant<UniqueID32>;
template struct UVEConstant<UniqueID64>;

template <>
const char* UVEAnimTexture<UniqueID32>::DNAType() {
  return "UVEAnimTexture<UniqueID32>";
}
template <>
const char* UVEAnimTexture<UniqueID64>::DNAType() {
  return "UVEAnimTexture<UniqueID64>";
}

template <class IDType>
void UVEAnimTexture<IDType>::_read(typename ReadYaml::StreamT& r) {
  tex.clear();
  if (auto rec = r.enterSubRecord("tex"))
    tex.read(r);
  if (auto rec = r.enterSubRecord("tileW"))
    tileW.read(r);
  if (auto rec = r.enterSubRecord("tileH"))
    tileH.read(r);
  if (auto rec = r.enterSubRecord("strideW"))
    strideW.read(r);
  if (auto rec = r.enterSubRecord("strideH"))
    strideH.read(r);
  if (auto rec = r.enterSubRecord("cycleFrames"))
    cycleFrames.read(r);
  if (auto rec = r.enterSubRecord("loop"))
    loop = r.readBool(nullptr);
}
template <class IDType>
void UVEAnimTexture<IDType>::_write(typename WriteYaml::StreamT& w) const {
  if (auto rec = w.enterSubRecord("tex"))
    tex.write(w);
  if (auto rec = w.enterSubRecord("tileW"))
    tileW.write(w);
  if (auto rec = w.enterSubRecord("tileH"))
    tileH.write(w);
  if (auto rec = w.enterSubRecord("strideW"))
    strideW.write(w);
  if (auto rec = w.enterSubRecord("strideH"))
    strideH.write(w);
  if (auto rec = w.enterSubRecord("cycleFrames"))
    cycleFrames.write(w);
  w.writeBool("loop", loop);
}
template <class IDType>
void UVEAnimTexture<IDType>::_binarySize(typename BinarySize::StreamT& _s) const {
  _s += 9;
  tex.binarySize(_s);
  tileW.binarySize(_s);
  tileH.binarySize(_s);
  strideW.binarySize(_s);
  strideH.binarySize(_s);
  cycleFrames.binarySize(_s);
}
template <class IDType>
void UVEAnimTexture<IDType>::_read(typename Read::StreamT& r) {
  tex.clear();
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('CNST'))
    tex.read(r);
  tileW.read(r);
  tileH.read(r);
  strideW.read(r);
  strideH.read(r);
  cycleFrames.read(r);
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('CNST'))
    loop = r.readBool();
}
template <class IDType>
void UVEAnimTexture<IDType>::_write(typename Write::StreamT& w) const {
  w.writeBytes((atInt8*)"CNST", 4);
  tex.write(w);
  tileW.write(w);
  tileH.write(w);
  strideW.write(w);
  strideH.write(w);
  cycleFrames.write(w);
  w.writeBytes((atInt8*)"CNST", 4);
  w.writeBool(loop);
}

AT_SUBSPECIALIZE_DNA_YAML(UVEAnimTexture<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(UVEAnimTexture<UniqueID64>)

template struct UVEAnimTexture<UniqueID32>;
template struct UVEAnimTexture<UniqueID64>;

template <>
const char* UVElementFactory<UniqueID32>::DNAType() {
  return "UVElementFactory<UniqueID32>";
}
template <>
const char* UVElementFactory<UniqueID64>::DNAType() {
  return "UVElementFactory<UniqueID64>";
}

template <class IDType>
void UVElementFactory<IDType>::_read(typename Read::StreamT& r) {
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  switch (clsId) {
  case SBIG('CNST'):
    m_elem.reset(new struct UVEConstant<IDType>);
    break;
  case SBIG('ATEX'):
    m_elem.reset(new struct UVEAnimTexture<IDType>);
    break;
  default:
    m_elem.reset();
    return;
  }
  m_elem->read(r);
}
template <class IDType>
void UVElementFactory<IDType>::_write(typename Write::StreamT& w) const {
  if (m_elem) {
    w.writeBytes((atInt8*)m_elem->ClassID(), 4);
    m_elem->write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}
template <class IDType>
void UVElementFactory<IDType>::_read(typename ReadYaml::StreamT& r) {
  if (auto rec = r.enterSubRecord("CNST")) {
    m_elem.reset(new struct UVEConstant<IDType>);
    m_elem->read(r);
  } else if (auto rec = r.enterSubRecord("ATEX")) {
    m_elem.reset(new struct UVEAnimTexture<IDType>);
    m_elem->read(r);
  } else
    m_elem.reset();
}
template <class IDType>
void UVElementFactory<IDType>::_write(typename WriteYaml::StreamT& w) const {
  if (m_elem)
    if (auto rec = w.enterSubRecord(m_elem->ClassID()))
      m_elem->write(w);
}
template <class IDType>
void UVElementFactory<IDType>::_binarySize(typename BinarySize::StreamT& _s) const {
  if (m_elem)
    m_elem->binarySize(_s);
  _s += 4;
}

AT_SUBSPECIALIZE_DNA_YAML(UVElementFactory<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(UVElementFactory<UniqueID64>)

template struct UVElementFactory<UniqueID32>;
template struct UVElementFactory<UniqueID64>;

template <>
const char* SpawnSystemKeyframeData<UniqueID32>::SpawnSystemKeyframeInfo::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID32>::SpawnSystemKeyframeInfo";
}
template <>
const char* SpawnSystemKeyframeData<UniqueID64>::SpawnSystemKeyframeInfo::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID64>::SpawnSystemKeyframeInfo";
}

template <class IDType>
template <class Op>
void SpawnSystemKeyframeData<IDType>::SpawnSystemKeyframeInfo::Enumerate(typename Op::StreamT& s) {
  Do<Op>({"id"}, id, s);
  Do<Op>({"a"}, a, s);
  Do<Op>({"b"}, b, s);
  Do<Op>({"c"}, c, s);
}

template <>
const char* SpawnSystemKeyframeData<UniqueID32>::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID32>";
}
template <>
const char* SpawnSystemKeyframeData<UniqueID64>::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID64>";
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_read(typename ReadYaml::StreamT& r) {
  if (auto rec = r.enterSubRecord("a"))
    a = r.readUint32(nullptr);
  if (auto rec = r.enterSubRecord("b"))
    b = r.readUint32(nullptr);
  if (auto rec = r.enterSubRecord("endFrame"))
    endFrame = r.readUint32(nullptr);
  if (auto rec = r.enterSubRecord("d"))
    d = r.readUint32(nullptr);
  spawns.clear();
  size_t spawnCount;
  if (auto v = r.enterSubVector("spawns", spawnCount)) {
    spawns.reserve(spawnCount);
    for (const auto& child : r.getCurNode()->m_seqChildren) {
      (void)child;
      if (auto rec = r.enterSubRecord(nullptr)) {
        spawns.emplace_back();
        spawns.back().first = r.readUint32("startFrame");
        size_t systemCount;
        if (auto v = r.enterSubVector("systems", systemCount)) {
          spawns.back().second.reserve(systemCount);
          for (const auto& in : r.getCurNode()->m_seqChildren) {
            (void)in;
            spawns.back().second.emplace_back();
            SpawnSystemKeyframeInfo& info = spawns.back().second.back();
            if (auto rec = r.enterSubRecord(nullptr))
              info.read(r);
          }
        }
      }
    }
  }
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_write(typename WriteYaml::StreamT& w) const {
  if (spawns.empty())
    return;
  w.writeUint32("a", a);
  w.writeUint32("b", b);
  w.writeUint32("endFrame", endFrame);
  w.writeUint32("d", d);
  if (auto v = w.enterSubVector("spawns")) {
    for (const auto& spawn : spawns) {
      if (auto rec = w.enterSubRecord(nullptr)) {
        w.writeUint32("startFrame", spawn.first);
        if (auto v = w.enterSubVector("systems"))
          for (const auto& info : spawn.second)
            if (auto rec = w.enterSubRecord(nullptr))
              info.write(w);
      }
    }
  }
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_binarySize(typename BinarySize::StreamT& s) const {
  s += 20;
  for (const auto& spawn : spawns) {
    s += 8;
    for (const auto& info : spawn.second)
      info.binarySize(s);
  }
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_read(typename Read::StreamT& r) {
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId != SBIG('CNST'))
    return;

  a = r.readUint32Big();
  b = r.readUint32Big();
  endFrame = r.readUint32Big();
  d = r.readUint32Big();
  uint32_t count = r.readUint32Big();
  spawns.clear();
  spawns.reserve(count);
  for (size_t i = 0; i < count; ++i) {
    spawns.emplace_back();
    spawns.back().first = r.readUint32Big();
    uint32_t infoCount = r.readUint32Big();
    spawns.back().second.reserve(infoCount);
    for (size_t j = 0; j < infoCount; ++j) {
      spawns.back().second.emplace_back();
      spawns.back().second.back().read(r);
    }
  }
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_write(typename Write::StreamT& w) const {
  if (spawns.empty()) {
    w.writeBytes((atInt8*)"NONE", 4);
    return;
  }
  w.writeBytes((atInt8*)"CNST", 4);
  w.writeUint32Big(a);
  w.writeUint32Big(b);
  w.writeUint32Big(endFrame);
  w.writeUint32Big(d);
  w.writeUint32Big(spawns.size());
  for (const auto& spawn : spawns) {
    w.writeUint32Big(spawn.first);
    w.writeUint32Big(spawn.second.size());
    for (const auto& info : spawn.second)
      info.write(w);
  }
}

AT_SUBSPECIALIZE_DNA_YAML(SpawnSystemKeyframeData<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(SpawnSystemKeyframeData<UniqueID64>)

template struct SpawnSystemKeyframeData<UniqueID32>;
template struct SpawnSystemKeyframeData<UniqueID64>;

template <>
const char* ChildResourceFactory<UniqueID32>::DNAType() {
  return "ChildResourceFactory<UniqueID32>";
}
template <>
const char* ChildResourceFactory<UniqueID64>::DNAType() {
  return "ChildResourceFactory<UniqueID64>";
}

template <class IDType>
void ChildResourceFactory<IDType>::_read(typename ReadYaml::StreamT& r) {
  id.clear();
  if (auto rec = r.enterSubRecord("CNST"))
    id.read(r);
}

template <class IDType>
void ChildResourceFactory<IDType>::_write(typename WriteYaml::StreamT& w) const {
  if (id.isValid())
    if (auto rec = w.enterSubRecord("CNST"))
      id.write(w);
}

template <class IDType>
void ChildResourceFactory<IDType>::_binarySize(typename BinarySize::StreamT& s) const {
  if (id.isValid())
    id.binarySize(s);
  s += 4;
}

template <class IDType>
void ChildResourceFactory<IDType>::_read(typename Read::StreamT& r) {
  id.clear();
  uint32_t clsId;
  r.readBytesToBuf(&clsId, 4);
  if (clsId == SBIG('CNST'))
    id.read(r);
}

template <class IDType>
void ChildResourceFactory<IDType>::_write(typename Write::StreamT& w) const {
  if (id.isValid()) {
    w.writeBytes((atInt8*)"CNST", 4);
    id.write(w);
  } else
    w.writeBytes((atInt8*)"NONE", 4);
}

AT_SUBSPECIALIZE_DNA_YAML(ChildResourceFactory<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(ChildResourceFactory<UniqueID64>)

template struct ChildResourceFactory<UniqueID32>;
template struct ChildResourceFactory<UniqueID64>;

} // namespace DataSpec::DNAParticle
