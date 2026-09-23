#include "Kyoto/Audio/CAudioGroupSet.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Audio/CAudioSys.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "rstl/auto_ptr.hpp"
#include <string.h>
#if defined(TARGET_PC)
#include "Metaforce/AudioAssets.hpp"
#include "Metaforce/Common.hpp"
namespace { constexpr borealis::Log Log{"audio assets"}; }
#endif

CAudioGroupSet::CAudioGroupSet(const TLockedToken< CAudioGrpSetLoc >& group)
: x0_baseDir(group->GetBaseDirName())
, x10_groupSetName(group->GetGroupSetName())
, x20_groupSetTok(group) {}

CAudioGroupSet::~CAudioGroupSet() {}

void CAudioGroupSet::Reload() {}

void CAudioGroupSet::FreeSampleBuffer() { x20_groupSetTok.data()->FreeSampleBuffer(); }

CAudioGrpSetLoc::CAudioGrpSetLoc(const rstl::auto_ptr< uchar >& data, int length)
: x0_data(data.release())
, x30_aramSize(0)
, x34_pool(nullptr)
, x38_project(nullptr)
, x3c_sampleDir(nullptr)
, x40_samples(nullptr) {
#if defined(TARGET_PC)
  metaforce::AudioGroupView view;
  REQUIRE(length >= 0 && metaforce::ReadAudioGroup({x0_data.get(), size_t(length)}, view),
          "Invalid AGSC wrapper ({} bytes)", length);
  SND_PC_ASSET_ERROR error{};
  REQUIRE(sndPCValidateGroup(&view.assets, &error), "Invalid AGSC {} at {}+{}: {}", view.name,
          error.section ? error.section : "", error.offset, error.reason ? error.reason : "");
  x10_baseDirName = rstl::string(view.baseDirectory.data(), view.baseDirectory.size());
  x20_groupSetName = rstl::string(view.name.data(), view.name.size());
  const size_t poolSize = (view.assets.pool.size + 3) & ~size_t(3);
  const size_t projectSize = (view.assets.project.size + 3) & ~size_t(3);
  x8_groupData = rstl::auto_ptr<uchar>(static_cast<uchar*>(
      CMemory::Alloc(poolSize + projectSize + view.assets.directory.size, IAllocator::kHI_RoundUpLen)));
  x34_pool = x8_groupData.get();
  x38_project = x34_pool + poolSize;
  x3c_sampleDir = x38_project + projectSize;
  memcpy(x34_pool, view.assets.pool.data, view.assets.pool.size);
  memcpy(x38_project, view.assets.project.data, view.assets.project.size);
  memcpy(x3c_sampleDir, view.assets.directory.data, view.assets.directory.size);
  x40_samples = const_cast<uchar*>(static_cast<const uchar*>(view.assets.samples.data));
  x30_aramSize = view.assets.samples.size;
#else
  uint readPosition;
  const uint poolSize = ReadHeader(data.get(), length, readPosition);
  CAudioSys::GetVerbose();

  const uint projectOffset = readPosition + poolSize;
#if TARGET_LITTLE_ENDIAN
  const uint projectSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + projectOffset));
#else
  const uint projectSize = *reinterpret_cast< uint* >(data.get() + projectOffset);
  CAudioSys::GetVerbose();

  const uint sampOffset = 4 + projectSize + projectOffset;
#if TARGET_LITTLE_ENDIAN
  const uint sampSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + sampOffset));
#else
  const uint sampSize = *reinterpret_cast< uint* >(data.get() + sampOffset);
#endif
  CAudioSys::GetVerbose();
  x30_aramSize = sampSize;

  const uint sdirOffset = 4 + sampOffset + sampSize;
#if TARGET_LITTLE_ENDIAN
  const uint sdirSize = CBasics::SwapBytes(*reinterpret_cast< uint* >(data.get() + sdirOffset));
#else
  const uint sdirSize = *reinterpret_cast< uint* >(data.get() + sdirOffset);
#endif
  CAudioSys::GetVerbose();

  x8_groupData = rstl::auto_ptr< uchar >(static_cast< uchar* >(
      CMemory::Alloc(poolSize + projectSize + sdirSize + 8, IAllocator::kHI_RoundUpLen)));
  uchar* ptr = x0_data.get();
  x34_pool = x8_groupData.get();
  memcpy(x34_pool, ptr + readPosition, poolSize);

  uint roundedPoolSize = ((poolSize + 3) & ~3);
  x38_project = x8_groupData.get() + roundedPoolSize;
  memcpy(x38_project, ptr + (projectOffset + 4), projectSize);

  uint roundedProjectSize = ((projectSize + 3) & ~3);
  roundedProjectSize = roundedPoolSize + roundedProjectSize;
  x3c_sampleDir = x8_groupData.get() + roundedProjectSize;
  memcpy(x3c_sampleDir, ptr + (sdirOffset + 4), sdirSize);
  x40_samples = &ptr[sampOffset + 4];
#endif
#endif
}

void CAudioGrpSetLoc::FreeSampleBuffer() {
  x0_data = nullptr;
  x40_samples = nullptr;
}

template <>
CFactoryFnReturn::CFactoryFnReturn(CAudioGrpSetLoc* ptr)
: obj(TToken< CAudioGrpSetLoc >::GetIObjObjectFor(rstl::auto_ptr< CAudioGrpSetLoc >(ptr))
          .release()) {}

const CFactoryFnReturn FAudioGroupSetLocDataFactory(const SObjectTag& tag,
                                                    const rstl::auto_ptr< uchar >& data, int length,
                                                    const CVParamTransfer& xfer) {
  return rs_new CAudioGrpSetLoc(data, length);
}
