#include "Runtime/Audio/CAudioGroupSet.hpp"

namespace urde {

amuse::AudioGroupData CAudioGroupSet::LoadData() {
  athena::io::MemoryReader r(m_buffer.get(), INT32_MAX);
  x10_baseName = r.readString();
  x20_name = r.readString();

  u8* buf = m_buffer.get() + r.position();
  uint32_t poolLen = hecl::SBig(*reinterpret_cast<uint32_t*>(buf));
  unsigned char* pool = buf + 4;
  buf += poolLen + 4;
  uint32_t projLen = hecl::SBig(*reinterpret_cast<uint32_t*>(buf));
  unsigned char* proj = buf + 4;
  buf += projLen + 4;
  uint32_t sampLen = hecl::SBig(*reinterpret_cast<uint32_t*>(buf));
  unsigned char* samp = buf + 4;
  buf += sampLen + 4;
  uint32_t sdirLen = hecl::SBig(*reinterpret_cast<uint32_t*>(buf));
  unsigned char* sdir = buf + 4;

  return {proj, projLen, pool, poolLen, sdir, sdirLen, samp, sampLen, amuse::GCNDataTag{}};
}

CAudioGroupSet::CAudioGroupSet(std::unique_ptr<u8[]>&& in) : m_buffer(std::move(in)), m_data(LoadData()) {}

CFactoryFnReturn FAudioGroupSetDataFactory(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const urde::CVParamTransfer& vparms, CObjectReference* selfRef) {
  return TToken<CAudioGroupSet>::GetIObjObjectFor(std::make_unique<CAudioGroupSet>(std::move(in)));
}

} // namespace urde
