#include "ParticleCommon.hpp"

namespace DataSpec::DNAParticle {
logvisor::Module LogModule("urde::DNAParticle");

template struct PEImpl<_RealElementFactory>;
template struct PEImpl<_IntElementFactory>;
template struct PEImpl<_VectorElementFactory>;
template struct PEImpl<_ColorElementFactory>;
template struct PEImpl<_ModVectorElementFactory>;
template struct PEImpl<_EmitterElementFactory>;
template struct PEImpl<_UVElementFactory<UniqueID32>>;
template struct PEImpl<_UVElementFactory<UniqueID64>>;

AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_RealElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_IntElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_VectorElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_ColorElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_ModVectorElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_EmitterElementFactory>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_UVElementFactory<UniqueID32>>)
AT_SUBSPECIALIZE_DNA_YAML(PEImpl<_UVElementFactory<UniqueID64>>)

template <>
void REConstant::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  val = r.readFloat();
}
template <>
void REConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeFloat(val);
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
  val = r.readUint32();
}
template <>
void IEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeUint32(val);
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
  if (auto v = r.enterSubVector(elemCount)) {
    for (size_t i = 0; i < 3 && i < elemCount; ++i) {
      if (auto rec = r.enterSubRecord())
        comps[i].read(r);
    }
  }
}
template <>
void VEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector())
    for (int i = 0; i < 3; ++i)
      if (auto rec = w.enterSubRecord())
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
    if (auto rec = r.enterSubRecord())
      comps[i].read(r);
}
template <>
void CEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector())
    for (int i = 0; i < 4; ++i)
      if (auto rec = w.enterSubRecord())
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
    if (auto rec = r.enterSubRecord())
      comps[i].read(r);
}
template <>
void MVEConstant::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  if (auto v = w.enterSubVector())
    for (int i = 0; i < 3; ++i)
      if (auto rec = w.enterSubRecord())
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
void BoolHelper::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  value = r.readBool();
}
template <>
void BoolHelper::Enumerate<BigDNA::WriteYaml>(typename WriteYaml::StreamT& w) {
  w.writeBool(value);
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
  w.writeBytes("CNST", 4);
  w.writeBool(value);
}

template struct ValueHelper<uint32_t>;
template struct ValueHelper<float>;

AT_SUBSPECIALIZE_DNA_YAML(ValueHelper<uint32_t>)
AT_SUBSPECIALIZE_DNA_YAML(ValueHelper<float>)

template <>
void EESimpleEmitterTR::Enumerate<BigDNA::ReadYaml>(typename ReadYaml::StreamT& r) {
  position.reset();
  velocity.reset();
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
  position.reset();
  velocity.reset();
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
  w.writeBytes("ILOC", 4);
  position.write(w);
  w.writeBytes("IVEC", 4);
  velocity.write(w);
}

template <>
std::string_view UVEConstant<UniqueID32>::DNAType() {
  return "UVEConstant<UniqueID32>"sv;
}
template <>
std::string_view UVEConstant<UniqueID64>::DNAType() {
  return "UVEConstant<UniqueID64>"sv;
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
  w.writeBytes("CNST", 4);
  tex.write(w);
}

