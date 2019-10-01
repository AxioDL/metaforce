#include "AFSM.hpp"

namespace DataSpec::DNAMP1 {

template <>
void AFSM::State::Transition::Enumerate<BigDNA::Read>(typename Read::StreamT& r) {
  triggerCount = r.readUint32Big();
  int i = 0;
  r.enumerate<Trigger>(triggers, triggerCount, [&](athena::io::IStreamReader& in, Trigger& tr) {
    tr.first = i == 0;
    tr.read(in);
    i++;
  });
}

template <>
void AFSM::State::Transition::Enumerate<BigDNA::Write>(typename Write::StreamT& w) {
  w.writeInt32Big(triggerCount);
  w.enumerate(triggers);
}

template <>
void AFSM::State::Transition::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  int i = 0;
  /* triggers */
  triggerCount = r.enumerate<Trigger>("triggers", triggers, [&](athena::io::YAMLDocReader& in, Trigger& tr) {
    tr.first = i == 0;
    tr.read(in);
    i++;
  });
}

template <>
void AFSM::State::Transition::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  /* triggers */
  w.enumerate("triggers", triggers);
}

template <>
void AFSM::State::Transition::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
  for (const Trigger& trig : triggers)
    trig.binarySize(s);
}

std::string_view AFSM::State::Transition::DNAType() { return "urde::DNAMP1::AFSM::Transition"sv; }

template <>
void AFSM::State::Transition::Trigger::Enumerate<BigDNA::Read>(athena::io::IStreamReader& __dna_reader) {
  /* name */
  name = __dna_reader.readString(-1);
  /* parameter */
  parameter = __dna_reader.readFloatBig();
  if (first) {
    /* targetState */
    targetState = __dna_reader.readUint32Big();
  }
}

template <>
void AFSM::State::Transition::Trigger::Enumerate<BigDNA::Write>(athena::io::IStreamWriter& __dna_writer) {
  /* name */
  __dna_writer.writeString(name, -1);
  /* parameter */
  __dna_writer.writeFloatBig(parameter);
  if (first) {
    /* targetState */
    __dna_writer.writeUint32Big(targetState);
  }
}

template <>
void AFSM::State::Transition::Trigger::Enumerate<BigDNA::ReadYaml>(athena::io::YAMLDocReader& __dna_docin) {
  /* name */
  name = __dna_docin.readString("name");
  /* parameter */
  parameter = __dna_docin.readFloat("parameter");
  if (first) {
    /* targetState */
    targetState = __dna_docin.readUint32("targetState");
  }
}

template <>
void AFSM::State::Transition::Trigger::Enumerate<BigDNA::WriteYaml>(athena::io::YAMLDocWriter& __dna_docout) {
  /* name */
  __dna_docout.writeString("name", name);
  /* parameter */
  __dna_docout.writeFloat("parameter", parameter);
  if (first) {
    /* targetState */
    __dna_docout.writeUint32("targetState", targetState);
  }
}

template <>
void AFSM::State::Transition::Trigger::Enumerate<BigDNA::BinarySize>(size_t& __isz) {
  __isz += name.size() + 1;
  __isz += (first ? 8 : 4);
}

std::string_view AFSM::State::Transition::Trigger::DNAType() {
  return "urde::DNAMP1::AFSM::State::Transition::Trigger"sv;
}

} // namespace DataSpec::DNAMP1
