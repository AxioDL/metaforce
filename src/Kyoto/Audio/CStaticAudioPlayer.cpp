#include "Kyoto/Audio/CAudioSys.hpp"
#include "rstl/algorithm.hpp"
#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/Audio/CStaticAudioPlayer.hpp>

#include <Kyoto/CDvdFile.hpp>
#include <Kyoto/CDvdRequest.hpp>

#include <rstl/list.hpp>
#include <rstl/math.hpp>

#include <dolphin/ai.h>
#include <dolphin/os.h>

#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include <algorithm>
#endif

static CStaticAudioPlayer* sCurrentPlayer = nullptr;
static rstl::reserved_vector< FAudioCallback, 4 > sAICallbacks;
#if !defined(TARGET_PC)
ATTRIBUTE_ALIGN_DECL(8, static bool sDMACallbackInstalled) = false;
static FAudioCallback sOldDMACallback = nullptr;
#endif

void CStaticAudioPlayer::InstallAICallback() {
#if defined(TARGET_PC)
  sndPCSetOutputCallback(sAICallbacks.empty() ? nullptr : AICallback);
#else
  bool old = CAudioSys::IsAICallbackEnabled();
  CAudioSys::EnableAICallback(true);

  if (!sDMACallbackInstalled && sAICallbacks.size() != 0) {
    sOldDMACallback = AIRegisterDMACallback(AICallback);
    sDMACallbackInstalled = true;
  } else if (sDMACallbackInstalled && sAICallbacks.size() == 0) {
    AIRegisterDMACallback(sOldDMACallback);
    sOldDMACallback = 0;
    sDMACallbackInstalled = false;
  }

  CAudioSys::EnableAICallback(old);
#endif
}

#if defined(TARGET_PC)
void CStaticAudioPlayer::AICallback(short* output, size_t frames, u32 rate, u32 channels) {
  for (int i = 0; i < sAICallbacks.size(); ++i) {
    sAICallbacks[i](output, frames, rate, channels);
  }
}
#else
void CStaticAudioPlayer::AICallback() {
  sOldDMACallback();

  for (int i = 0; i < sAICallbacks.size(); ++i) {
    sAICallbacks[i]();
  }
}
#endif

