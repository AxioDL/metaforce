#include "Runtime/Audio/CAudioGroupSet.hpp"

#include <cstring>

namespace metaforce {

amuse::AudioGroupData CAudioGroupSet::LoadData() {
  const auto readU32 = [](const u8* ptr) {
    uint32_t value;
    std::memcpy(&value, ptr, sizeof(value));
    return SBig(value);
  };

  athena::io::MemoryReader r(m_buffer.get(), INT32_MAX);
  x10_baseName = r.readString();
  x20_name = r.readString();

  u8* buf = m_buffer.get() + r.position();
  const uint32_t poolLen = readU32(buf);
  unsigned char* pool = buf + 4;
  buf += poolLen + 4;
  const uint32_t projLen = readU32(buf);
  unsigned char* proj = buf + 4;
  buf += projLen + 4;
  const uint32_t sampLen = readU32(buf);
  unsigned char* samp = buf + 4;
  buf += sampLen + 4;
  const uint32_t sdirLen = readU32(buf);
  unsigned char* sdir = buf + 4;

  return {proj, projLen, pool, poolLen, sdir, sdirLen, samp, sampLen, amuse::GCNDataTag{}};
}

CAudioGroupSet::CAudioGroupSet(std::unique_ptr<u8[]>&& in) : m_buffer(std::move(in)), m_data(LoadData()) {}

CFactoryFnReturn FAudioGroupSetDataFactory(const metaforce::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                           const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef) {
  return TToken<CAudioGroupSet>::GetIObjObjectFor(std::make_unique<CAudioGroupSet>(std::move(in)));
}

} // namespace metaforce
