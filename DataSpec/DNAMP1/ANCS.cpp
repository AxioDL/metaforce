#include "ANCS.hpp"
#include "hecl/Blender/Connection.hpp"

namespace DataSpec {
extern hecl::Database::DataSpecEntry SpecEntMP1;
extern hecl::Database::DataSpecEntry SpecEntMP1PC;

namespace DNAMP1 {

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::Enumerate<BigDNA::Read>(
    athena::io::IStreamReader& reader) {
  parmType = reader.readUint32Big();
  weightFunction = reader.readUint32Big();
  weight = reader.readFloatBig();
  switch (DataType(parmType)) {
  case DataType::Int32:
    range[0].int32 = reader.readInt32Big();
    range[1].int32 = reader.readInt32Big();
    break;
  case DataType::UInt32:
  case DataType::Enum:
    range[0].uint32 = reader.readUint32Big();
    range[1].uint32 = reader.readUint32Big();
    break;
  case DataType::Float:
    range[0].float32 = reader.readFloatBig();
    range[1].float32 = reader.readFloatBig();
    break;
  case DataType::Bool:
    range[0].bool1 = reader.readBool();
    range[1].bool1 = reader.readBool();
    break;
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::Enumerate<BigDNA::Write>(
    athena::io::IStreamWriter& writer) {
  writer.writeUint32Big(parmType);
  writer.writeUint32Big(weightFunction);
  writer.writeFloatBig(weight);
  switch (DataType(parmType)) {
  case DataType::Int32:
    writer.writeInt32Big(range[0].int32);
    writer.writeInt32Big(range[1].int32);
    break;
  case DataType::UInt32:
  case DataType::Enum:
    writer.writeUint32Big(range[0].uint32);
    writer.writeUint32Big(range[1].uint32);
    break;
  case DataType::Float:
    writer.writeFloatBig(range[0].float32);
    writer.writeFloatBig(range[1].float32);
    break;
  case DataType::Bool:
    writer.writeBool(range[0].bool1);
    writer.writeBool(range[1].bool1);
    break;
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 12;
  switch (DataType(parmType)) {
  case DataType::Int32:
  case DataType::UInt32:
  case DataType::Enum:
  case DataType::Float:
    __isz += 8;
    break;
  case DataType::Bool:
    __isz += 2;
    break;
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::Enumerate<BigDNA::ReadYaml>(
    athena::io::YAMLDocReader& reader) {
  parmType = reader.readUint32("parmType");
  weightFunction = reader.readUint32("weightFunction");
  weight = reader.readFloat("weight");
  size_t parmValCount;
  if (auto v = reader.enterSubVector("range", parmValCount)) {
    switch (DataType(parmType)) {
    case DataType::Int32:
      range[0].int32 = reader.readInt32();
      range[1].int32 = reader.readInt32();
      break;
    case DataType::UInt32:
    case DataType::Enum:
      range[0].uint32 = reader.readUint32();
      range[1].uint32 = reader.readUint32();
      break;
    case DataType::Float:
      range[0].float32 = reader.readFloat();
      range[1].float32 = reader.readFloat();
      break;
    case DataType::Bool:
      range[0].bool1 = reader.readBool();
      range[1].bool1 = reader.readBool();
      break;
    default:
      break;
    }
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::Enumerate<BigDNA::WriteYaml>(
    athena::io::YAMLDocWriter& writer) {
  writer.writeUint32("parmType", parmType);
  writer.writeUint32("weightFunction", weightFunction);
  writer.writeFloat("weight", weight);
  if (auto v = writer.enterSubVector("range")) {
    switch (DataType(parmType)) {
    case DataType::Int32:
      writer.writeInt32(range[0].int32);
      writer.writeInt32(range[1].int32);
      break;
    case DataType::UInt32:
    case DataType::Enum:
      writer.writeUint32(range[0].uint32);
      writer.writeUint32(range[1].uint32);
      break;
    case DataType::Float:
      writer.writeFloat(range[0].float32);
      writer.writeFloat(range[1].float32);
      break;
    case DataType::Bool:
      writer.writeBool(range[0].bool1);
      writer.writeBool(range[1].bool1);
      break;
    }
  }
}

std::string_view ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo::DNAType() {
  return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::ParmInfo"sv;
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::Enumerate<BigDNA::Read>(
    athena::io::IStreamReader& reader) {
  id = reader.readUint32Big();
  atUint32 parmInfoCount = reader.readUint32Big();
  atUint32 animInfoCount = reader.readUint32Big();

  reader.enumerate(parmInfos, parmInfoCount);

  animInfos.clear();
  animInfos.reserve(animInfoCount);
  reader.enumerate<AnimInfo>(animInfos, animInfoCount,
                             [this, parmInfoCount](athena::io::IStreamReader& reader, AnimInfo& ai) {
                               ai.id = reader.readUint32Big();
                               ai.parmVals.reserve(parmInfoCount);
                               for (const ParmInfo& pi : parmInfos) {
                                 switch (ParmInfo::DataType(pi.parmType)) {
                                 case ParmInfo::DataType::Int32:
                                   ai.parmVals.emplace_back(reader.readInt32Big());
                                   break;
                                 case ParmInfo::DataType::UInt32:
                                 case ParmInfo::DataType::Enum:
                                   ai.parmVals.emplace_back(reader.readUint32Big());
                                   break;
                                 case ParmInfo::DataType::Float:
                                   ai.parmVals.emplace_back(reader.readFloatBig());
                                   break;
                                 case ParmInfo::DataType::Bool:
                                   ai.parmVals.emplace_back(reader.readBool());
                                   break;
                                 default:
                                   break;
                                 }
                               }
                             });
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::Enumerate<BigDNA::Write>(
    athena::io::IStreamWriter& writer) {
  writer.writeUint32Big(id);
  writer.writeUint32Big(parmInfos.size());
  writer.writeUint32Big(animInfos.size());

  for (const ParmInfo& pi : parmInfos)
    pi.write(writer);

  for (const AnimInfo& ai : animInfos) {
    writer.writeUint32Big(ai.id);
    auto it = ai.parmVals.begin();
    for (const ParmInfo& pi : parmInfos) {
      ParmInfo::Parm pVal;
      if (it != ai.parmVals.end())
        pVal = *it++;
      switch (ParmInfo::DataType(pi.parmType)) {
      case ParmInfo::DataType::Int32:
        writer.writeInt32Big(pVal.int32);
        break;
      case ParmInfo::DataType::UInt32:
      case ParmInfo::DataType::Enum:
        writer.writeUint32Big(pVal.uint32);
        break;
      case ParmInfo::DataType::Float:
        writer.writeFloatBig(pVal.float32);
        break;
      case ParmInfo::DataType::Bool:
        writer.writeBool(pVal.bool1);
        break;
      default:
        break;
      }
    }
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 12;
  for (const ParmInfo& pi : parmInfos)
    pi.binarySize(__isz);

  __isz += animInfos.size() * 4;
  for (const ParmInfo& pi : parmInfos) {
    switch (ParmInfo::DataType(pi.parmType)) {
    case ParmInfo::DataType::Int32:
    case ParmInfo::DataType::UInt32:
    case ParmInfo::DataType::Enum:
    case ParmInfo::DataType::Float:
      __isz += animInfos.size() * 4;
      break;
    case ParmInfo::DataType::Bool:
      __isz += animInfos.size();
      break;
    default:
      break;
    }
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::Enumerate<BigDNA::ReadYaml>(
    athena::io::YAMLDocReader& reader) {
  id = reader.readUint32("id");

  size_t parmInfoCount = reader.enumerate("parmInfos", parmInfos);

  reader.enumerate<AnimInfo>("animInfos", animInfos,
                             [this, parmInfoCount](athena::io::YAMLDocReader& reader, AnimInfo& ai) {
                               ai.id = reader.readUint32("id");
                               ai.parmVals.reserve(parmInfoCount);
                               size_t parmValCount;
                               if (auto v = reader.enterSubVector("parms", parmValCount)) {
                                 for (const ParmInfo& pi : parmInfos) {
                                   switch (ParmInfo::DataType(pi.parmType)) {
                                   case ParmInfo::DataType::Int32:
                                     ai.parmVals.emplace_back(reader.readInt32());
                                     break;
                                   case ParmInfo::DataType::UInt32:
                                   case ParmInfo::DataType::Enum:
                                     ai.parmVals.emplace_back(reader.readUint32());
                                     break;
                                   case ParmInfo::DataType::Float:
                                     ai.parmVals.emplace_back(reader.readFloat());
                                     break;
                                   case ParmInfo::DataType::Bool:
                                     ai.parmVals.emplace_back(reader.readBool());
                                     break;
                                   default:
                                     break;
                                   }
                                 }
                               }
                             });
}

template <>
void ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::Enumerate<BigDNA::WriteYaml>(
    athena::io::YAMLDocWriter& writer) {
  writer.writeUint32("id", id);

  writer.enumerate("parmInfos", parmInfos);

  writer.enumerate<AnimInfo>("animInfos", animInfos, [this](athena::io::YAMLDocWriter& writer, const AnimInfo& ai) {
    writer.writeUint32("id", ai.id);
    auto it = ai.parmVals.begin();
    if (auto v = writer.enterSubVector("parms")) {
      for (const ParmInfo& pi : parmInfos) {
        ParmInfo::Parm pVal;
        if (it != ai.parmVals.end())
          pVal = *it++;
        switch (ParmInfo::DataType(pi.parmType)) {
        case ParmInfo::DataType::Int32:
          writer.writeInt32(pVal.int32);
          break;
        case ParmInfo::DataType::UInt32:
        case ParmInfo::DataType::Enum:
          writer.writeUint32(pVal.uint32);
          break;
        case ParmInfo::DataType::Float:
          writer.writeFloat(pVal.float32);
          break;
        case ParmInfo::DataType::Bool:
          writer.writeBool(pVal.bool1);
          break;
        default:
          break;
        }
      }
    }
  });
}

std::string_view ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState::DNAType() {
  return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo::PASDatabase::AnimState"sv;
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  idx = reader.readUint32Big();
  atUint16 sectionCount = reader.readUint16Big();
  name = reader.readString();
  cmdl.read(reader);
  cskr.read(reader);
  cinf.read(reader);

  atUint32 animationCount = reader.readUint32Big();
  reader.enumerate(animations, animationCount);

  pasDatabase.read(reader);

  atUint32 partCount = reader.readUint32Big();
  reader.enumerate(partResData.part, partCount);

  atUint32 swhcCount = reader.readUint32Big();
  reader.enumerate(partResData.swhc, swhcCount);

  atUint32 unkCount = reader.readUint32Big();
  reader.enumerate(partResData.unk, unkCount);

  partResData.elsc.clear();
  if (sectionCount > 5) {
    atUint32 elscCount = reader.readUint32Big();
    reader.enumerate(partResData.elsc, elscCount);
  }

  unk1 = reader.readUint32Big();

  animAABBs.clear();
  if (sectionCount > 1) {
    atUint32 aabbCount = reader.readUint32Big();
    reader.enumerate(animAABBs, aabbCount);
  }

  effects.clear();
  if (sectionCount > 2) {
    atUint32 effectCount = reader.readUint32Big();
    reader.enumerate(effects, effectCount);
  }

  if (sectionCount > 3) {
    cmdlIce.read(reader);
    cskrIce.read(reader);
  }

  animIdxs.clear();
  if (sectionCount > 4) {
    atUint32 aidxCount = reader.readUint32Big();
    reader.enumerateBig(animIdxs, aidxCount);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  writer.writeUint32Big(idx);

  atUint16 sectionCount;
  if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce.isValid())
    sectionCount = 4;
  else if (effects.size())
    sectionCount = 3;
  else if (animAABBs.size())
    sectionCount = 2;
  else
    sectionCount = 1;
  writer.writeUint16Big(sectionCount);

  writer.writeString(name);
  cmdl.write(writer);
  cskr.write(writer);
  cinf.write(writer);

  writer.writeUint32Big(animations.size());
  writer.enumerate(animations);

  pasDatabase.write(writer);

  writer.writeUint32Big(partResData.part.size());
  writer.enumerate(partResData.part);

  writer.writeUint32Big(partResData.swhc.size());
  writer.enumerate(partResData.swhc);

  writer.writeUint32Big(partResData.unk.size());
  writer.enumerate(partResData.unk);

  if (sectionCount > 5) {
    writer.writeUint32Big(partResData.elsc.size());
    writer.enumerate(partResData.elsc);
  }

  writer.writeUint32Big(unk1);

  if (sectionCount > 1) {
    writer.writeUint32Big(animAABBs.size());
    writer.enumerate(animAABBs);
  }

  if (sectionCount > 2) {
    writer.writeUint32Big(effects.size());
    writer.enumerate(effects);
  }

  if (sectionCount > 3) {
    cmdlIce.write(writer);
    cskrIce.write(writer);
  }

  if (sectionCount > 4) {
    writer.writeUint32Big(animIdxs.size());
    for (atUint32 idx : animIdxs)
      writer.writeUint32Big(idx);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 6;

  atUint16 sectionCount;
  if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce.isValid())
    sectionCount = 4;
  else if (effects.size())
    sectionCount = 3;
  else if (animAABBs.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  __isz += name.size() + 1;
  __isz += 12;

  __isz += 4;
  for (const Animation& a : animations)
    a.binarySize(__isz);

  pasDatabase.binarySize(__isz);

  __isz += 4;
  for (const UniqueID32& id : partResData.part)
    id.binarySize(__isz);

  __isz += 4;
  for (const UniqueID32& id : partResData.swhc)
    id.binarySize(__isz);

  __isz += 4;
  for (const UniqueID32& id : partResData.unk)
    id.binarySize(__isz);

  if (sectionCount > 5) {
    __isz += 4;
    for (const UniqueID32& id : partResData.elsc)
      id.binarySize(__isz);
  }

  __isz += 4;

  if (sectionCount > 1) {
    __isz += 4;
    for (const ActionAABB& aabb : animAABBs)
      aabb.binarySize(__isz);
  }

  if (sectionCount > 2) {
    __isz += 4;
    for (const Effect& e : effects)
      e.binarySize(__isz);
  }

  if (sectionCount > 3)
    __isz += 8;

  if (sectionCount > 4)
    __isz += 4 + animIdxs.size() * 4;
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  idx = reader.readUint32("idx");
  atUint16 sectionCount = reader.readUint16("sectionCount");
  name = reader.readString("name");
  reader.enumerate("cmdl", cmdl);

  reader.enumerate("animations", animations);

  reader.enumerate("pasDatabase", pasDatabase);

  reader.enumerate("part", partResData.part);

  reader.enumerate("swhc", partResData.swhc);

  reader.enumerate("unk", partResData.unk);

  partResData.elsc.clear();
  if (sectionCount > 5) {
    reader.enumerate("elsc", partResData.elsc);
  }

  unk1 = reader.readUint32("unk1");

  animAABBs.clear();
  if (sectionCount > 1) {
    reader.enumerate("part", animAABBs);
  }

  effects.clear();
  if (sectionCount > 2) {
    reader.enumerate("effects", effects);
  }

  if (sectionCount > 3) {
    reader.enumerate("cmdlIce", cmdlIce);
  }

  animIdxs.clear();
  if (sectionCount > 4) {
    reader.enumerate("animIdxs", animIdxs);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  writer.writeUint32("idx", idx);

  atUint16 sectionCount;
  if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce.isValid())
    sectionCount = 4;
  else if (effects.size())
    sectionCount = 3;
  else if (animAABBs.size())
    sectionCount = 2;
  else
    sectionCount = 1;
  writer.writeUint16("sectionCount", sectionCount);

  writer.writeString("name", name);
  writer.enumerate("cmdl", cmdl);

  writer.enumerate("animations", animations);

  writer.enumerate("pasDatabase", pasDatabase);

  writer.enumerate("part", partResData.part);

  writer.enumerate("swhc", partResData.swhc);

  writer.enumerate("unk", partResData.unk);

  if (sectionCount > 5) {
    writer.enumerate("elsc", partResData.elsc);
  }

  writer.writeUint32("unk1", unk1);

  if (sectionCount > 1) {
    writer.enumerate("animAABBs", animAABBs);
  }

  if (sectionCount > 2) {
    writer.enumerate("effects", effects);
  }

  if (sectionCount > 3) {
    writer.enumerate("cmdlIce", cmdlIce);
  }

  if (sectionCount > 4) {
    writer.enumerate("animIdxs", animIdxs);
  }
}

std::string_view ANCS::CharacterSet::CharacterInfo::DNAType() {
  return "urde::DNAMP1::ANCS::CharacterSet::CharacterInfo"sv;
}

template <>
void ANCS::AnimationSet::MetaAnimFactory::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
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
void ANCS::AnimationSet::MetaAnimFactory::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  if (!m_anim)
    return;
  writer.writeInt32Big(atUint32(m_anim->m_type));
  m_anim->write(writer);
}

template <>
void ANCS::AnimationSet::MetaAnimFactory::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  if (!m_anim)
    return;
  __isz += 4;
  m_anim->binarySize(__isz);
}

template <>
void ANCS::AnimationSet::MetaAnimFactory::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  std::string type = reader.readString("type");
  std::transform(type.begin(), type.end(), type.begin(), tolower);
  if (type == "primitive") {
    m_anim.reset(new struct MetaAnimPrimitive);
    m_anim->read(reader);
  } else if (type == "blend") {
    m_anim.reset(new struct MetaAnimBlend);
    m_anim->read(reader);
  } else if (type == "phaseblend") {
    m_anim.reset(new struct MetaAnimPhaseBlend);
    m_anim->read(reader);
  } else if (type == "random") {
    m_anim.reset(new struct MetaAnimRandom);
    m_anim->read(reader);
  } else if (type == "sequence") {
    m_anim.reset(new struct MetaAnimSequence);
    m_anim->read(reader);
  } else {
    m_anim.reset(nullptr);
  }
}

template <>
void ANCS::AnimationSet::MetaAnimFactory::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  if (!m_anim)
    return;
  writer.writeString("type", m_anim->m_typeStr);
  m_anim->write(writer);
}

std::string_view ANCS::AnimationSet::MetaAnimFactory::DNAType() {
  return "urde::DNAMP1::ANCS::AnimationSet::MetaAnimFactory"sv;
}

template <>
void ANCS::AnimationSet::MetaTransFactory::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  IMetaTrans::Type type(IMetaTrans::Type(reader.readUint32Big()));
  switch (type) {
  case IMetaTrans::Type::MetaAnim:
    m_trans.reset(new struct MetaTransMetaAnim);
    m_trans->read(reader);
    break;
  case IMetaTrans::Type::Trans:
    m_trans.reset(new struct MetaTransTrans);
    m_trans->read(reader);
    break;
  case IMetaTrans::Type::PhaseTrans:
    m_trans.reset(new struct MetaTransPhaseTrans);
    m_trans->read(reader);
    break;
  case IMetaTrans::Type::NoTrans:
  default:
    m_trans.reset(nullptr);
    break;
  }
}

template <>
void ANCS::AnimationSet::MetaTransFactory::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  if (!m_trans) {
    writer.writeInt32Big(atUint32(IMetaTrans::Type::NoTrans));
    return;
  }
  writer.writeInt32Big(atUint32(m_trans->m_type));
  m_trans->write(writer);
}

template <>
void ANCS::AnimationSet::MetaTransFactory::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += 4;
  if (!m_trans)
    return;
  m_trans->binarySize(__isz);
}

template <>
void ANCS::AnimationSet::MetaTransFactory::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  std::string type = reader.readString("type");
  std::transform(type.begin(), type.end(), type.begin(), tolower);
  if (type == "metaanim") {
    m_trans.reset(new struct MetaTransMetaAnim);
    m_trans->read(reader);
  } else if (type == "trans") {
    m_trans.reset(new struct MetaTransTrans);
    m_trans->read(reader);
  } else if (type == "phasetrans") {
    m_trans.reset(new struct MetaTransPhaseTrans);
    m_trans->read(reader);
  } else {
    m_trans.reset(nullptr);
  }
}

template <>
void ANCS::AnimationSet::MetaTransFactory::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  if (!m_trans) {
    writer.writeString("type", "NoTrans");
    return;
  }
  writer.writeString("type", m_trans->m_typeStr ? m_trans->m_typeStr : "NoTrans");
  m_trans->write(writer);
}

std::string_view ANCS::AnimationSet::MetaTransFactory::DNAType() {
  return "urde::DNAMP1::ANCS::AnimationSet::MetaTransFactory"sv;
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::Read>(athena::io::IStreamReader& reader) {
  atUint16 sectionCount = reader.readUint16Big();

  atUint32 animationCount = reader.readUint32Big();
  reader.enumerate(animations, animationCount);

  atUint32 transitionCount = reader.readUint32Big();
  reader.enumerate(transitions, transitionCount);
  defaultTransition.read(reader);

  additiveAnims.clear();
  if (sectionCount > 1) {
    atUint32 additiveAnimCount = reader.readUint32Big();
    reader.enumerate(additiveAnims, additiveAnimCount);
    additiveDefaultFadeInDur = reader.readFloatBig();
    additiveDefaultFadeOutDur = reader.readFloatBig();
  }

  halfTransitions.clear();
  if (sectionCount > 2) {
    atUint32 halfTransitionCount = reader.readUint32Big();
    reader.enumerate(halfTransitions, halfTransitionCount);
  }

  animResources.clear();
  if (sectionCount > 3) {
    atUint32 animResourcesCount = reader.readUint32Big();
    reader.enumerate(animResources, animResourcesCount);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& writer) {
  atUint16 sectionCount;
  if (animResources.size())
    sectionCount = 4;
  else if (halfTransitions.size())
    sectionCount = 3;
  else if (additiveAnims.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  writer.writeUint16Big(sectionCount);

  writer.writeUint32Big(animations.size());
  writer.enumerate(animations);

  writer.writeUint32Big(transitions.size());
  writer.enumerate(transitions);
  defaultTransition.write(writer);

  if (sectionCount > 1) {
    writer.writeUint32Big(additiveAnims.size());
    writer.enumerate(additiveAnims);
    writer.writeFloatBig(additiveDefaultFadeInDur);
    writer.writeFloatBig(additiveDefaultFadeOutDur);
  }

  if (sectionCount > 2) {
    writer.writeUint32Big(halfTransitions.size());
    writer.enumerate(halfTransitions);
  }

  if (sectionCount > 3) {
    writer.writeUint32Big(animResources.size());
    writer.enumerate(animResources);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  atUint16 sectionCount;
  if (animResources.size())
    sectionCount = 4;
  else if (halfTransitions.size())
    sectionCount = 3;
  else if (additiveAnims.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  __isz += 6;
  for (const Animation& a : animations)
    a.binarySize(__isz);

  __isz += 4;
  for (const Transition& t : transitions)
    t.binarySize(__isz);
  defaultTransition.binarySize(__isz);

  if (sectionCount > 1) {
    __isz += 4;
    for (const AdditiveAnimationInfo& aa : additiveAnims)
      aa.binarySize(__isz);
    __isz += 8;
  }

  if (sectionCount > 2) {
    __isz += 4;
    for (const HalfTransition& ht : halfTransitions)
      ht.binarySize(__isz);
  }

  if (sectionCount > 3) {
    __isz += 4;
    for (const AnimationResources& ar : animResources)
      ar.binarySize(__isz);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& reader) {
  atUint16 sectionCount = reader.readUint16("sectionCount");

  reader.enumerate("animations", animations);

  reader.enumerate("transitions", transitions);
  reader.enumerate("defaultTransition", defaultTransition);

  additiveAnims.clear();
  if (sectionCount > 1) {
    reader.enumerate("additiveAnims", additiveAnims);
    additiveDefaultFadeInDur = reader.readFloat("additiveDefaultFadeInDur");
    additiveDefaultFadeOutDur = reader.readFloat("additiveDefaultFadeOutDur");
  }

  halfTransitions.clear();
  if (sectionCount > 2) {
    reader.enumerate("halfTransitions", halfTransitions);
  }

  animResources.clear();
  if (sectionCount > 3) {
    reader.enumerate("animResources", animResources);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& writer) {
  atUint16 sectionCount;
  if (animResources.size())
    sectionCount = 4;
  else if (halfTransitions.size())
    sectionCount = 3;
  else if (additiveAnims.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  writer.writeUint16("sectionCount", sectionCount);

  writer.enumerate("animations", animations);

  writer.enumerate("transitions", transitions);
  writer.enumerate("defaultTransition", defaultTransition);

  if (sectionCount > 1) {
    writer.enumerate("additiveAnims", additiveAnims);
    writer.writeFloat("additiveDefaultFadeInDur", additiveDefaultFadeInDur);
    writer.writeFloat("additiveDefaultFadeOutDur", additiveDefaultFadeOutDur);
  }

  if (sectionCount > 2) {
    writer.enumerate("halfTransitions", halfTransitions);
  }

  if (sectionCount > 3) {
    writer.enumerate("animResources", animResources);
  }
}

void ANCS::AnimationSet::MetaAnimPrimitive::gatherPrimitives(
    PAKRouter<PAKBridge>* pakRouter, std::map<atUint32, DNAANCS::AnimationResInfo<UniqueID32>>& out) {
  if (!pakRouter) {
    out[animIdx] = {animName, animId, UniqueID32(), false};
    return;
  }

  const nod::Node* node;
  const PAK::Entry* entry = pakRouter->lookupEntry(animId, &node, true);
  if (!entry) {
    out[animIdx] = {animName, animId, UniqueID32(), false};
    return;
  }

  PAKEntryReadStream rs = entry->beginReadStream(*node);
  out[animIdx] = {animName, animId, ANIM::GetEVNTId(rs), false};
}

std::string_view ANCS::AnimationSet::DNAType() { return "urde::DNAMP1::ANCS::AnimationSet"sv; }

bool ANCS::Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                   PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                   std::function<void(const hecl::SystemChar*)> fileChanged) {
  hecl::ProjectPath yamlPath = outPath.getWithExtension(_SYS_STR(".yaml"), true);
  hecl::ProjectPath::Type yamlType = yamlPath.getPathType();
  hecl::ProjectPath blendPath = outPath.getWithExtension(_SYS_STR(".blend"), true);
  hecl::ProjectPath::Type blendType = blendPath.getPathType();

  ANCS ancs;
  ancs.read(rs);

  if (force || yamlType == hecl::ProjectPath::Type::None || blendType == hecl::ProjectPath::Type::None) {
    if (force || yamlType == hecl::ProjectPath::Type::None) {
      athena::io::FileWriter writer(yamlPath.getAbsolutePath());
      athena::io::ToYAMLStream(ancs, writer);
    }

    if (force || blendType == hecl::ProjectPath::Type::None) {
      hecl::blender::Connection& conn = btok.getBlenderConnection();
      DNAANCS::ReadANCSToBlender<PAKRouter<PAKBridge>, ANCS, MaterialSet, DNACMDL::SurfaceHeader_1, 2>(
          conn, ancs, blendPath, pakRouter, entry, dataSpec, fileChanged, force);
    }
  }

  return true;
}

bool ANCS::Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNAANCS::Actor& actor) {
  /* Search for yaml */
  hecl::ProjectPath yamlPath = inPath.getWithExtension(_SYS_STR(".yaml"), true);
  if (!yamlPath.isFile())
    Log.report(logvisor::Fatal, fmt(_SYS_STR("'{}' not found as file")), yamlPath.getRelativePath());

  athena::io::FileReader reader(yamlPath.getAbsolutePath());
  if (!reader.isOpen())
    Log.report(logvisor::Fatal, fmt(_SYS_STR("can't open '{}' for reading")), yamlPath.getRelativePath());

  if (!athena::io::ValidateFromYAMLStream<ANCS>(reader)) {
    Log.report(logvisor::Fatal, fmt(_SYS_STR("'{}' is not urde::DNAMP1::ANCS type")), yamlPath.getRelativePath());
  }

  athena::io::YAMLDocReader yamlReader;
  if (!yamlReader.parse(&reader)) {
    Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to parse '{}'")), yamlPath.getRelativePath());
  }
  ANCS ancs;
  ancs.read(yamlReader);

  /* Set Character Resource IDs */
  for (ANCS::CharacterSet::CharacterInfo& ch : ancs.characterSet.characters) {
    ch.cmdl = UniqueID32{};
    ch.cskr = UniqueID32{};
    ch.cinf = UniqueID32{};
    ch.cmdlIce = UniqueID32Zero{};
    ch.cskrIce = UniqueID32Zero{};

    int subtypeIdx = 0;
    ch.animAABBs.clear();
    for (const DNAANCS::Actor::Subtype& sub : actor.subtypes) {
      if (sub.name == ch.name) {
        hecl::SystemStringConv chSysName(ch.name);
        ch.cskr = inPath.ensureAuxInfo(fmt::format(fmt(_SYS_STR("{}_{}.CSKR")), chSysName, sub.cskrId));

        /* Add subtype AABBs */
        ch.animAABBs.reserve(actor.actions.size());
        for (const DNAANCS::Action& act : actor.actions) {
          const auto& sourceAABB = act.subtypeAABBs[subtypeIdx];
          ch.animAABBs.emplace_back();
          auto& destAABB = ch.animAABBs.back();
          destAABB.name = act.name;
          destAABB.aabb[0] = sourceAABB.first.val;
          destAABB.aabb[1] = sourceAABB.second.val;
        }

        if (sub.armature >= 0) {
          const DNAANCS::Armature& arm = actor.armatures[sub.armature];
          ch.cinf = arm.path;
          ch.cmdl = sub.mesh;
          auto search = std::find_if(sub.overlayMeshes.cbegin(), sub.overlayMeshes.cend(),
                                     [](const auto& p) { return p.name == "ICE"; });
          if (search != sub.overlayMeshes.cend()) {
            hecl::SystemStringConv overlaySys(search->name);
            ch.cmdlIce = search->mesh;
            ch.cskrIce = inPath.ensureAuxInfo(
                fmt::format(fmt(_SYS_STR("{}.{}_{}.CSKR")), chSysName, overlaySys, search->cskrId));
          }
        }

        break;
      }
      ++subtypeIdx;
    }

    std::sort(ch.animAABBs.begin(), ch.animAABBs.end(),
              [](const ANCS::CharacterSet::CharacterInfo::ActionAABB& a,
                 const ANCS::CharacterSet::CharacterInfo::ActionAABB& b) { return a.name < b.name; });
  }

  /* Set Animation Resource IDs */
  ancs.enumeratePrimitives([&](AnimationSet::MetaAnimPrimitive& prim) {
    hecl::SystemStringConv sysStr(prim.animName);
    for (const DNAANCS::Action& act : actor.actions) {
      if (act.name == prim.animName) {
        hecl::ProjectPath pathOut = inPath.ensureAuxInfo(fmt::format(fmt(_SYS_STR("{}_{}.ANIM")), sysStr, act.animId));
        prim.animId = pathOut;
        break;
      }
    }
    return true;
  });

  /* Gather ANIM resources */
  hecl::DirectoryEnumerator dEnum(inPath.getParentPath().getAbsolutePath());
  ancs.animationSet.animResources.reserve(actor.actions.size());
  for (const DNAANCS::Action& act : actor.actions) {
    hecl::SystemStringConv sysStr(act.name);
    hecl::ProjectPath pathOut = inPath.ensureAuxInfo(fmt::format(fmt(_SYS_STR("{}_{}.ANIM")), sysStr, act.animId));

    ancs.animationSet.animResources.emplace_back();
    ancs.animationSet.animResources.back().animId = pathOut;

    /* Check for associated EVNT YAML */
    hecl::SystemString testPrefix(inPath.getWithExtension(
        fmt::format(fmt(_SYS_STR(".{}_")), sysStr).c_str(), true).getLastComponent());
    hecl::ProjectPath evntYamlPath;
    for (const auto& ent : dEnum) {
      if (hecl::StringUtils::BeginsWith(ent.m_name, testPrefix.c_str()) &&
          hecl::StringUtils::EndsWith(ent.m_name, _SYS_STR(".evnt.yaml"))) {
        evntYamlPath = hecl::ProjectPath(inPath.getParentPath(), ent.m_name);
        break;
      }
    }
    if (evntYamlPath.isFile()) {
      evntYamlPath = evntYamlPath.ensureAuxInfo(_SYS_STR(""));
      ancs.animationSet.animResources.back().evntId = evntYamlPath;
    }
  }

  /* Write out ANCS */
  athena::io::TransactionalFileWriter w(outPath.getAbsolutePath());
  ancs.write(w);

  return true;
}

static const hecl::SystemRegex regCskrNameId(_SYS_STR(R"((.*)_[0-9a-fA-F]{8}\.CSKR)"),
                                             std::regex::ECMAScript | std::regex::optimize);
static const hecl::SystemRegex regCskrName(_SYS_STR(R"((.*)\.CSKR)"),
                                           std::regex::ECMAScript | std::regex::optimize);

bool ANCS::CookCSKR(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNAANCS::Actor& actor,
                    const std::function<bool(const hecl::ProjectPath& modelPath)>& modelCookFunc) {
  auto auxInfo = inPath.getAuxInfo();
  hecl::SystemViewRegexMatch match;
  if (!std::regex_search(auxInfo.begin(), auxInfo.end(), match, regCskrNameId) &&
      !std::regex_search(auxInfo.begin(), auxInfo.end(), match, regCskrName))
    return false;

  hecl::SystemString subName = match[1].str();
  hecl::SystemString overName;
  auto dotPos = subName.rfind(_SYS_STR('.'));
  if (dotPos != hecl::SystemString::npos) {
    overName = hecl::SystemString(subName.begin() + dotPos + 1, subName.end());
    subName = hecl::SystemString(subName.begin(), subName.begin() + dotPos);
  }
  hecl::SystemUTF8Conv subNameView(subName);
  hecl::SystemUTF8Conv overNameView(overName);

  /* Build bone ID map */
  std::unordered_map<std::string, atInt32> boneIdMap;
  for (const DNAANCS::Armature& arm : actor.armatures) {
    CINF cinf(*arm.armature, boneIdMap);
  }

  const DNAANCS::Actor::Subtype* subtype = nullptr;
  if (subName != _SYS_STR("ATTACH")) {
    for (const DNAANCS::Actor::Subtype& sub : actor.subtypes) {
      if (sub.name == subNameView.str()) {
        subtype = &sub;
        break;
      }
    }
    if (!subtype)
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to find subtype '{}'")), subName);
  }

  const hecl::ProjectPath* modelPath = nullptr;
  if (subName == _SYS_STR("ATTACH")) {
    const DNAANCS::Actor::Attachment* attachment = nullptr;
    for (const DNAANCS::Actor::Attachment& att : actor.attachments) {
      if (att.name == overNameView.str()) {
        attachment = &att;
        break;
      }
    }
    if (!attachment)
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to find attachment '{}'")), overName);
    modelPath = &attachment->mesh;
  } else if (overName.empty()) {
    modelPath = &subtype->mesh;
  } else {
    for (const auto& overlay : subtype->overlayMeshes)
      if (overlay.name == overNameView.str()) {
        modelPath = &overlay.mesh;
        break;
      }
  }
  if (!modelPath)
    Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to resolve model path of {}:{}")), subName, overName);

  if (!modelPath->isFile())
    Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to resolve '{}'")), modelPath->getRelativePath());

  hecl::ProjectPath skinIntPath = modelPath->getCookedPath(SpecEntMP1).getWithExtension(_SYS_STR(".skinint"));
  if (!skinIntPath.isFileOrGlob() || skinIntPath.getModtime() < modelPath->getModtime())
    if (!modelCookFunc(*modelPath))
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to cook '{}'")), modelPath->getRelativePath());

  std::vector<std::pair<std::vector<std::pair<uint32_t, float>>, uint32_t>> skins;
  uint32_t posCount = 0;
  uint32_t normCount = 0;
  athena::io::FileReader skinIO(skinIntPath.getAbsolutePath(), 1024 * 32, false);
  if (!skinIO.hasError()) {
    std::vector<std::string> boneNames;
    uint32_t boneNameCount = skinIO.readUint32Big();
    boneNames.reserve(boneNameCount);
    for (uint32_t i = 0; i < boneNameCount; ++i)
      boneNames.push_back(skinIO.readString());

    uint32_t skinCount = skinIO.readUint32Big();
    skins.resize(skinCount);
    for (uint32_t i = 0; i < skinCount; ++i) {
      std::pair<std::vector<std::pair<uint32_t, float>>, uint32_t>& virtualBone = skins[i];
      uint32_t bindCount = skinIO.readUint32Big();
      virtualBone.first.reserve(bindCount);
      for (uint32_t j = 0; j < bindCount; ++j) {
        uint32_t bIdx = skinIO.readUint32Big();
        float weight = skinIO.readFloatBig();
        const std::string& name = boneNames[bIdx];
        auto search = boneIdMap.find(name);
        if (search == boneIdMap.cend())
          Log.report(logvisor::Fatal, fmt("unable to find bone '{}' in {}"), name,
                     inPath.getRelativePathUTF8());
        virtualBone.first.emplace_back(search->second, weight);
      }
      virtualBone.second = skinIO.readUint32Big();
    }

    posCount = skinIO.readUint32Big();
    normCount = skinIO.readUint32Big();

    skinIO.close();
  }

  athena::io::TransactionalFileWriter skinOut(outPath.getAbsolutePath());

  skinOut.writeUint32Big(skins.size());
  for (auto& virtuaBone : skins) {
    skinOut.writeUint32Big(virtuaBone.first.size());
    for (auto& bind : virtuaBone.first) {
      skinOut.writeUint32Big(bind.first);
      skinOut.writeFloatBig(bind.second);
    }
    skinOut.writeUint32Big(virtuaBone.second);
  }

  skinOut.writeUint32Big(0xffffffff);
  skinOut.writeUint32Big(posCount);
  skinOut.writeUint32Big(0xffffffff);
  skinOut.writeUint32Big(normCount);

  return true;
}

bool ANCS::CookCSKRPC(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNAANCS::Actor& actor,
                      const std::function<bool(const hecl::ProjectPath& modelPath)>& modelCookFunc) {
  auto auxInfo = inPath.getAuxInfo();
  hecl::SystemViewRegexMatch match;
  if (!std::regex_search(auxInfo.begin(), auxInfo.end(), match, regCskrNameId) &&
      !std::regex_search(auxInfo.begin(), auxInfo.end(), match, regCskrName))
    return false;

  hecl::SystemString subName = match[1].str();
  hecl::SystemString overName;
  auto dotPos = subName.rfind(_SYS_STR('.'));
  if (dotPos != hecl::SystemString::npos) {
    overName = hecl::SystemString(subName.begin() + dotPos + 1, subName.end());
    subName = hecl::SystemString(subName.begin(), subName.begin() + dotPos);
  }
  hecl::SystemUTF8Conv subNameView(subName);
  hecl::SystemUTF8Conv overNameView(overName);

  /* Build bone ID map */
  std::unordered_map<std::string, atInt32> boneIdMap;
  for (const DNAANCS::Armature& arm : actor.armatures) {
    CINF cinf(*arm.armature, boneIdMap);
  }

  const DNAANCS::Actor::Subtype* subtype = nullptr;
  if (subName != _SYS_STR("ATTACH")) {
    for (const DNAANCS::Actor::Subtype& sub : actor.subtypes) {
      if (sub.name == subNameView.str()) {
        subtype = &sub;
        break;
      }
    }
    if (!subtype)
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to find subtype '{}'")), subName);
  }

  const hecl::ProjectPath* modelPath = nullptr;
  if (subName == _SYS_STR("ATTACH")) {
    const DNAANCS::Actor::Attachment* attachment = nullptr;
    for (const DNAANCS::Actor::Attachment& att : actor.attachments) {
      if (att.name == overNameView.str()) {
        attachment = &att;
        break;
      }
    }
    if (!attachment)
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to find attachment '{}'")), overName);
    modelPath = &attachment->mesh;
  } else if (overName.empty()) {
    modelPath = &subtype->mesh;
  } else {
    for (const auto& overlay : subtype->overlayMeshes)
      if (overlay.name == overNameView.str()) {
        modelPath = &overlay.mesh;
        break;
      }
  }
  if (!modelPath)
    Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to resolve model path of {}:{}")), subName, overName);

  if (!modelPath->isFile())
    Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to resolve '{}'")), modelPath->getRelativePath());

  hecl::ProjectPath skinIntPath = modelPath->getCookedPath(SpecEntMP1PC).getWithExtension(_SYS_STR(".skinint"));
  if (!skinIntPath.isFileOrGlob() || skinIntPath.getModtime() < modelPath->getModtime())
    if (!modelCookFunc(*modelPath))
      Log.report(logvisor::Fatal, fmt(_SYS_STR("unable to cook '{}'")), modelPath->getRelativePath());

  uint32_t bankCount = 0;
  std::vector<std::vector<uint32_t>> skinBanks;
  std::vector<std::string> boneNames;
  std::vector<std::vector<std::pair<uint32_t, float>>> skins;
  atUint64 uniquePoolIndexLen = 0;
  std::unique_ptr<atUint8[]> uniquePoolIndexData;
  athena::io::FileReader skinIO(skinIntPath.getAbsolutePath(), 1024 * 32, false);
  if (!skinIO.hasError()) {
    bankCount = skinIO.readUint32Big();
    skinBanks.reserve(bankCount);
    for (uint32_t i = 0; i < bankCount; ++i) {
      skinBanks.emplace_back();
      std::vector<uint32_t>& bonesOut = skinBanks.back();
      uint32_t boneCount = skinIO.readUint32Big();
      bonesOut.reserve(boneCount);
      for (uint32_t j = 0; j < boneCount; ++j) {
        uint32_t idx = skinIO.readUint32Big();
        bonesOut.push_back(idx);
      }
    }

    uint32_t boneNameCount = skinIO.readUint32Big();
    boneNames.reserve(boneNameCount);
    for (uint32_t i = 0; i < boneNameCount; ++i)
      boneNames.push_back(skinIO.readString());

    uint32_t skinCount = skinIO.readUint32Big();
    skins.resize(skinCount);
    for (uint32_t i = 0; i < skinCount; ++i) {
      std::vector<std::pair<uint32_t, float>>& virtualBone = skins[i];
      uint32_t bindCount = skinIO.readUint32Big();
      virtualBone.reserve(bindCount);
      for (uint32_t j = 0; j < bindCount; ++j) {
        uint32_t bIdx = skinIO.readUint32Big();
        float weight = skinIO.readFloatBig();
        const std::string& name = boneNames[bIdx];
        auto search = boneIdMap.find(name);
        if (search == boneIdMap.cend())
          Log.report(logvisor::Fatal, fmt("unable to find bone '{}' in {}"), name,
                     inPath.getRelativePathUTF8());
        virtualBone.emplace_back(search->second, weight);
      }
    }

    uniquePoolIndexLen = skinIO.length() - skinIO.position();
    uniquePoolIndexData = skinIO.readUBytes(uniquePoolIndexLen);

    skinIO.close();
  }

  athena::io::TransactionalFileWriter skinOut(outPath.getAbsolutePath());

  skinOut.writeUint32Big(bankCount);
  for (const std::vector<uint32_t>& bank : skinBanks) {
    skinOut.writeUint32Big(bank.size());
    for (uint32_t bIdx : bank) {
      const std::string& name = boneNames[bIdx];
      auto search = boneIdMap.find(name);
      if (search == boneIdMap.cend())
        Log.report(logvisor::Fatal, fmt("unable to find bone '{}' in {}"), name,
                   inPath.getRelativePathUTF8());
      skinOut.writeUint32Big(search->second);
    }
  }

  skinOut.writeUint32Big(skins.size());
  for (auto& virtuaBone : skins) {
    skinOut.writeUint32Big(virtuaBone.size());
    for (auto& bind : virtuaBone) {
      skinOut.writeUint32Big(bind.first);
      skinOut.writeFloatBig(bind.second);
    }
  }

  if (uniquePoolIndexLen)
    skinOut.writeUBytes(uniquePoolIndexData.get(), uniquePoolIndexLen);

  return true;
}

static const hecl::SystemRegex regAnimNameId(_SYS_STR(R"((.*)_[0-9a-fA-F]{8}\.ANIM)"),
                                             std::regex::ECMAScript | std::regex::optimize);
static const hecl::SystemRegex regAnimName(_SYS_STR(R"((.*)\.ANIM)"),
                                           std::regex::ECMAScript | std::regex::optimize);

bool ANCS::CookANIM(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath, const DNAANCS::Actor& actor,
                    hecl::blender::DataStream& ds, bool pc) {
  auto auxInfo = inPath.getAuxInfo();
  hecl::SystemViewRegexMatch match;
  if (!std::regex_search(auxInfo.begin(), auxInfo.end(), match, regAnimNameId) &&
      !std::regex_search(auxInfo.begin(), auxInfo.end(), match, regAnimName))
    return false;

  hecl::SystemString actName = match[1].str();
  hecl::SystemUTF8Conv actNameView(actName);
  DNAANCS::Action action = ds.compileActionChannelsOnly(actNameView.str());

  if (!actor.armatures.size())
    Log.report(logvisor::Fatal, fmt(_SYS_STR("0 armatures in {}")), inPath.getRelativePath());

  /* Build bone ID map */
  std::unordered_map<std::string, atInt32> boneIdMap;
  std::optional<CINF> rigCinf;
  std::optional<DNAANIM::RigInverter<CINF>> rigInv;
  for (const DNAANCS::Armature& arm : actor.armatures) {
    if (!rigInv) {
      rigCinf.emplace(*arm.armature, boneIdMap);
      auto matrices = ds.getBoneMatrices(arm.name);
      rigInv.emplace(*rigCinf, matrices);
    } else {
      CINF cinf(*arm.armature, boneIdMap);
    }
  }

  ANIM anim(action, boneIdMap, *rigInv, pc);

  /* Check for associated EVNT YAML */
  hecl::SystemString testPrefix(inPath.getWithExtension(
      fmt::format(fmt(_SYS_STR(".{}_")), actName).c_str(), true).getLastComponent());
  hecl::ProjectPath evntYamlPath;
  for (const auto& ent : hecl::DirectoryEnumerator(inPath.getParentPath().getAbsolutePath())) {
    if (hecl::StringUtils::BeginsWith(ent.m_name, testPrefix.c_str()) &&
        hecl::StringUtils::EndsWith(ent.m_name, _SYS_STR(".evnt.yaml"))) {
      evntYamlPath = hecl::ProjectPath(inPath.getParentPath(), ent.m_name);
      break;
    }
  }
  if (evntYamlPath.isFile()) {
    evntYamlPath = evntYamlPath.ensureAuxInfo(_SYS_STR(""));
    anim.m_anim->evnt = evntYamlPath;
  }

  /* Write out ANIM resource */
  athena::io::TransactionalFileWriter w(outPath.getAbsolutePath());
  anim.write(w);
  return true;
}

} // namespace DNAMP1
} // namespace DataSpec
