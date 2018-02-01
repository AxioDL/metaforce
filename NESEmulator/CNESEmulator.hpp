#ifndef __URDE_CNESEMULATOR_HPP__
#define __URDE_CNESEMULATOR_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/audiodev/IAudioVoice.hpp"
#include "zeus/CMatrix4f.hpp"

namespace urde
{
class CFinalInput;
class IDvdRequest;

namespace MP1
{

#define NUM_AUDIO_BUFFERS 10

class CNESEmulator : public boo::IAudioVoiceCallback
{
    static bool EmulatorConstructed;

    std::unique_ptr<u8[]> m_nesEmuPBuf;
    std::shared_ptr<IDvdRequest> m_dvdReq;

    struct Vert
    {
        zeus::CVector3f m_pos;
        zeus::CVector2f m_uv;
    };

    struct Uniform
    {
        zeus::CMatrix4f m_matrix;
        zeus::CColor m_color;
    };

    boo::ObjToken<boo::ITextureD> m_texture;
    boo::ObjToken<boo::IGraphicsBufferD> m_uniBuf;
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IShaderDataBinding> m_shadBind;

    std::unique_ptr<u8[]> m_audioBufBlock;
    u8* m_audioBufs[NUM_AUDIO_BUFFERS];
    int m_headBuf = 0;
    int m_tailBuf = 0;
    int m_procBufs = NUM_AUDIO_BUFFERS;
    size_t m_posInBuf = 0;
    boo::ObjToken<boo::IAudioVoice> m_booVoice;

    bool x20_wantsQuit = false;
    u8 x21_saveState[18];
    bool x34_wantsLoad = false;
    bool x38_stateLoaded = false;
    u8 x39_loadState[18];
    static void DecompressROM(u8* dataIn, u8* dataOut, u32 dataOutLen = 0x20000, u8 descrambleSeed = 0xe9,
                              u32 checkDataLen = 0x1FFFC, u32 checksumMagic = 0xA663);
    void InitializeEmulator();
    void DeinitializeEmulator();
    void NesEmuMainLoop();
public:
    CNESEmulator();
    ~CNESEmulator();
    void ProcessUserInput(const CFinalInput& input, int);
    void Update();
    void Draw(const zeus::CColor& mulColor, bool filtering);
    void LoadState(const u8* state);
    const u8* GetSaveState() const { return x21_saveState; }
    bool WantsQuit() const { return x20_wantsQuit; }
    bool WantsLoad() const { return x34_wantsLoad; }

    int audioUpdate();
    void preSupplyAudio(boo::IAudioVoice& voice, double dt) {}
    size_t supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data);
};

}
}

#endif // __URDE_CNESEMULATOR_HPP__
