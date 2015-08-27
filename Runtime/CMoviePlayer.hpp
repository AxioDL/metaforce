#ifndef __RETRO_CMOVIEPLAYER_HPP__
#define __RETRO_CMOVIEPLAYER_HPP__

#include "RetroTypes.hpp"
#include "CDvdFile.hpp"

namespace Retro
{
class CVector3f;

class CMoviePlayer : public CDvdFile
{
public:
    enum EPlayMode
    {
    };

    CMoviePlayer(const char* path, float startTime, bool flag);

    static void VerifyCallbackStatus();
    static void DisableStaticAudio();
    static void SetStaticAudioVolume(int vol);
    static void SetStaticAudio(const void* data, u32 length, u32 loopStart, u32 loopEnd);
    void MixAudio(short* out, const short* in, u32 length);
    static void MixStaticAudio(short* out, const short* in, u32 length);
    static void StaticMyAudioCallback();
    void Rewind();

    bool GetIsMovieFinishedPlaying() const;
    bool GetIsFullyCached() const;
    float GetPlayedSeconds() const;
    float GetTotalSeconds() const;
    void SetPlayMode(EPlayMode);
    void DrawFrame(const CVector3f& a, const CVector3f& b, const CVector3f& c, const CVector3f& d);
    void Update(float dt);
    void DecodeFromRead(const void* data);
    void ReadCompleted();
    void PostDVDReadRequestIfNeeded();
    void InitializeTextures();
};

}

#endif // __RETRO_CMOVIEPLAYER_HPP__
