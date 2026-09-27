#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(rstl::string name = "", ushort type = kPT_Sound,
                const CCharAnimTime& time = CCharAnimTime(), int index = -1, bool unique = false,
                float weight = 1.f, int charIdx = -1, int flags = 0, int sfxId = 0,
                float fallOff = 0.f, float maxDist = 0.f)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , mSfxId(sfxId)
  , mFalloff(fallOff)
  , mMaxDist(maxDist) {}

  CSoundPOINode(CInputStream& in)
  : CPOINode(in)
  , mSfxId(in.ReadInt32())
  , mFalloff(in.ReadFloat())
  , mMaxDist(in.ReadFloat()) {}

  uint GetSoundId() const { return mSfxId; }
  float GetFallOff() const { return mFalloff; }
  float GetMaxDistance() const { return mMaxDist; }

  static CSoundPOINode CopyNodeMinusStartTime(const CSoundPOINode& node,
                                              const CCharAnimTime& startTime) {
    return CSoundPOINode(node.GetString(), node.GetPoiType(), node.GetTime() - startTime,
                         node.GetIndex(), node.GetSaveState(), node.GetWeight(),
                         node.GetCharacterIndex(), node.GetFlags(), node.GetSoundId(),
                         node.GetFallOff(), node.GetMaxDistance());
  }

private:
  uint mSfxId;
  float mFalloff;
  float mMaxDist;
};

#endif // _CSOUNDPOINODE
