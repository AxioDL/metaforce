#include "ANCS.hpp"

namespace DataSpec::DNAMP2 {

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
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

  atUint32 elscCount = reader.readUint32Big();
  reader.enumerate(partResData.elsc, elscCount);

  atUint32 spscCount = reader.readUint32Big();
  reader.enumerate(partResData.spsc, spscCount);

  atUint32 unkCount2 = reader.readUint32Big();
  if (unkCount2)
    abort();
  reader.enumerate(partResData.unk2, unkCount2);

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

  extents.clear();
  if (sectionCount > 9) {
    unk4 = reader.readUint32Big();
    unk5 = reader.readUByte();
    atUint32 extentsCount = reader.readUint32Big();
    reader.enumerate(extents, extentsCount);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  writer.writeUint32Big(idx);

  atUint16 sectionCount;
  if (unk4 || unk5 || extents.size())
    sectionCount = 10;
  else if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce)
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

  writer.writeUint32Big(partResData.elsc.size());
  writer.enumerate(partResData.elsc);

  writer.writeUint32Big(partResData.spsc.size());
  writer.enumerate(partResData.spsc);

  writer.writeUint32Big(partResData.unk2.size());
  writer.enumerate(partResData.unk2);

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

  if (sectionCount > 9) {
    writer.writeUint32Big(unk4);
    writer.writeUByte(unk5);
    writer.writeUint32Big(extents.size());
    writer.enumerate(extents);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  atUint16 sectionCount;
  if (unk4 || unk5 || extents.size())
    sectionCount = 10;
  else if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce)
    sectionCount = 4;
  else if (effects.size())
    sectionCount = 3;
  else if (animAABBs.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  s += 6;

  s += name.size() + 1;
  s += 12;

  s += 4;
  for (const Animation& anim : animations)
    anim.binarySize(s);

  pasDatabase.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.part)
    id.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.swhc)
    id.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.unk)
    id.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.elsc)
    id.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.spsc)
    id.binarySize(s);

  s += 4;
  for (const UniqueID32& id : partResData.unk2)
    id.binarySize(s);

  s += 4;

  if (sectionCount > 1) {
    s += 4;
    for (const MP1CharacterInfo::ActionAABB& aabb : animAABBs)
      aabb.binarySize(s);
  }

  if (sectionCount > 2) {
    s += 4;
    for (const Effect& e : effects)
      e.binarySize(s);
  }

  if (sectionCount > 3)
    s += 8;

  if (sectionCount > 4)
    s += 4 + animIdxs.size() * 4;

  if (sectionCount > 9) {
    s += 9;
    for (const Extents& e : extents)
      e.binarySize(s);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  idx = reader.readUint32("idx");
  atUint16 sectionCount = reader.readUint16("sectionCount");
  name = reader.readString("name");

  reader.enumerate("animations", animations);

  reader.enumerate("pasDatabase", pasDatabase);

  reader.enumerate("part", partResData.part);

  reader.enumerate("swhc", partResData.swhc);

  reader.enumerate("unk", partResData.unk);

  reader.enumerate("elsc", partResData.elsc);

  reader.enumerate("spsc", partResData.spsc);

  reader.enumerate("unk2", partResData.unk2);

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

  extents.clear();
  if (sectionCount > 9) {
    unk4 = reader.readUint32("unk4");
    unk5 = reader.readUByte("unk5");
    reader.enumerate("extents", extents);
  }
}

template <>
void ANCS::CharacterSet::CharacterInfo::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  writer.writeUint32("idx", idx);

  atUint16 sectionCount;
  if (unk4 || unk5 || extents.size())
    sectionCount = 10;
  else if (partResData.elsc.size())
    sectionCount = 6;
  else if (animIdxs.size())
    sectionCount = 5;
  else if (cmdlIce)
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

  writer.enumerate("elsc", partResData.elsc);

  writer.enumerate("spsc", partResData.spsc);

  writer.enumerate("unk2", partResData.unk2);

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

  if (sectionCount > 9) {
    writer.writeUint32("unk4", unk4);
    writer.writeUByte("unk5", unk5);
    writer.enumerate("extents", extents);
  }
}

