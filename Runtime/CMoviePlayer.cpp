#include "CMoviePlayer.hpp"

namespace Retro
{

CMoviePlayer::CMoviePlayer(const char* path, float startTime, bool flag)
: CDvdFile(path)
{
}

void CMoviePlayer::VerifyCallbackStatus()
{
}
void CMoviePlayer::DisableStaticAudio()
{
}
void CMoviePlayer::SetStaticAudioVolume(int vol)
{
}
void CMoviePlayer::SetStaticAudio(const void* data, u32 length, u32 loopStart, u32 loopEnd)
{
}
void CMoviePlayer::MixAudio(short* out, const short* in, u32 length)
{
}
void CMoviePlayer::MixStaticAudio(short* out, const short* in, u32 length)
{
}
void CMoviePlayer::StaticMyAudioCallback()
{
}
void CMoviePlayer::Rewind()
{
}

bool CMoviePlayer::GetIsMovieFinishedPlaying() const
{
}
bool CMoviePlayer::GetIsFullyCached() const
{
}
float CMoviePlayer::GetPlayedSeconds() const
{
}
float CMoviePlayer::GetTotalSeconds() const
{
}
void CMoviePlayer::SetPlayMode(EPlayMode mode)
{
}
void CMoviePlayer::DrawFrame(const CVector3f& a, const CVector3f& b,
                             const CVector3f& c, const CVector3f& d)
{
}
void CMoviePlayer::Update(float dt)
{
}
void CMoviePlayer::DecodeFromRead(const void* data)
{
}
void CMoviePlayer::ReadCompleted()
{
}
void CMoviePlayer::PostDVDReadRequestIfNeeded()
{
}
void CMoviePlayer::InitializeTextures()
{
}

}