AT_SUBSPECIALIZE_DNA_YAML(UVEConstant<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(UVEConstant<UniqueID64>)

template struct UVEConstant<UniqueID32>;
template struct UVEConstant<UniqueID64>;

template <>
std::string_view UVEAnimTexture<UniqueID32>::DNAType() {
  return "UVEAnimTexture<UniqueID32>"sv;
}
template <>
std::string_view UVEAnimTexture<UniqueID64>::DNAType() {
  return "UVEAnimTexture<UniqueID64>"sv;
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
    loop = r.readBool();
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
  w.writeBytes("CNST", 4);
  tex.write(w);
  tileW.write(w);
  tileH.write(w);
  strideW.write(w);
  strideH.write(w);
  cycleFrames.write(w);
  w.writeBytes("CNST", 4);
  w.writeBool(loop);
}

AT_SUBSPECIALIZE_DNA_YAML(UVEAnimTexture<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(UVEAnimTexture<UniqueID64>)

template struct UVEAnimTexture<UniqueID32>;
template struct UVEAnimTexture<UniqueID64>;

template <>
std::string_view UVElementFactory<UniqueID32>::DNAType() {
  return "UVElementFactory<UniqueID32>"sv;
}
template <>
std::string_view UVElementFactory<UniqueID64>::DNAType() {
  return "UVElementFactory<UniqueID64>"sv;
}

template <>
std::string_view SpawnSystemKeyframeData<UniqueID32>::SpawnSystemKeyframeInfo::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID32>::SpawnSystemKeyframeInfo"sv;
}
template <>
std::string_view SpawnSystemKeyframeData<UniqueID64>::SpawnSystemKeyframeInfo::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID64>::SpawnSystemKeyframeInfo"sv;
}

template <class IDType>
template <class Op>
void SpawnSystemKeyframeData<IDType>::SpawnSystemKeyframeInfo::Enumerate(typename Op::StreamT& s) {
  Do<Op>(athena::io::PropId{"id"}, id, s);
  Do<Op>(athena::io::PropId{"a"}, a, s);
  Do<Op>(athena::io::PropId{"b"}, b, s);
  Do<Op>(athena::io::PropId{"c"}, c, s);
}

template <>
std::string_view SpawnSystemKeyframeData<UniqueID32>::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID32>"sv;
}
template <>
std::string_view SpawnSystemKeyframeData<UniqueID64>::DNAType() {
  return "SpawnSystemKeyframeData<UniqueID64>"sv;
}

template <class IDType>
void SpawnSystemKeyframeData<IDType>::_read(typename ReadYaml::StreamT& r) {
  if (auto rec = r.enterSubRecord("a"))
    a = r.readUint32();
  if (auto rec = r.enterSubRecord("b"))
    b = r.readUint32();
  if (auto rec = r.enterSubRecord("endFrame"))
    endFrame = r.readUint32();
  if (auto rec = r.enterSubRecord("d"))
    d = r.readUint32();
  spawns.clear();
  size_t spawnCount;
  if (auto v = r.enterSubVector("spawns", spawnCount)) {
    spawns.reserve(spawnCount);
    for (const auto& child : r.getCurNode()->m_seqChildren) {
      (void)child;
      if (auto rec = r.enterSubRecord()) {
        spawns.emplace_back();
        spawns.back().first = r.readUint32("startFrame");
        size_t systemCount;
        if (auto v = r.enterSubVector("systems", systemCount)) {
          spawns.back().second.reserve(systemCount);
          for (const auto& in : r.getCurNode()->m_seqChildren) {
            (void)in;
            spawns.back().second.emplace_back();
            SpawnSystemKeyframeInfo& info = spawns.back().second.back();
            if (auto rec = r.enterSubRecord())
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
      if (auto rec = w.enterSubRecord()) {
        w.writeUint32("startFrame", spawn.first);
        if (auto v = w.enterSubVector("systems"))
          for (const auto& info : spawn.second)
            if (auto rec = w.enterSubRecord())
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
    w.writeBytes("NONE", 4);
    return;
  }
  w.writeBytes("CNST", 4);
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
std::string_view ChildResourceFactory<UniqueID32>::DNAType() {
  return "ChildResourceFactory<UniqueID32>"sv;
}
template <>
std::string_view ChildResourceFactory<UniqueID64>::DNAType() {
  return "ChildResourceFactory<UniqueID64>"sv;
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
    w.writeBytes("CNST", 4);
    id.write(w);
  } else
    w.writeBytes("NONE", 4);
}

AT_SUBSPECIALIZE_DNA_YAML(ChildResourceFactory<UniqueID32>)
AT_SUBSPECIALIZE_DNA_YAML(ChildResourceFactory<UniqueID64>)

template struct ChildResourceFactory<UniqueID32>;
template struct ChildResourceFactory<UniqueID64>;

} // namespace DataSpec::DNAParticle
