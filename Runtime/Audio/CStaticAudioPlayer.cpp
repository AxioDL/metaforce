#include "Runtime/Audio/CStaticAudioPlayer.hpp"

#include "Runtime/CDvdFile.hpp"
#include "Runtime/CDvdRequest.hpp"

namespace urde {

#define RSF_BUFFER_SIZE 0x20000

CStaticAudioPlayer::CStaticAudioPlayer(boo::IAudioVoiceEngine& engine, std::string_view path, int loopStart,
                                       int loopEnd)
: x0_path(path)
, x1c_loopStartSamp(loopStart & 0xfffffffe)
, x20_loopEndSamp(loopEnd & 0xfffffffe)
, m_voiceCallback(*this)
, m_voice(engine.allocateNewStereoVoice(32000, &m_voiceCallback)) {
  // These are mixed directly into boo voice engine instead
  // x28_dmaLeft.reset(new u8[640]);
  // x30_dmaRight.reset(new u8[640]);

  CDvdFile file(path);
  x10_rsfRem = file.Length();
  x14_rsfLength = x10_rsfRem;

  u32 bufCount = (x10_rsfRem + RSF_BUFFER_SIZE - 1) / RSF_BUFFER_SIZE;
  x48_buffers.reserve(bufCount);
  x38_dvdRequests.reserve(bufCount);

  for (int remBytes = x10_rsfRem; remBytes > 0; remBytes -= RSF_BUFFER_SIZE) {
    u32 thisSz = RSF_BUFFER_SIZE;
    if (remBytes < RSF_BUFFER_SIZE)
      thisSz = ROUND_UP_32(remBytes);

    x48_buffers.emplace_back(new u8[thisSz]);
    x38_dvdRequests.push_back(file.AsyncRead(x48_buffers.back().get(), thisSz));
  }

  g72x_init_state(&x58_leftState);
  g72x_init_state(&x8c_rightState);
}

bool CStaticAudioPlayer::IsReady() {
  if (x38_dvdRequests.size())
    return x38_dvdRequests.back()->IsComplete();
  return true;
}

void CStaticAudioPlayer::DecodeMonoAndMix(s16* bufOut, u32 numSamples, u32 cur, u32 loopEndCur, u32 loopStartCur,
                                          int vol, g72x_state& state,
                                          std::optional<g72x_state>& loopState) const {
  for (u32 remBytes = numSamples / 2; remBytes;) {
    u32 curBuf = cur / RSF_BUFFER_SIZE;
    u32 thisBytes = (curBuf + 1) * RSF_BUFFER_SIZE - cur;
    thisBytes = std::min(thisBytes, remBytes);
    u32 remTillLoop = loopEndCur - cur;
    remTillLoop = std::min(remTillLoop, thisBytes);

    const std::unique_ptr<u8[]>& buf = x48_buffers[curBuf];
    const u8* byte = &buf[cur - curBuf * RSF_BUFFER_SIZE];

    for (u32 i = 0; i < remTillLoop; ++i, ++byte) {
      if (!loopState && cur + i == loopStartCur)
        loopState.emplace(state);

      *bufOut = SampClamp(((g721_decoder(*byte & 0xf, &state) * vol) >> 15));
      bufOut += 2;

      *bufOut = SampClamp(((g721_decoder(*byte >> 4 & 0xf, &state) * vol) >> 15));
      bufOut += 2;
    }

    cur += remTillLoop;
    remBytes -= remTillLoop;
    if (cur == loopEndCur) {
      cur = loopStartCur;
      if (loopState)
        state = *loopState;
    }
  }
}

void CStaticAudioPlayer::Decode(s16* bufOut, u32 numSamples) {
  DecodeMonoAndMix(bufOut, numSamples, x18_curSamp / 2, x20_loopEndSamp / 2, x1c_loopStartSamp / 2, xc0_volume,
                   x58_leftState, m_leftStateLoop);

  u32 halfway = x14_rsfLength / 2;
  DecodeMonoAndMix(bufOut + 1, numSamples, x18_curSamp / 2 + halfway, x20_loopEndSamp / 2 + halfway,
                   x1c_loopStartSamp / 2 + halfway, xc0_volume, x8c_rightState, m_rightStateLoop);

  for (u32 remSamples = numSamples; remSamples;) {
    u32 remTillLoop = x20_loopEndSamp - x18_curSamp;
    remTillLoop = std::min(remTillLoop, remSamples);

    x18_curSamp += remTillLoop;
    remSamples -= remTillLoop;

    if (x18_curSamp == x20_loopEndSamp)
      x18_curSamp = x1c_loopStartSamp;
  }
}

} // namespace urde
