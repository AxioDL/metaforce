#include "CCubeSurface.hpp"

#include "Streams/IOStreams.hpp"

namespace metaforce {
CCubeSurface::CCubeSurface(const u8* ptr, u32 len) : x0_data(ptr) {
  CMemoryInStream mem(ptr, len, CMemoryInStream::EOwnerShip::NotOwned);
  x0_center = mem.Get<zeus::CVector3f>();
  xc_materialIndex = mem.ReadLong();
  x10_displayListSize = mem.ReadLong();
  mem.ReadLong(); // x14_parent
  mem.ReadLong(); // x18_nextSurface
  x1c_extraSize = mem.ReadLong();
  x20_normal = mem.Get<zeus::CVector3f>();
  if (x1c_extraSize > 0) {
    x24_bounds = mem.Get<zeus::CAABox>();
  }
}
} // namespace metaforce