void CStaticAudioPlayer::RunDMACallback(const FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();
  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sAICallbacks.begin(), sAICallbacks.end(), callback);
  if (it == sAICallbacks.end()) {
    sAICallbacks.push_back(callback);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

void CStaticAudioPlayer::CancelDMACallback(FAudioCallback callback) {
  volatile const bool old = OSDisableInterrupts();

  const rstl::reserved_vector< FAudioCallback, 4 >::iterator it =
      rstl::find(sAICallbacks.begin(), sAICallbacks.end(), callback);
  if (it != sAICallbacks.end()) {
    sAICallbacks.erase(it);
  }

  InstallAICallback();
  OSRestoreInterrupts(old);
}

CStaticAudioPlayer::CStaticAudioPlayer(const rstl::string& filepath, const int loopStart,
                                       const int loopEnd)
: x0_filepath(filepath)
, x10_rsfRem(-1)
, x18_curSamp(0)
, x1c_loopStartSamp(loopStart & ~1)
, x20_loopEndSamp(loopEnd & ~1)
, x24_curBuf(0)
, x28_dmaLeft((uchar*)CMemory::Alloc(640, IAllocator::kHI_RoundUpLen))
, x30_dmaRight((uchar*)CMemory::Alloc(640, IAllocator::kHI_RoundUpLen))
, xc0_volume(32768) {
  CDvdFile dvdFile(filepath.data());
  x10_rsfRem = dvdFile.GetFileSize();
  x14_rsfLength = x10_rsfRem;
  int bufferCount = ((x10_rsfRem - 1) + 0x20000) / 0x20000;
  x48_buffers.reserve(bufferCount);
  x38_dvdRequests.reserve(bufferCount);

  for (int i = x10_rsfRem; i > 0; i -= 0x20000) {
    uint uVar1 = 0x20000;
    if (i <= 0x20000) {
      uVar1 = (i + 31) & ~31;
    }

    rstl::auto_ptr< uchar > buf((uchar*)CMemory::Alloc(uVar1, IAllocator::kHI_RoundUpLen));
    x48_buffers.push_back(buf);
    x38_dvdRequests.push_back(dvdFile.SyncRead(buf.get(), uVar1));
  }
}

CStaticAudioPlayer::~CStaticAudioPlayer() { StopMixOut(); }

const bool CStaticAudioPlayer::IsReady() const {
  return !x38_dvdRequests.empty() ? x38_dvdRequests.back()->IsComplete() : true;
}

void CStaticAudioPlayer::StartMixOut() {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  if (sCurrentPlayer == this) {
    return;
  }

  x38_dvdRequests.clear();
  x18_curSamp = 0;
  g72x_init_state(&x58_leftState);
  g72x_init_state(&x8c_rightState);
#if defined(TARGET_PC)
  m_audioPhase = 0;
  m_audioPairRead = 2;
  m_audioPrimed = false;
#endif
  sCurrentPlayer = this;
  RunDMACallback(MixCallback);
}

void CStaticAudioPlayer::StopMixOut() {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  if (sCurrentPlayer == this) {
    CancelDMACallback(MixCallback);
    sCurrentPlayer = NULL;
  }
}

#if defined(TARGET_PC)
void CStaticAudioPlayer::MixCallback(short* output, size_t frames, u32 rate, u32 channels) {
  if (sCurrentPlayer != nullptr) {
    sCurrentPlayer->DoMix(output, frames, rate, channels);
  }
}

void CStaticAudioPlayer::DoMix(short* output, size_t frames, u32 rate, u32 channels) {
  // RSF stores two 32 kHz G.721 channels, with two samples packed in each byte.
  // Keep decoding pairs even when the output rate requires fractional frames.
  const auto readFrame = [this](short* frame) {
    if (m_audioPairRead == 2) {
      std::fill_n(m_audioPair, 4, short(0));
      ushort* pair = reinterpret_cast< ushort* >(m_audioPair);
      Decode(pair, pair, 2);
      m_audioPairRead = 0;
    }
    frame[0] = m_audioPair[m_audioPairRead * 2];
    frame[1] = m_audioPair[m_audioPairRead * 2 + 1];
    ++m_audioPairRead;
  };
  if (!m_audioPrimed) {
    readFrame(m_audioHistory[0]);
    readFrame(m_audioHistory[1]);
    m_audioPrimed = true;
  }

  for (size_t frame = 0; frame < frames; ++frame, output += channels) {
    for (u32 channel = 0; channel < 2; ++channel) {
      // The original DMA buffer is R,L; native output uses FL,FR.
      const s32 a = m_audioHistory[0][1 - channel];
      const s32 b = m_audioHistory[1][1 - channel];
      const s32 sample = a + (s64(b - a) * m_audioPhase) / rate;
      if (xc0_volume != 0) {
        output[channel] = std::clamp(s32(output[channel]) + sample, -32768, 32767);
      }
    }
    m_audioPhase += 32000;
    while (m_audioPhase >= rate) {
      m_audioPhase -= rate;
      m_audioHistory[0][0] = m_audioHistory[1][0];
      m_audioHistory[0][1] = m_audioHistory[1][1];
      readFrame(m_audioHistory[1]);
    }
  }
}
#else
void CStaticAudioPlayer::MixCallback() { sCurrentPlayer->DoMix(); }

void CStaticAudioPlayer::DoMix() {
  u32 aiStart = OSCachedToPhysical(AIGetDMAStartAddr());
  x24_curBuf ^= 1;
  uintptr_t buf =
      reinterpret_cast< uintptr_t >(x24_curBuf != 0 ? x30_dmaRight.get() : x28_dmaLeft.get());

  AIInitDMA(buf, 0x280);
  u32 cookie = OSEnableInterrupts();
  if (aiStart != 0) {
    DCInvalidateRange((void*)aiStart, 0x280);
  }

  Decode((ushort*)buf, (ushort*)aiStart, 160);
  DCFlushRange((void*)buf, 0x280);
  OSRestoreInterrupts(cookie);
}
#endif

void CStaticAudioPlayer::Decode(const ushort* bufIn, ushort* bufOut, int numSamples) {
  int curSamp = x18_curSamp / 2;
  int loopEndSamp = x20_loopEndSamp / 2;
  int loopStartSamp = x1c_loopStartSamp / 2;
  DecodeMonoAndMix(const_cast< ushort* >(bufIn), bufOut, numSamples, curSamp, loopEndSamp,
                   loopStartSamp, xc0_volume, x58_leftState);

  int halfLen = x14_rsfLength / 2;
  DecodeMonoAndMix(const_cast< ushort* >(bufIn + 1), bufOut + 1, numSamples, curSamp + halfLen,
                   loopEndSamp + halfLen, loopStartSamp + halfLen, xc0_volume, x8c_rightState);

  int remSamples = numSamples;
  while (remSamples != 0) {
    int remTillLoop = x20_loopEndSamp - x18_curSamp;
    int rs = remSamples;
    int consumed = rstl::min_val(rs, remTillLoop);
    x18_curSamp += consumed;
    remSamples -= consumed;
    if (x18_curSamp == x20_loopEndSamp) {
      x18_curSamp = x1c_loopStartSamp;
    }
  }
}

void CStaticAudioPlayer::DecodeMonoAndMix(ushort* bufIn, ushort* bufOut, int numSamples,
                                          int curSample, int sampleEnd, int sampleStart, int vol,
                                          g72x_state& state) {
  for (int remBytes = numSamples / 2; remBytes != 0;) {
    int rb = remBytes;
    int curBuf = curSample / 0x20000;
    int thisBytes = ((curBuf + 1) * 0x20000) - curSample;
    thisBytes = rstl::min_val(rb, thisBytes);

    int remTillLoop = sampleEnd - curSample;
    thisBytes = rstl::min_val(thisBytes, remTillLoop);

    uchar* byte = x48_buffers[curBuf].get() + (curSample - (curBuf * 0x20000));
    int i = 0;
    while (i < thisBytes) {
      int samp1 =
          reinterpret_cast< short* >(bufOut)[0] + ((vol * g721_decoder(*byte & 0xf, &state)) >> 15);
      int samp2 =
          reinterpret_cast< short* >(bufOut)[2] + ((vol * g721_decoder(*byte >> 4, &state)) >> 15);

      short clamped1;
      if (samp1 < -0x8000) {
        clamped1 = -0x8000;
      } else if (samp1 > 0x7fff) {
        clamped1 = 0x7fff;
      } else {
        clamped1 = samp1;
      }
      bufIn[0] = clamped1;

      short clamped2;
      if (samp2 < -0x8000) {
        clamped2 = -0x8000;
      } else if (samp2 > 0x7fff) {
        clamped2 = 0x7fff;
      } else {
        clamped2 = samp2;
      }
      bufIn[2] = clamped2;

      bufIn += 4;
      ++byte;
      bufOut += 4;
      ++i;
    }

    curSample += thisBytes;
    remBytes -= thisBytes;
    if (curSample == sampleEnd) {
      curSample = sampleStart;
    }
  }
}

void CStaticAudioPlayer::SetVolume(uchar vol) {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  if (static_cast< uchar >(vol) > 127) {
    vol = 127;
  }
  xc0_volume = CAudioSys::kVolumeTable[static_cast< uchar >(vol)];
}
