#ifndef __URDE_CNESEMULATOR_HPP__
#define __URDE_CNESEMULATOR_HPP__

#include "RetroTypes.hpp"
#include "zeus/CColor.hpp"
#include "boo/graphicsdev/IGraphicsDataFactory.hpp"
#include "boo/audiodev/IAudioVoice.hpp"
#include "zeus/CMatrix4f.hpp"

namespace urde
{
struct CFinalInput;
class IDvdRequest;

namespace MP1
{

#define NUM_AUDIO_BUFFERS 3

class CNESEmulator final : public boo::IAudioVoiceCallback
{
public:
    enum class EPasswordEntryState
    {
        NotPasswordScreen,
        NotEntered,
        Entered
    };
private:
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

    //void* x4_loadBuf;
    //void* x8_rom;
    //void* xc_state;
    //OSModuleInfo* x10_module = x4_loadBuf;
    //void* x14_bss;
    //void* x18_prgram;
    //void* x1c_wram;
    bool x20_gameOver = false;
    u8 x21_passwordFromNES[18];
    EPasswordEntryState x34_passwordEntryState = EPasswordEntryState::NotPasswordScreen;
    bool x38_passwordPending = false;
    u8 x39_passwordToNES[18];
    static void DecompressROM(u8* dataIn, u8* dataOut, u32 dataOutLen = 0x20000, u8 descrambleSeed = 0xe9,
                              u32 checkDataLen = 0x1FFFC, u32 checksumMagic = 0xA663);
    void InitializeEmulator();
    void DeinitializeEmulator();
    void NesEmuMainLoop(bool forceDraw = false);
    static bool CheckForGameOver(const u8* vram, u8* passwordOut = nullptr);
    static EPasswordEntryState CheckForPasswordEntryScreen(const uint8_t* vram);
    static bool SetPasswordIntoEntryScreen(u8* vram, u8* wram, const u8* password);
public:
    CNESEmulator();
    ~CNESEmulator();
    void ProcessUserInput(const CFinalInput& input, int);
    void Update();
    void Draw(const zeus::CColor& mulColor, bool filtering);
    void LoadPassword(const u8* state);
    const u8* GetPassword() const { return x21_passwordFromNES; }
    bool IsGameOver() const { return x20_gameOver; }
    EPasswordEntryState GetPasswordEntryState() const { return x34_passwordEntryState; }

    int audioUpdate();
    void preSupplyAudio(boo::IAudioVoice& voice, double dt) {}
    size_t supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data);
};

}
}

#endif // __URDE_CNESEMULATOR_HPP__
