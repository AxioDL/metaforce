#include "Kyoto/Animation/CFBStreamedCompression.hpp"

#include "Kyoto/Animation/CAnimPOIData.hpp"
#include "Kyoto/Animation/CFBStreamedAnimReader.hpp"
#include "Kyoto/IObjectStore.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/SObjectTag.hpp"

rstl::auto_ptr< uint > CFBStreamedCompression::GetRotationsAndOffsets(uint words,
                                                                      CInputStream& in) {
  rstl::auto_ptr< uint > data(rs_new uint[words]);
  void* cursor = data.get();
  CStandardMultiFormatHeader* mainHeader = static_cast< CStandardMultiFormatHeader* >(cursor);
  new (mainHeader) CStandardMultiFormatHeader(in);
  cursor = const_cast< void* >(mainHeader->AfterEnd());
  CFBStreamedCompressionTimeHeader* timeHeader =
      static_cast< CFBStreamedCompressionTimeHeader* >(cursor);
  new (timeHeader) CFBStreamedCompressionTimeHeader(in);
  cursor = const_cast< void* >(timeHeader->AfterEnd());
  in.Get< uint >();
  CFBStreamedPerChannelHeaderList* channels =
      static_cast< CFBStreamedPerChannelHeaderList* >(cursor);
  new (channels) CFBStreamedPerChannelHeaderList(in);
  const CFBStreamedPerChannelHeader& first = *channels->begin();
  cursor = const_cast< uchar* >(channels->AfterEnd());
  uint wordCount = static_cast< uint >(
      static_cast< float >(
          channels->GetSumOfBitCounts() * first.GetRotationBitStorage().GetWidth() + 31) /
      32.f);
  for (uint i = 0; i < wordCount; ++i) {
    TLoadedVal< uint >::Write(cursor, in.Get< uint >());
    cursor = static_cast< uchar* >(cursor) + sizeof(uint);
  }
  return data;
}

CFBStreamedCompression::CFBStreamedCompression(CInputStream& in, IObjectStore& store)
: mScratchSize(in.Get< uint >())
, mEvnt(in.Get< uint >())
, mEvntToken(nullptr)
, mRotsAndOffs(GetRotationsAndOffsets(mScratchSize / 4 + 1, in).release())
, mRootOffset(0.f, 0.f, 0.f) {
  if (mEvnt != 0) {
    mEvntToken = rs_new TLockedToken< CAnimPOIData >(store.GetObj(SObjectTag('EVNT', mEvnt)));
  }

  const CStandardMultiFormatHeader& mainHeader = MainHeader();
  const CFBStreamedCompressionTimeHeader& timeHeader = TimeHeader(mainHeader);
  const CFBStreamedPerChannelHeaderList& channels = GetPerChannelHeaderList(timeHeader);

  const CFBStreamedPerChannelHeader* firstChannel = &*channels.begin();
  const uint* bytes = GetBytes(channels);
  uint keyframes = GetNumKeyframes();
  CMemoryInputToBitLevelLoader input(bytes);
  CBitLevelLoader< CMemoryInputToBitLevelLoader > loader(input);
  uint rootIndex = 0;
  for (CFBStreamedPerChannelHeaderList::const_iterator channel(firstChannel, channels.size());
       channel != channels.end(); ++channel) {
    if (channel->GetSegId() == CSegId::Root()) {
      break;
    }
    ++rootIndex;
  }

  CFBStreamedAnimReaderTotals totals(*this);
  totals.CalculateDown();
  CVector3f previous = totals.GetVector(rootIndex);
  float distance = 0.f;
  for (uint i = 0; i < keyframes; ++i) {
    totals.IncrementInto(loader, *this, totals);
    totals.CalculateDown();
    CVector3f current = totals.GetVector(rootIndex);
    CVector3f difference = current - previous;
    previous = current;
    float delta = difference.Magnitude();
    if (!close_enough(delta, 0.f)) {
      distance += delta;
    }
  }
  mAverageVelocity = distance / GetAnimationDuration().GetSeconds();
}

CFBStreamedCompression::~CFBStreamedCompression() {}

CCharAnimTime CFBStreamedCompression::GetAnimationDuration() const {
  return MainHeader().GetMaxTime();
}
