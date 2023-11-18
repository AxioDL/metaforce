#pragma once

#include <memory>
#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Character/CAnimPOIData.hpp"

#include <zeus/CVector3f.hpp>

namespace metaforce {
class IObjectStore;

class CFBStreamedCompression {
  friend class CFBStreamedAnimReader;
  friend class CFBStreamedAnimReaderTotals;
  friend class CFBStreamedPairOfTotals;

public:
  struct Header {
    u32 unk0;
    float duration;
    float interval;
    u32 rootBoneId;
    u32 looping;
    u32 rotDiv;
    float translationMult;
    u32 boneChannelCount;
    u32 unk3;

    void read(CInputStream& in) {
      /* unk0 */
      unk0 = in.ReadLong();
      /* duration */
      duration = in.ReadFloat();
      /* interval */
      interval = in.ReadFloat();
      /* rootBoneId */
      rootBoneId = in.ReadLong();
      /* looping */
      looping = in.ReadLong();
      /* rotDiv */
      rotDiv = in.ReadLong();
      /* translationMult */
      translationMult = in.ReadFloat();
      /* boneChannelCount */
      boneChannelCount = in.ReadLong();
      /* unk3 */
      unk3 = in.ReadLong();
    }
  };

private:
  bool m_pc;
  u32 x0_scratchSize;
  CAssetId x4_evnt;
  TLockedToken<CAnimPOIData> x8_evntToken;
  std::unique_ptr<u32[]> xc_rotsAndOffs;
  float x10_averageVelocity;
  zeus::CVector3f x14_rootOffset;

  u8* ReadBoneChannelDescriptors(u8* out, CInputStream& in) const;
  u32 ComputeBitstreamWords(const u8* chans) const;
  std::unique_ptr<u32[]> GetRotationsAndOffsets(u32 words, CInputStream& in) const;
  float CalculateAverageVelocity(const u8* chans) const;

public:
  explicit CFBStreamedCompression(CInputStream& in, IObjectStore& objStore, bool pc);
  const Header& MainHeader() const { return *reinterpret_cast<const Header*>(xc_rotsAndOffs.get()); }
  const u32* GetTimes() const;
  const u8* GetPerChannelHeaders() const;
  const u8* GetBitstreamPointer() const;
  bool IsLooping() const { return MainHeader().looping; }
  CCharAnimTime GetAnimationDuration() const { return MainHeader().duration; }
  float GetAverageVelocity() const { return x10_averageVelocity; }
  const zeus::CVector3f& GetRootOffset() const { return x14_rootOffset; }
  bool HasPOIData() const { return x8_evntToken.HasReference(); }
  const std::vector<CBoolPOINode>& GetBoolPOIStream() const { return x8_evntToken->GetBoolPOIStream(); }
  const std::vector<CInt32POINode>& GetInt32POIStream() const { return x8_evntToken->GetInt32POIStream(); }
  const std::vector<CParticlePOINode>& GetParticlePOIStream() const { return x8_evntToken->GetParticlePOIStream(); }
  const std::vector<CSoundPOINode>& GetSoundPOIStream() const { return x8_evntToken->GetSoundPOIStream(); }
};

} // namespace metaforce