const char* ANCS::CharacterSet::CharacterInfo::DNAType() { return "urde::DNAMP2::ANCS::CharacterSet::CharacterInfo"; }

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::Read>(typename Read::StreamT& reader) {
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
    floatA = reader.readFloatBig();
    floatB = reader.readFloatBig();
  }

  halfTransitions.clear();
  if (sectionCount > 2) {
    atUint32 halfTransitionCount = reader.readUint32Big();
    reader.enumerate(halfTransitions, halfTransitionCount);
  }

  evnts.clear();
  if (sectionCount > 3) {
    atUint32 evntsCount = reader.readUint32Big();
    reader.enumerate(evnts, evntsCount);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::Write>(typename Write::StreamT& writer) {
  atUint16 sectionCount;
  if (evnts.size())
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
    writer.writeFloatBig(floatA);
    writer.writeFloatBig(floatB);
  }

  if (sectionCount > 2) {
    writer.writeUint32Big(halfTransitions.size());
    writer.enumerate(halfTransitions);
  }

  if (sectionCount > 3) {
    writer.writeUint32Big(evnts.size());
    writer.enumerate(evnts);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  atUint16 sectionCount;
  if (evnts.size())
    sectionCount = 4;
  else if (halfTransitions.size())
    sectionCount = 3;
  else if (additiveAnims.size())
    sectionCount = 2;
  else
    sectionCount = 1;

  s += 6;
  for (const MP1AnimationSet::Animation& anim : animations)
    anim.binarySize(s);

  s += 4;
  for (const MP1AnimationSet::Transition& trans : transitions)
    trans.binarySize(s);
  defaultTransition.binarySize(s);

  if (sectionCount > 1) {
    s += 4;
    for (const MP1AnimationSet::AdditiveAnimationInfo& aaInfo : additiveAnims)
      aaInfo.binarySize(s);
    s += 8;
  }

  if (sectionCount > 2) {
    s += 4;
    for (const MP1AnimationSet::HalfTransition& ht : halfTransitions)
      ht.binarySize(s);
  }

  if (sectionCount > 3) {
    s += 4;
    for (const EVNT& evnt : evnts)
      evnt.binarySize(s);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& reader) {
  atUint16 sectionCount = reader.readUint16("sectionCount");

  reader.enumerate("animations", animations);

  reader.enumerate("transitions", transitions);
  reader.enumerate("defaultTransition", defaultTransition);

  additiveAnims.clear();
  if (sectionCount > 1) {
    reader.enumerate("additiveAnims", additiveAnims);
    floatA = reader.readFloat("floatA");
    floatB = reader.readFloat("floatB");
  }

  halfTransitions.clear();
  if (sectionCount > 2) {
    reader.enumerate("halfTransitions", halfTransitions);
  }

  evnts.clear();
  if (sectionCount > 3) {
    reader.enumerate("evnts", evnts);
  }
}

template <>
void ANCS::AnimationSet::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& writer) {
  atUint16 sectionCount;
  if (evnts.size())
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
    writer.writeFloat("floatA", floatA);
    writer.writeFloat("floatB", floatB);
  }

  if (sectionCount > 2) {
    writer.enumerate("halfTransitions", halfTransitions);
  }

  if (sectionCount > 3) {
    writer.enumerate("evnts", evnts);
  }
}

const char* ANCS::AnimationSet::DNAType() { return "urde::DNAMP2::ANCS::AnimationSet"; }

template <class Op>
void ANCS::AnimationSet::EVNT::Enumerate(typename Op::StreamT& s) {
  Do<Op>({"version"}, version, s);
  DoSize<Op>({"loopEventCount"}, loopEventCount, s);
  Do<Op>({"loopEvents"}, loopEvents, loopEventCount, s);
  if (version == 2) {
    DoSize<Op>({"uevtEventCount"}, uevtEventCount, s);
    Do<Op>({"uevtEvents"}, uevtEvents, uevtEventCount, s);
  }
  DoSize<Op>({"effectEventCount"}, effectEventCount, s);
  Do<Op>({"effectEvents"}, effectEvents, effectEventCount, s);
  DoSize<Op>({"sfxEventCount"}, sfxEventCount, s);
  Do<Op>({"sfxEvents"}, sfxEvents, sfxEventCount, s);
}

AT_SPECIALIZE_DNA(ANCS::AnimationSet::EVNT)

const char* ANCS::AnimationSet::EVNT::DNAType() { return "urde::DNAMP2::ANCS::AnimationSet::EVNT"; }

} // namespace DataSpec::DNAMP2
