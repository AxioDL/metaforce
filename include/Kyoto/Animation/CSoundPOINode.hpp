#ifndef _CSOUNDPOINODE
#define _CSOUNDPOINODE

#include "Kyoto/Animation/CPOINode.hpp"
#include "Kyoto/Streams/CInputStream.hpp"

class CSoundPOINode : public CPOINode {
public:
  CSoundPOINode(const rstl::string name = "", const EPOIType type = kPT_Sound,
                const CCharAnimTime& time = CCharAnimTime(), const int index = -1,
                const bool unique = false, const float weight = 1.f, const int charIdx = -1,
                const int flags = 0, const int sfxId = 0, const float fallOff = 0.f,
                const float maxDist = 0.f)
  : CPOINode(name, type, time, index, unique, weight, charIdx, flags)
  , mSfxId(sfxId)
  , mFalloff(fallOff)
  , mMaxDist(maxDist) {}

  CSoundPOINode(CInputStream& in)
  : CPOINode(in)
  , mSfxId(in.ReadInt32())
  , mFalloff(in.Get< float >())
  , mMaxDist(in.Get< float >()) {}

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
