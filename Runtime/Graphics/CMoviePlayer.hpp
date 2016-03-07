#ifndef __PSHAG_CMOVIEPLAYER_HPP__
#define __PSHAG_CMOVIEPLAYER_HPP__

#include "RetroTypes.hpp"
#include "CDvdFile.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "specter/View.hpp"

namespace urde
{
class CVector3f;

class CMoviePlayer : public CDvdFile
{
public:
    enum class EPlayMode
    {
        Stopped,
        Playing
    };
private:
    struct THPHeader
    {
        u32 magic;
        u32 version;
        u32 maxBufferSize;
        u32 maxAudioSamples;
        float fps;
        u32 numFrames;
        u32 firstFrameSize;
        u32 dataSize;
        u32 componentDataOffset;
        u32 offsetsDataOffset;
        u32 firstFrameOffset;
        u32 lastFrameOffset;
        void swapBig();
    } x28_thpHead;

    struct THPComponents
    {
        u32 numComponents;
        enum class Type : u8
        {
            Video = 0x0,
            Audio = 0x1,
            None = 0xff
        } comps[16];
        void swapBig();
    } x58_thpComponents;

    struct THPVideoInfo
    {
        u32 width;
        u32 height;
        void swapBig();
    } x6c_videoInfo;

    struct THPAudioInfo
    {
        u32 numChannels;
        u32 sampleRate;
        u32 numSamples;
        void swapBig();
    } x74_audioInfo;

    struct THPFrameHeader
    {
        u32 nextSize;
        u32 prevSize;
        u32 imageSize;
        u32 audioSize;
        void swapBig();
    };

    struct THPAudioFrameHeader
    {
        u32 channelSize;
        u32 numSamples;
        s16 channelCoefs[2][8][2];
        s16 channelPrevs[2][2];
        void swapBig();
    };

    struct CTHPTextureSet
    {
        boo::ITextureD* Y[2] = {};
        boo::ITextureD* U = nullptr;
        boo::ITextureD* V = nullptr;
        std::unique_ptr<s16[]> audioBuf;
        boo::IShaderDataBinding* binding[2] = {};
    };
    std::vector<CTHPTextureSet> x80_textures;
    std::unique_ptr<uint8_t[]> x90_requestBuf;
    std::shared_ptr<IDvdRequest> x98_request;
    std::vector<std::unique_ptr<uint8_t[]>> xa0_bufferQueue;

    u32 xb0_nextReadSize = 0;
    u32 xb4_nextReadOff = 0;
    u32 xb8_readSize = 0;
    u32 xbc_readOff = 0;
    u32 xc0_loadedFrames = 0;
    u32 xc4_ = 0;
    u32 xc8_curFrame = 0;
    u32 xcc_decodedTexSlot = 0;
    u32 xd0_ = 0;
    u32 xd4_ = 0;
    s32 xd8_ = 0;
    float xdc_frameRem = 0.f;
    EPlayMode xe0_playMode = EPlayMode::Playing;
    float xe4_totalSeconds = 0.f;
    float xe8_curSeconds = 0.f;
    float xec_preLoadSeconds;
    u32 xf0_preLoadFrames = 0;
    u32 xf8_ = 0;
    u32 xfc_ = 0;

    boo::GraphicsDataToken m_token;
    std::unique_ptr<uint8_t[]> m_yuvBuf;

    union
    {
        struct
        {
            bool xf4_24_loop : 1; bool xf4_25_hasAudio : 1;
            bool xf4_26_fieldFlip : 1; bool m_deinterlace : 1;
        };
        u8 m_dummy = 0;
    };

    struct ViewBlock
    {
        zeus::CMatrix4f m_mv;
        zeus::CColor m_color = zeus::CColor::skWhite;
    } m_viewVertBlock;
    boo::IGraphicsBufferD* m_blockBuf;

    uint64_t m_loadedFrameCount = 0;
    uint64_t m_playedFrameCount = 0;

public:

    CMoviePlayer(const char* path, float preLoadSeconds, bool loop, bool deinterlace);

    static u32 THPAudioDecode(s16* buffer, const u8* audioFrame, bool stereo);
    static void VerifyCallbackStatus();
    static void DisableStaticAudio();
    static void SetStaticAudioVolume(int vol);
    static void SetStaticAudio(const void* data, u32 length, u32 loopStart, u32 loopEnd);
    void MixAudio(short* out, const short* in, u32 length);
    static void MixStaticAudio(short* out, const short* in, u32 length);
    static void StaticMyAudioCallback();
    void Rewind();

    bool GetIsMovieFinishedPlaying() const
    {
        if (xf4_24_loop)
            return false;
        return xc8_curFrame == x28_thpHead.numFrames;
    }
    bool GetIsFullyCached() const {return xa0_bufferQueue.size() >= xf0_preLoadFrames;}
    float GetPlayedSeconds() const {return xdc_frameRem + xe8_curSeconds;}
    float GetTotalSeconds() const {return xe4_totalSeconds;}
    void SetPlayMode(EPlayMode mode) {xe0_playMode = mode;}
    void DrawFrame(const CVector3f& a, const CVector3f& b, const CVector3f& c, const CVector3f& d);
    void Update(float dt);
    void DecodeFromRead(const void* data);
    void ReadCompleted();
    void PostDVDReadRequestIfNeeded();

    static void Initialize();
    static void Shutdown();
};

}

#endif // __PSHAG_CMOVIEPLAYER_HPP__
