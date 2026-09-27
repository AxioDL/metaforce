#ifndef _CSTATICAUDIOPLAYER
#define _CSTATICAUDIOPLAYER

#include "dolphin/hw_regs.h"

#include <Kyoto/Audio/g721.h>
#include <rstl/auto_ptr.hpp>
#include <rstl/string.hpp>
#include <rstl/vector.hpp>

class CDvdRequest;
#if defined(TARGET_PC)
typedef void (*FAudioCallback)(short* output, size_t frames, u32 rate, u32 channels);
#else
typedef void (*FAudioCallback)();
#endif

class CStaticAudioPlayer {
public:
  CStaticAudioPlayer(const rstl::string& filepath, const int w1, const int w2);
  ~CStaticAudioPlayer();

  const bool IsReady() const;
  void StartMixOut();
  void StopMixOut();

#if defined(TARGET_PC)
  static void MixCallback(short* output, size_t frames, u32 rate, u32 channels);
  void DoMix(short* output, size_t frames, u32 rate, u32 channels);
#else
  static void MixCallback();
  void DoMix();
#endif
  static void RunDMACallback(FAudioCallback);
  static void CancelDMACallback(FAudioCallback);
  static void InstallAICallback();
#if defined(TARGET_PC)
  static void AICallback(short* output, size_t frames, u32 rate, u32 channels);
#else
  static void AICallback();
#endif

  void Decode(const ushort* bufIn, ushort* bufOut, int numSamples);
  void DecodeMonoAndMix(ushort* bufIn, ushort* bufOut, int numSamples,
                        int curSample, int sampleEnd, int sampleStart,
                        int vol, g72x_state& state);
  void SetVolume(uchar vol);

private:
  rstl::string mFilepath;
  int mRsfRem;
  int mRsfLength;
  int mCurSamp;
  int mLoopStartSamp;
  int mLoopEndSamp;
  int mCurBuf;
  rstl::auto_ptr< uchar > mDmaLeft;
  rstl::auto_ptr< uchar > mDmaRight;
  rstl::vector< rstl::auto_ptr< CDvdRequest > > mDvdRequests;
  rstl::vector< rstl::auto_ptr< uchar > > mBuffers;
  g72x_state mLeftState;
  g72x_state mRightState;
  ushort mVolume;
#if defined(TARGET_PC)
  u32 m_audioPhase = 0;
  short m_audioHistory[2][2] = {};
  short m_audioPair[4] = {};
  u32 m_audioPairRead = 2;
  bool m_audioPrimed = false;
#endif
};

#endif // _CSTATICAUDIOPLAYER
