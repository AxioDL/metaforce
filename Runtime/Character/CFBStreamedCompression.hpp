#ifndef __URDE_CFSTREAMEDCOMPRESSION_HPP__
#define __URDE_CFSTREAMEDCOMPRESSION_HPP__

#include "RetroTypes.hpp"
#include "CToken.hpp"
#include "CAnimPOIData.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class IObjectStore;

class CFBStreamedCompression
{
    friend class CFBStreamedAnimReader;
    friend class CFBStreamedAnimReaderTotals;
    friend class CFBStreamedPairOfTotals;

public:
    struct Header
    {
        u32 unk0;
        float duration;
        float interval;
        u32 rootBoneId;
        u32 looping;
        u32 rotDiv;
        float translationMult;
        u32 boneChannelCount;
        u32 unk3;

        void read(CInputStream& in)
        {
            /* unk0 */
            unk0 = in.readUint32Big();
            /* duration */
            duration = in.readFloatBig();
            /* interval */
            interval = in.readFloatBig();
            /* rootBoneId */
            rootBoneId = in.readUint32Big();
            /* looping */
            looping = in.readUint32Big();
            /* rotDiv */
            rotDiv = in.readUint32Big();
            /* translationMult */
            translationMult = in.readFloatBig();
            /* boneChannelCount */
            boneChannelCount = in.readUint32Big();
            /* unk3 */
            unk3 = in.readUint32Big();
        }
    };

private:
    bool m_pc;
    u32 x0_scratchSize;
    ResId x4_evnt;
    TLockedToken<CAnimPOIData> x8_evntToken;
    std::unique_ptr<u32[]> xc_rotsAndOffs;
    zeus::CVector3f x14_rootOffset;

    u8* ReadBoneChannelDescriptors(u8* out, CInputStream& in);
    u32 ComputeBitstreamWords(const u8* chans);
    std::unique_ptr<u32[]> GetRotationsAndOffsets(u32 words, CInputStream& in);

public:
    CFBStreamedCompression(CInputStream& in, IObjectStore& objStore, bool pc);
    const Header& MainHeader() const { return *reinterpret_cast<const Header*>(xc_rotsAndOffs.get()); }
    const u8* GetPerChannelHeaders() const;
    const u8* GetBitstreamPointer() const;
    bool IsLooping() const { return MainHeader().looping; }
    CCharAnimTime GetAnimationDuration() const { return MainHeader().duration; }
    const zeus::CVector3f& GetRootOffset() const { return x14_rootOffset; }
    bool HasPOIData() const { return x8_evntToken; }
    const std::vector<CBoolPOINode>& GetBoolPOIStream() const { return x8_evntToken->GetBoolPOIStream(); }
    const std::vector<CInt32POINode>& GetInt32POIStream() const { return x8_evntToken->GetInt32POIStream(); }
    const std::vector<CParticlePOINode>& GetParticlePOIStream() const { return x8_evntToken->GetParticlePOIStream(); }
    const std::vector<CSoundPOINode>& GetSoundPOIStream() const { return x8_evntToken->GetSoundPOIStream(); }
};

}

#endif // __URDE_CFSTREAMEDCOMPRESSION_HPP__
