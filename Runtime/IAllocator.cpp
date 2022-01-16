#include "IAllocator.hpp"

namespace metaforce {
IAllocator::SMetrics::SMetrics(u32 heapSize, u32 unk1, u32 unk2, u32 unk3, u32 unk4, u32 heapSize2, u32 unk5, u32 unk6,
                               u32 unk7, u32 unk8, u32 unk9, u32 smallAllocNumAllocs, u32 smallAllocAllocatedSize,
                               u32 smallAllocRemainingSize, u32 mediumAllocNumAllocs, u32 mediumAllocAllocatedSize,
                               u32 mediumAllocBlocksAvailable, u32 unk10, u32 unk11, u32 unk12,
                               u32 mediumAllocTotalAllocated, u32 fakeStatics)
: x0_heapSize(heapSize)
, x4_(unk1)
, x8_(unk2)
, xc_(unk3)
, x10_(unk4)
, x14_heapSize2(heapSize2)
, x18_(unk5)
, x1c_(unk6)
, x20_(unk7)
, x24_(unk8)
, x28_(unk9)
, x2c_smallNumAllocs(smallAllocNumAllocs)
, x30_smallAllocatedSize(smallAllocAllocatedSize)
, x34_smallRemainingSize(smallAllocRemainingSize)
, x38_mediumNumAllocs(mediumAllocNumAllocs)
, x3c_mediumAllocatedSize(mediumAllocAllocatedSize)
, x40_mediumBlocksAvailable(mediumAllocBlocksAvailable)
, x44_(unk10)
, x48_(unk11)
, x4c_(unk12)
, x50_mediumTotalAllocated(mediumAllocTotalAllocated)
, x54_fakeStatics(fakeStatics) {}
}
