#ifndef __URDE_CSTREAMAUDIOMANAGER_HPP__
#define __URDE_CSTREAMAUDIOMANAGER_HPP__

#include "RetroTypes.hpp"

namespace urde
{

class CStreamAudioManager
{
    static u8 g_MusicVolume;
    static u8 g_SfxVolume;
    static bool g_MusicUnmute;
    static bool g_SfxUnmute;

    static float GetTargetDSPVolume(float fileVol, bool music);
    static void StopStreaming(bool oneshot);
    static void UpdateDSP(bool oneshot, float dt);
    static void UpdateDSPStreamers(float dt);

public:
    static void Start(bool oneshot, const std::string& fileName, u8 volume,
                      bool music, float fadeIn, float fadeOut);
    static void Stop(bool oneshot, const std::string& fileName);
    static void FadeBackIn(bool oneshot, float fadeTime);
    static void TemporaryFadeOut(bool oneshot, float fadeTime);
    static void Update(float dt);
    static void Initialize();
};

}

#endif // __URDE_CSTREAMAUDIOMANAGER_HPP__
