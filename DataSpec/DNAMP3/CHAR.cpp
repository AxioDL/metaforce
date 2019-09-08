#include "CHAR.hpp"

namespace DataSpec::DNAMP3 {

template <>
void CHAR::AnimationInfo::EVNT::SFXEvent::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  EventBase::read(reader);
  caudId.read(reader);
  unk1 = reader.readUint32Big();
  unk2 = reader.readUint32Big();
  unk3 = reader.readUint32Big();
  reader.enumerateBig(unk3Vals, unk3);
  extraType = reader.readUint32Big();
  if (extraType == 1)
    extraFloat = reader.readFloatBig();
  else if (extraType == 2)
    reader.seek(35, athena::SeekOrigin::Current);
}

template <>
void CHAR::AnimationInfo::EVNT::SFXEvent::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  EventBase::write(writer);
  caudId.write(writer);
  writer.writeUint32Big(unk1);
  writer.writeUint32Big(unk2);
  writer.writeUint32Big(unk3);
  writer.enumerateBig(unk3Vals);
  writer.writeUint32Big(extraType);
  if (extraType == 1)
    writer.writeFloatBig(extraFloat);
  else if (extraType == 2)
    writer.seek(35, athena::SeekOrigin::Current);
}

template <>
void CHAR::AnimationInfo::EVNT::SFXEvent::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  EventBase::binarySize(__isz);
  caudId.binarySize(__isz);
  __isz += 16;
  __isz += unk3Vals.size() * 4;
  if (extraType == 1)
    __isz += 4;
  else if (extraType == 2)
    __isz += 35;
}

template <>
void CHAR::AnimationInfo::EVNT::SFXEvent::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  EventBase::read(reader);
  reader.enumerate("caudId", caudId);
  unk1 = reader.readUint32("unk1");
  unk2 = reader.readUint32("unk2");
  unk3 = reader.enumerate("unk3Vals", unk3Vals);
  extraType = reader.readUint32("extraType");
  if (extraType == 1)
    extraFloat = reader.readFloat("extraFloat");
}

template <>
void CHAR::AnimationInfo::EVNT::SFXEvent::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  EventBase::write(writer);
  writer.enumerate("caudId", caudId);
  writer.writeUint32("unk1", unk1);
  writer.writeUint32("unk2", unk2);
  writer.enumerate("unk3Vals", unk3Vals);
  writer.writeUint32("extraType", extraType);
  if (extraType == 1)
    writer.writeFloat("extraFloat", extraFloat);
}

const char* CHAR::AnimationInfo::EVNT::SFXEvent::DNAType() {
  return "urde::DNAMP3::CHAR::AnimationInfo::EVNT::SFXEvent";
}

template <>
void CHAR::AnimationInfo::MetaAnimFactory::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  IMetaAnim::Type type(IMetaAnim::Type(reader.readUint32Big()));
  switch (type) {
  case IMetaAnim::Type::Primitive:
    m_anim.reset(new struct MetaAnimPrimitive);
    m_anim->read(reader);
    break;
  case IMetaAnim::Type::Blend:
    m_anim.reset(new struct MetaAnimBlend);
    m_anim->read(reader);
    break;
  case IMetaAnim::Type::PhaseBlend:
    m_anim.reset(new struct MetaAnimPhaseBlend);
    m_anim->read(reader);
    break;
  case IMetaAnim::Type::Random:
    m_anim.reset(new struct MetaAnimRandom);
    m_anim->read(reader);
    break;
  case IMetaAnim::Type::Sequence:
    m_anim.reset(new struct MetaAnimSequence);
    m_anim->read(reader);
    break;
  default:
    m_anim.reset(nullptr);
    break;
  }
}

template <>
void CHAR::AnimationInfo::MetaAnimFactory::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  if (!m_anim)
    return;
  writer.writeInt32Big(atInt32(m_anim->m_type));
  m_anim->write(writer);
}

template <>
void CHAR::AnimationInfo::MetaAnimFactory::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  if (!m_anim)
    return;
  __isz += 4;
  m_anim->binarySize(__isz);
}

template <>
void CHAR::AnimationInfo::MetaAnimFactory::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  std::string type = reader.readString("type");
  std::transform(type.begin(), type.end(), type.begin(), tolower);
  if (!type.compare("primitive")) {
    m_anim.reset(new struct MetaAnimPrimitive);
    m_anim->read(reader);
  } else if (!type.compare("blend")) {
    m_anim.reset(new struct MetaAnimBlend);
    m_anim->read(reader);
  } else if (!type.compare("phaseblend")) {
    m_anim.reset(new struct MetaAnimPhaseBlend);
    m_anim->read(reader);
  } else if (!type.compare("random")) {
    m_anim.reset(new struct MetaAnimRandom);
    m_anim->read(reader);
  } else if (!type.compare("sequence")) {
    m_anim.reset(new struct MetaAnimSequence);
    m_anim->read(reader);
  } else {
    m_anim.reset(nullptr);
  }
}

template <>
void CHAR::AnimationInfo::MetaAnimFactory::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  if (!m_anim)
    return;
  writer.writeString("type", m_anim->m_typeStr);
  m_anim->write(writer);
}

const char* CHAR::AnimationInfo::MetaAnimFactory::DNAType() {
  return "urde::DNAMP3::CHAR::AnimationInfo::MetaAnimFactory";
}

} // namespace DataSpec::DNAMP3
