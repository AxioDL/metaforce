#ifndef __URDE_CSTATICAUDIOPLAYER_HPP__
#define __URDE_CSTATICAUDIOPLAYER_HPP__

#include "CAudioSys.hpp"
#include "RetroTypes.hpp"
#include "g721.h"
#include "boo/audiodev/IAudioVoice.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"

namespace urde
{
class IDvdRequest;

class CStaticAudioPlayer
{
    std::string x0_path;
    u32 x10_rsfRem = -1;
    u32 x14_rsfLength;
    u32 x18_curSamp = 0;
    u32 x1c_loopStartSamp;
    u32 x20_loopEndSamp;
    //u32 x24_ = 0;
    //std::unique_ptr<u8[]> x28_dmaLeft;
    //std::unique_ptr<u8[]> x30_dmaRight;
    std::vector<std::shared_ptr<IDvdRequest>> x38_dvdRequests;
    std::vector<std::unique_ptr<u8[]>> x48_buffers;
    g72x_state x58_leftState;
    g72x_state x8c_rightState;
    std::experimental::optional<g72x_state> m_leftStateLoop;
    std::experimental::optional<g72x_state> m_rightStateLoop;
    u32 xc0_volume = 32768; // Out of 32768

    static int16_t SampClamp(int32_t val)
    {
        if (val < -32768) val = -32768;
        else if (val > 32767) val = 32767;
        return val;
    }

    struct AudioVoiceCallback : boo::IAudioVoiceCallback
    {
        CStaticAudioPlayer& m_parent;
        void preSupplyAudio(boo::IAudioVoice&, double) {}
        size_t supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data)
        {
            if (m_parent.IsReady())
            {
                m_parent.x38_dvdRequests.clear();
                m_parent.Decode(data, frames);
            }
            else
                memset(data, 0, 4 * frames);
            return frames;
        }
        AudioVoiceCallback(CStaticAudioPlayer& p) : m_parent(p) {}
    } m_voiceCallback;
    boo::ObjToken<boo::IAudioVoice> m_voice;

public:
    CStaticAudioPlayer(boo::IAudioVoiceEngine& engine, std::string_view path,
                       int loopStart, int loopEnd);
    CStaticAudioPlayer(std::string_view path,
                       int loopStart, int loopEnd)
    : CStaticAudioPlayer(*CAudioSys::GetVoiceEngine(), path, loopStart, loopEnd) {}

    bool IsReady();
    void DecodeMonoAndMix(s16* bufOut, u32 numSamples,
                          u32 cur, u32 loopEndCur, u32 loopStartCur,
                          int vol, g72x_state& state,
                          std::experimental::optional<g72x_state>& loopState) const;
    void Decode(s16* bufOut, u32 numSamples);
    void SetVolume(float vol)
    {
        xc0_volume = zeus::clamp(0.f, vol, 1.f) * 32768.f;
    }

    void StartMixing()
    {
        m_voice->start();
    }
    void StopMixing()
    {
        m_voice->stop();
    }
};

}

#endif // __URDE_CSTATICAUDIOPLAYER_HPP__
