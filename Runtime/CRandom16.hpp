#pragma once

#include "Runtime/GCNTypes.hpp"

namespace urde {

class CRandom16 {
  s32 m_seed;
  static CRandom16* g_randomNumber;

public:
  explicit CRandom16(s32 seed = 99) : m_seed(seed) {}

  s32 Next() {
    m_seed = (m_seed * 0x41c64e6d) + 0x00003039;
    return (m_seed >> 16) & 0xffff;
  }

  s32 GetSeed() const { return m_seed; }

  void SetSeed(s32 seed) { m_seed = seed; }

  float Float() { return Next() * 0.000015259022f; }

  float Range(float min, float max) { return min + Float() * (max - min); }

  s32 Range(s32 min, s32 max) { return min + (Next() % ((max - min) + 1)); }

  static CRandom16* GetRandomNumber() { return g_randomNumber; }
  static void SetRandomNumber(CRandom16* rnd) { g_randomNumber = rnd; }
};

class CGlobalRandom {
  CRandom16& m_random;
  CGlobalRandom* m_prev;
  static CGlobalRandom* g_currentGlobalRandom;

public:
  CGlobalRandom(CRandom16& rand) : m_random(rand), m_prev(g_currentGlobalRandom) {
    g_currentGlobalRandom = this;
    CRandom16::SetRandomNumber(&m_random);
  }
  ~CGlobalRandom() {
    g_currentGlobalRandom = m_prev;
    if (m_prev)
      CRandom16::SetRandomNumber(&m_prev->m_random);
    else
      CRandom16::SetRandomNumber(nullptr);
  }
};

} // namespace urde
