#include "NESemu/emusound.h"
#include "dolphin/os.h"
#include "dolphin/vi.h"
#include "types.h"

#if VERSION != VERSION_GM8P_00
#define NES_CLOCK_SPEED 1789773
#define NES_CLOCK_RATE (NES_CLOCK_SPEED - 0.25f)
#define SOUND_SAMPLE_RATE 2.038168f
#endif

typedef struct _NESSoundStruct2 {
  u8 _00;
  union {
    u32 _04;
    struct {
      u16 _04_hi;
      u16 _04_lo;
    } _04p;
  };
  u32 _08;
  u32 _0C;
  u32 _10;
  u32 _14;
  u16 _18;
  u16 _1A;
  u8 _1C;
  u8 _1D;
  s16 _1E;
  u8 _20;
  u8 _21;
  u8 _22;
  u8 _23;
} NESSoundStruct2;

typedef struct _NESSoundStruct3 {
  u8 _00;
  union {
    u32 _04;
    struct {
      u16 _04_hi, _04_lo;
    } _04p;
  };
  u32 _08;
  u8 _0C;
  u8 _0D;
  u16 _0E;
  s8* _10;
  s32 _14;
} NESSoundStruct3;

typedef struct _NESSoundStruct {
  u8 _00;
  union {
    u32 _04;
    struct {
      u16 _04_hi;
      u16 _04_lo;
    } _04p;
  };
  u32 _08;
  u16 _0C;
  u16 _0E;
  u32 _10;
  u32 _14;
  u32 _18;
  u8 _1C;
  u8 _1D;
  u32 _20;
  u32 _24;
  u8 _28;
  u16 _2A;
  u16 _2C;
  u32 _30;
  u32 _34;
  u8 _38;
  u8 _39;
  u8 _3A;
  u8 _3B;
  u8 _3C;
  u16 _3E;
  u16 _40;
  u16 _42;
  u8 _44;
  u8 _45[2];
  u8 _47;
  u16 _48;
  u8 _4A;
} NESSoundStruct; // size 0x4C

typedef struct _NESSoundStructF {
  u8 _00;
  union {
    u32 _04;
    struct {
      u16 _04_hi;
      u16 _04_lo;
    } _04p;
  };
  union {
    u32 _08;
    struct {
      u16 _08_hi, _08_lo;
    } _08p;
  };
  s32 _0C;
  u32 _10;
  u16 _14;
  u16 _16;
  u32 _18;
  u16 _1C;
  u8 _1E;
  u8 _1F;
  u8 _20;
  u8 _21[3];
  u32 _24;
  u32 _28;
  u8 _2C;
  u8 _2D;
  u8 _2E;
  u8 _2F;
  u32 _30;
  u32 _34;
  u8 _38;
  u8 _39;
  u8 _3A;
  u8 _3B;
  u16 _3C;
  u16 _3E;
  u16 _40;
  u8 _42;
  u8 _43;
  s8 _44;
  u8 _45;
  u8 _46;
  s8 modulationAccumulator;
  u8 _48;
  u8 _49;
  u8 _4A;
} NESSoundStructF; // size 0x4C

CHECK_SIZEOF(NESSoundStruct, 0x4C)
CHECK_SIZEOF(NESSoundStruct2, 0x24)
CHECK_SIZEOF(NESSoundStruct3, 0x18)
CHECK_SIZEOF(NESSoundStructF, 0x4C)

typedef struct _PulseChannel {

  // $4000/$4004
  u8 Duty : 2;
  u8 EnvelopeLoopCounter : 1;
  u8 ConstantVolume : 1;
  u8 EnvelopePeriod_Volume : 4;
  // $4001/$4005
  u8 SweepUnitEnabled : 1;
  u8 SweepUnitPeriod : 3;
  u8 SweepUnitNegative : 1;
  u8 SweepUnitShiftCount : 3;
  // $4002/$4006
  u8 TimerLow : 8;
  // $4003/$4007
  u8 LengthCounterLow : 5;
  u8 TimerHigh : 3;
} PulseChannel;

typedef struct _TriangleChannel {

  // $4008
  u8 LengthCounter : 1;
  u8 LinearCounterReloadVaue : 7;
  // $4009
  u8 _unused_4009 : 8;
  // $400A
  u8 TimerLow : 8;
  // $400B
  u8 LengthCounterLoad : 5;
  u8 TimerHigh : 3;
} TriangleChannel;

typedef struct _NoiseChannel {

  // $400C
  u8 _unused_400C_high2 : 2;
  u8 LoopEnvelope : 1;
  u8 ConstantVolume : 1;
  u8 EnvelopePeriod_Volume : 4;
  // $400D
  u8 _unused_400D : 8;
  // $400E
  u8 LoopNoise : 1;
  u8 _unused_400E_3bits : 3;
  u8 Period : 4;
  // $400F
  u8 LengthCounterLoad : 5;
  u8 _unused_400F_low3 : 3;
} NoiseChannel;

typedef struct _DMCChannel {

  // $4010
  u8 IRQEnable : 1;
  u8 LoopSample : 1;
  u8 _unused_4010_middle2 : 2;
  u8 FrequencyIndex : 4;
  // $4011
  u8 _unused_4011_heigh : 1;
  u8 DirectLoad : 7;
  // $4012
  u8 SampleAddress : 8;
  // $4013
  u8 SampleLength : 8;
} DMCChannel;

typedef struct _WaveTableEntry {
  u8 Read : 2;
  u8 Sample : 6;
} WaveTableEntry;

typedef struct _unknown_C0 {
  u8 _0_7 : 1; // 10000000
  u8 _0_6 : 3; // 01100000
  u8 _0_3 : 4; // 00001111
  u8 _1;
  u8 _2;
  u8 _3_7 : 1; // 10000000
  u8 _3_6 : 3; // 01110000
  u8 _3_3 : 4; // 00001111
} Unknown_VRC;

typedef struct _APURegisters {
  // $4000
  PulseChannel Pulse1;
  // $4004
  PulseChannel Pulse2;
  // $4008
  TriangleChannel Triangle;
  // $400C
  NoiseChannel Noise;
  // $4010
  DMCChannel DMC;
  // $4014
  u8 _unused_4014;

  // $4015
  u8 DMCInterrupt : 1;
  u8 FrameInterrupt : 1;
  u8 _unused_4015_1bit : 1;
  u8 LengthCounterStatus : 1;
  u8 NoiseStatus : 1;
  u8 TriangleStatus : 1;
  u8 Pulse2Status : 1;
  u8 Pulse1Status : 1;
  // $4016
  u8 _unused_4016;

  // $4017
  u8 FrameCounter_5FrameSequence : 1;
  u8 FrameCounter_DisableFrameInterrupt : 1;
  u8 _unused_4017_low : 6;

  u8 _18[0x28];
  // $4040 - $4079 https://www.nesdev.org/wiki/FDS_audio#Wavetable_RAM_($4040-$407F)
  WaveTableEntry WaveTable[0x40];
  // $4080 https://www.nesdev.org/wiki/FDS_audio#Volume_envelope_($4080)
  u8 VolumeEnvelopeEnabled : 1;
  u8 VolumeEnvelopeIncreasing : 1;
  u8 VolumeEnvelopeGainAndSpeed : 6;
  // $4081
  u8 _unused_4081;
  // $4082 https://www.nesdev.org/wiki/FDS_audio#Frequency_low_($4082)
  u8 FrequencyLow;
  // $4083 https://www.nesdev.org/wiki/FDS_audio#Frequency_high_($4083)
  u8 HighSpeedFrequency : 1;
  u8 DisableSweepAndEnvelope : 1;
  u8 _unused_4083_midbits : 2;
  u8 FrequencyHigh : 4;
  // $4084 https://www.nesdev.org/wiki/FDS_audio#Mod_envelope_($4084)
  u8 ModEnvelopeEnabled : 1;
  u8 ModEnvelopeIncreasing : 1;
  u8 ModEnvelopeGainAndSpeed : 6;
  // $4085 https://www.nesdev.org/wiki/FDS_audio#Mod_counter_($4085)
  u8 _unused_4085_high : 1;
  u8 ModCounter : 7;
  // $4086 https://www.nesdev.org/wiki/FDS_audio#Mod_frequency_low_($4086)
  u8 ModFrequencyLow;
  // $4087 https://www.nesdev.org/wiki/FDS_audio#Mod_frequency_high_($4087)
  u8 HaltModTableCounter : 1;
  u8 ForceAccumulatorStepOut : 1;
  u8 _unused_4087_mid : 2;
  u8 ModFrequencyHigh : 4;
  // $4088 https://www.nesdev.org/wiki/FDS_audio#Mod_table_write_($4088)
  // unused in the following code
  u8 _unused_4088_high : 5;
  u8 ModulationTableWrite : 3;
  // $4089 https://www.nesdev.org/wiki/FDS_audio#Wave_write_/_master_volume_($4089)
  u8 WaveTableWriteEnable : 1;
  u8 _unused_4089_midbits : 5;
  u8 WaveTableMasterVolume : 2;
  // $408A https://www.nesdev.org/wiki/FDS_audio#Envelope_speed_($408A)
  u8 EnvelopeSpeed;

  u8 _8B[0x35];

  Unknown_VRC _C0;
  Unknown_VRC _C4;
  Unknown_VRC _C8;
  u8 _CC[9];
  u8 _unused_D5_high : 5;
  u8 _D5_2 : 1;
  u8 _D5_1 : 1;
  u8 _D5_0 : 1;

} APURegisters;

typedef union _sbufferStruct {
  APURegisters apu;
  u8 buff[0x4100 - 0x4000];
} sbufferStruct;

// bss
struct PcmHeader {
  s8* samples;
  u16* offsets;
  u16* lengths;
  u16* frequencies;
};
// Unreferenced state retained by the original module.
u32 lbl_1_bss_1940;
u8 beforemode;
u8 move_to_50;
u32 lbl_1_bss_1948;
PcmHeader PCMH2;
u8 MMC_MODE;
u8* NOISE_DTABLE;
NESSoundStruct SoundA, SoundB, SoundC, SoundD;
NESSoundStruct2 SoundE;
NESSoundStruct SoundX, SoundY, SoundZ;
NESSoundStruct SoundM, SoundN;
NESSoundStructF SoundF;
NESSoundStruct3 SoundP;

u8 DUMMY_ACTIVE[9];

// sbss
u32 PHASE_SYNC_FLAG;
u32 FRAME_SYNC_FLAG;
u32 PHASE_SYNC_COUNTER;
u32 FRAME_SYNC_COUNTER;
s8 NOISE_PULSE;

// data

// UNUSED
u8 LEN_TABLE[] = {0x05, 0x7e, 0x0a,  0x01, 0x13, 0x02, 0x28, 0x03, 0x50, 0x04, 0x1e,
                  0x05, 0x07, 0x06,  0x0d, 0x07, 0x06, 0x08, 0x0c, 0x09, 0x18, 0x0a,
                  0x30, 0x0b, 0x060, 0x0c, 0x24, 0x0d, 0x08, 0x0e, 0x10, 0x0f};

// UNUSED
f32 NOISE_FREQ_TABLE[] = {4.f,  2.f,   1.f,   0.5f,  0.25f, 0.2f,  0.15f, 0.12f,
                          0.1f, 0.08f, 0.07f, 0.06f, 0.05f, 0.04f, 0.03f, 0.03f};

sbufferStruct sbuffer = {0};

u8 LEN_TABLE_HVC[] = {0x05, 0x7e, 0x0a, 0x01, 0x13, 0x02, 0x28, 0x03, 0x50, 0x04, 0x1e,
                      0x05, 0x07, 0x06, 0x0d, 0x07, 0x06, 0x08, 0x0c, 0x09, 0x18, 0x0a,
                      0x30, 0x0b, 0x60, 0x0c, 0x24, 0x0d, 0x08, 0x0e, 0x10, 0x0f};

s8 TRIANGLE_TABLE[] = {0x00, 0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78, 0x78, 0x69,
                       0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f, 0x00, -0x11, -0x22, -0x33, -0x45,
                       -0x56, -0x67, -0x78, -0x78, -0x67, -0x56, -0x45, -0x33, -0x22, -0x11};

static int NOISE_TABLE[] = {0x000df86a, 0x0006fc35, 0x00037e1a, 0x0001bf0d, 0x0000df86, 0x00009504,
                            0x00006fc3, 0x00005969, 0x000046d1, 0x00003852, 0x000025a5, 0x00001c29,
                            0x000012c6, 0x00000e14, 0x00000708, 0x000006fd};

u8 NOISE_PULSE_TABLE[] = {0x30, 0x32, 0x34, 0x38, 0x3c, 0x3d, 0x3e, 0x3f,
                          0x40, 0x42, 0x44, 0x46, 0x48, 0x50, 0x60, 0x60};

u32 DMOD_TABLE[] = {0x000010bd, 0x000012db, 0x00001513, 0x00001663, 0x0000190d, 0x00001c36,
                    0x00001fb4, 0x00002115, 0x000025b6, 0x00002cbf, 0x000032a4, 0x000037fb,
                    0x00004399, 0x00005475, 0x00006385, 0x000084b1};

// UNUSED
u8 VOLTABLE_DELTAMOD[] = {
    0x00, 0x00, 0x00, 0xff, 0x01, 0xfc, 0x02, 0xf7, 0x03, 0xf0, 0x04, 0xe7, 0x05, 0xdc, 0x06, 0xcf,
    0x07, 0xc0, 0x08, 0xaf, 0x09, 0x9c, 0x0a, 0x87, 0x0b, 0x70, 0x0c, 0x57, 0x0d, 0x3c, 0x0e, 0x1f,
    0x0f, 0x00, 0x0f, 0xdf, 0x10, 0xbc, 0x11, 0x97, 0x12, 0x70, 0x13, 0x47, 0x14, 0x1c, 0x14, 0xef,
    0x15, 0xc0, 0x16, 0x8f, 0x17, 0x5c, 0x18, 0x27, 0x18, 0xf0, 0x19, 0xb7, 0x1a, 0x7c, 0x1b, 0x3f,
    0x1c, 0x00, 0x1c, 0xbf, 0x1d, 0x7c, 0x1e, 0x37, 0x1e, 0xf0, 0x1f, 0xa7, 0x20, 0x5c, 0x21, 0x0f,
    0x21, 0xc0, 0x22, 0x6f, 0x23, 0x1c, 0x23, 0xc7, 0x24, 0x70, 0x25, 0x17, 0x25, 0xbc, 0x26, 0x5f,
    0x27, 0x00, 0x27, 0x9f, 0x28, 0x3c, 0x28, 0xd7, 0x29, 0x70, 0x2a, 0x07, 0x2a, 0x9c, 0x2b, 0x2f,
    0x2b, 0xc0, 0x2c, 0x4f, 0x2c, 0xdc, 0x2d, 0x67, 0x2d, 0xf0, 0x2e, 0x77, 0x2e, 0xfc, 0x2f, 0x7f,
    0x30, 0x00, 0x30, 0x7f, 0x30, 0xfc, 0x31, 0x77, 0x31, 0xf0, 0x32, 0x67, 0x32, 0xdc, 0x33, 0x4f,
    0x33, 0xc0, 0x34, 0x2f, 0x34, 0x9c, 0x35, 0x07, 0x35, 0x70, 0x35, 0xd7, 0x36, 0x3c, 0x36, 0x9f,
    0x37, 0x00, 0x37, 0x5f, 0x37, 0xbc, 0x38, 0x17, 0x38, 0x70, 0x38, 0xc7, 0x39, 0x1c, 0x39, 0x6f,
    0x39, 0xc0, 0x3a, 0x0f, 0x3a, 0x5c, 0x3a, 0xa7, 0x3a, 0xf0, 0x3b, 0x37, 0x3b, 0x7c, 0x3b, 0xbf,
    0x3c, 0x00, 0x3c, 0x3f, 0x3c, 0x7c, 0x3c, 0xb7, 0x3c, 0xf0, 0x3d, 0x27, 0x3d, 0x5c, 0x3d, 0x8f,
    0x3d, 0xc0, 0x3d, 0xef, 0x3e, 0x1c, 0x3e, 0x47, 0x3e, 0x70, 0x3e, 0x97, 0x3e, 0xbc, 0x3e, 0xdf,
    0x3f, 0x00, 0x3f, 0x1f, 0x3f, 0x3c, 0x3f, 0x57, 0x3f, 0x70, 0x3f, 0x87, 0x3f, 0x9c, 0x3f, 0xaf,
    0x3f, 0xc0, 0x3f, 0xcf, 0x3f, 0xdc, 0x3f, 0xe7, 0x3f, 0xf0, 0x3f, 0xf7, 0x3f, 0xfc, 0x3f, 0xff};

// UNUSED
u8 VOLTABLE_DISKFM[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10,
                        0x12, 0x14, 0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x22, 0x24, 0x26,
                        0x28, 0x2a, 0x2c, 0x2e, 0x30, 0x32, 0x34, 0x36, 0x38, 0x3a, 0x3c};

// UNUSED
u8 VOLTABLE_DISKFM_SUB[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                            0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                            0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};

// UNUSED
s8 DISK_SUB_GAIN[] = {0, 1, 2, 4, 0, -4, -2, -1};
u32 DISKFM_GAINTABLE[] = {30, 20, 15, 12};

// UNUSED

u8 DEB1[16] = {0};
// sdata
u8* ROM_TOP_C000 = DEB1;
u8* ROM_TOP_E000 = DEB1;
u32 DISK_FRAME_SAMPLE = 0x215;
u32 FRAME_SAMPLE = 0x215;
u32 PHASE_SAMPLE = 0x85;

#if VERSION == VERSION_GM8P_00
u32 SOUND_SAMPLE_RATE = 2;
f32 NES_CLOCK_RATE = 1789882.75f;
u32 NES_FRAME_RATE = 60;
#endif

f32 VOLTABLE_HVCPULSE[] = {0.0f,      0.080128,  0.157051f, 0.224359f, 0.294872f, 0.365385f,
                           0.435897f, 0.50641f,  0.592949f, 0.657051f, 0.730769f, 0.801282f,
                           0.865385f, 0.929487f, 0.99359f,  1.064103f, 1.064103f};

u8 VOLTABLE_VRCPULSE[] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0a, 0x0c, 0x0e, 0x10, 0x12, 0x14,
                          0x16, 0x18, 0x1a, 0x1c, 0x1e, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
                          0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f};

static void Jac_bzero(void* dest, s32 size) {
  u8* bytes = static_cast< u8* >(dest);
  for (s32 i = 0; i < size; ++i) {
    *bytes++ = 0;
  }
}

s32 __GetWave_Pulse(s32 a, s32 b) {
  switch (a) {

  case 0: {
    if (b >= 4 && b <= 7) {
      return 1;
    } else {
      return 0;
    }
  }
  case 1: {
    return b <= 7;
  }
  case 2: {
    return b <= 15;
  }
  case 3: {
    return b > 7;
  }
  case 0xb: {
    return b & 0xff;
  }
  case 0xc: {
    return b <= 0 ? 127 : 0;
  }
  case 0xd: {
    return b <= 1 ? 127 : 0;
  }
  case 0xe: {
    return b <= 2 ? 127 : 0;
  }
  case 0xf: {
    return b <= 3 ? 127 : 0;
  }
  case 0x10: {
    return b <= 4 ? 127 : 0;
  }
  case 0x11: {
    return b <= 5 ? 0 : 127;
  }
  case 0x12: {
    return b <= 6 ? 0 : 127;
  }
  case 0x13: {
    return b <= 7 ? 0 : 127;
  }
  }
  return a;
}

u32 __PitchTo32_VRC_C(u16 pitch) {
  f32 scalefactor = 2.f;
  return static_cast< u32 >(NES_CLOCK_RATE / scalefactor / (pitch + 1) / 1000.875f * 32768.f);
}

u32 __PitchTo32_VRC_PULSE(u16 pitch) {
  f32 scalefactor = 16.f;
  return static_cast< u32 >(NES_CLOCK_RATE / scalefactor / (pitch + 1) / 1000.875f * 32768.f);
}

u32 __PitchTo32_HVC(u16 pitch) {
  f32 scalefactor = 8.f;
  return static_cast< u32 >(NES_CLOCK_RATE / scalefactor / (pitch + 1) / 1000.875f * 32768.f);
}

void __Sound_Write_VRC(u16 a, u8 b) {
  NESSoundStruct* pSound;
  Unknown_VRC* pBuffer;
  int v;
  int xyzIndex;

  u16 index = a - 0xc0;
  if (index <= 3) {
    pSound = &SoundX;
    xyzIndex = 0;
    pBuffer = &sbuffer.apu._C0;
  } else if (index <= 7) {
    pSound = &SoundY;
    xyzIndex = 1;
    pBuffer = &sbuffer.apu._C4;
  } else {
    pSound = &SoundZ;
    xyzIndex = 2;
    pBuffer = &sbuffer.apu._C8;
  }

  if (index == 21) {
    if (!(b & 1)) {
      SoundX._00 = 0;
    }
    if (!(b & 2)) {
      SoundY._00 = 0;
    }
    if (!(b & 4)) {
      SoundZ._00 = 0;
    }
    return;
  }

  switch (index & 3) {
  case 0: {
    pSound->_3C = pBuffer->_0_6;
    pSound->_28 = pBuffer->_0_7;
    pSound->_1C = VOLTABLE_VRCPULSE[pBuffer->_0_3];
    pSound->_1D = pBuffer->_0_3;
    pSound->_2A = pSound->_1D;
    if (xyzIndex == 2) {
      pSound->_24 = ((pSound->_3C & 3) << 4) | pSound->_2A;
      pSound->_1C = (pSound->_24 * 7) / 3;
      if (pSound->_1C >= 0x7f) {
        pSound->_1C = 0x7f;
      }
    }
  } break;
  case 2: {
    pSound->_0C = (pBuffer->_3_3 << 8) | pBuffer->_2;
    if (xyzIndex == 2) {
      pSound->_08 = __PitchTo32_VRC_C(pSound->_0C);
    } else {
      pSound->_08 = __PitchTo32_VRC_PULSE(pSound->_0C);
    }
  } break;
  case 3: {
    pSound->_0C = (pBuffer->_3_3 << 8) | pBuffer->_2;
    if (xyzIndex == 2) {
      pSound->_08 = __PitchTo32_VRC_C(pSound->_0C);
    } else {
      pSound->_08 = __PitchTo32_VRC_PULSE(pSound->_0C);
    }

    if (!pSound->_00 && !pSound->_3E) {
      pSound->_04 = 0;
    }

    if (xyzIndex == 0) {
      v = sbuffer.apu._D5_0;
    }
    if (xyzIndex == 1) {
      v = sbuffer.apu._D5_1;
    }
    if (xyzIndex == 2) {
      v = sbuffer.apu._D5_2;
    }

    if (static_cast< u8 >(v) == 1 && pBuffer->_3_7) {
      pSound->_00 = 1;
    } else {
      pSound->_00 = 0;
    }

    pSound->_2A = pSound->_1D;
    pSound->_20 = 0;
  } break;
  }
}

void __Sound_Write_MMC5(u16 a, u8 b) {
  NESSoundStruct* pSound;
  PulseChannel* pBuffer;
  u32 v;
  int xyzIndex;

  u16 index = a - 0xc0;
  pBuffer = 0;
  if (index <= 3) {
    pSound = &SoundX;
    xyzIndex = 0;
    pBuffer = reinterpret_cast< PulseChannel* >(&sbuffer.apu._C0);
  } else if (index <= 7) {
    pSound = &SoundY;
    xyzIndex = 1;
    pBuffer = reinterpret_cast< PulseChannel* >(&sbuffer.apu._C4);
  }

  if (index == 17) {
    SoundZ._3E = (b & 0xff) << 6;
  } else if (index == 21) {
    if (!(b & 1)) {
      SoundX._00 = 0;
    }
    if (!(b & 2)) {
      SoundY._00 = 0;
    }
  } else if (pBuffer) {
    switch (index & 3) {
    case 0: {
      pSound->_3C = pBuffer->Duty;
      v = pSound->_28;
      pSound->_28 = pBuffer->ConstantVolume ^ 1;
      pSound->_14 = pBuffer->EnvelopeLoopCounter;
      pSound->_1D = VOLTABLE_HVCPULSE[pBuffer->EnvelopePeriod_Volume];
      if (pSound->_28 == 0 && pSound->_00 == 1) {
        pSound->_1C = VOLTABLE_HVCPULSE[pBuffer->EnvelopePeriod_Volume];
      } else {
        pSound->_24 = PHASE_SAMPLE * (pBuffer->EnvelopePeriod_Volume + 1);
        if (v == 0) {
          pSound->_2A = 0xf;
          pSound->_20 = 0;
        }
      }
    } break;
    case 2: {
      pSound->_0C = pBuffer->TimerHigh << 8 | pBuffer->TimerLow;
      pSound->_08 = __PitchTo32_HVC(pSound->_0C);
    } break;
    case 3: {
      pSound->_0C = pBuffer->TimerHigh << 8 | pBuffer->TimerLow;
      pSound->_08 = __PitchTo32_HVC(pSound->_0C);
      pSound->_18 = LEN_TABLE_HVC[pBuffer->LengthCounterLow];
      pSound->_10 = FRAME_SAMPLE * pSound->_18;
      if (!pSound->_00 && !pSound->_3E) {
        pSound->_04 = 0;
      }

      if (xyzIndex == 0) {
        v = sbuffer.apu._D5_0;
      }
      if (xyzIndex == 1) {
        v = sbuffer.apu._D5_1;
      }

      if ((v & 0xff) != 0) {
        pSound->_00 = 1;
      }

      if (pBuffer->ConstantVolume == 0) {
        pSound->_1C = VOLTABLE_HVCPULSE[15];
      } else {
        pSound->_1C = pSound->_1D;
      }

      pSound->_2A = 0xf;
      pSound->_20 = 0;
      pSound->_30 = 0;
      pSound->_3A = 0;
    } break;
    }
  }
}

u16 __ProcessSoundMMC5(NESSoundStruct* sound) {
  u16 volume = sound->_1C;
  if (!sound->_00) {
    if (!sound->_3E) {
      return 0;
    }
    sound->_3E--;
    volume = sound->_3E;
  }
  if (sound->_0C <= 7) {
    sound->_44 = 1;
  } else if (sound->_0C >= 0x800 && !sound->_39) {
    sound->_44 = 1;
  } else {
    sound->_44 = 0;
  }

  u16 sample;
  if (sound->_44 == 1) {
    sample = 0;
  } else {
    sample = static_cast< u8 >(__GetWave_Pulse(sound->_3C, sound->_04p._04_hi));
    sample *= volume;
    sound->_04 += sound->_08;
    sound->_04 &= 0x1FFFFF;
    sound->_3E = volume;
    sound->_40 = sound->_08;
  }
  if (sound->_28) {
    sound->_20++;
    if (sound->_20 % sound->_24 == 0) {
      switch (sound->_14) {
      case 0:
        if (sound->_2A) {
          sound->_2A--;
        } else {
          sound->_00 = 0;
        }
        break;
      case 1:
        if (sound->_2A) {
          sound->_2A--;
        } else {
          sound->_2A = 15;
        }
        break;
      }
      sound->_1C = VOLTABLE_HVCPULSE[sound->_2A];
    }
  }
  if (!sound->_14) {
    if (sound->_10) {
      sound->_10--;
    }
    if (!sound->_10) {
      sound->_00 = 0;
    }
  }
  return sample;
}

u16 __ProcessSoundVRC(int channel) {
  NESSoundStruct* sound;
  switch (channel) {
  case 0:
    sound = &SoundX;
    break;
  case 1:
    sound = &SoundY;
    break;
  case 2:
    sound = &SoundZ;
    break;
  }
  u16 volume = sound->_1C;
  if (!sound->_00) {
    if (!sound->_3E) {
      return 0;
    }
    sound->_3E--;
    volume = sound->_3E;
    sound->_08 = sound->_40;
  }
  if (channel <= 1 && sound->_28) {
    return static_cast< u16 >(TRIANGLE_TABLE[sound->_2A] << 5);
  }
  u16 wave;
  if (channel == 2) {
    wave = static_cast< u8 >(__GetWave_Pulse(11, sound->_04p._04_hi));
  } else {
    wave = static_cast< u8 >(__GetWave_Pulse(sound->_3C + 12, sound->_04p._04_hi));
  }
  sound->_04 += sound->_08;
  if (channel == 2) {
    sound->_04p._04_hi %= 112;
  } else {
    sound->_04 &= 0xFFFFF;
  }
  sound->_3E = volume;
  sound->_40 = sound->_08;
  return static_cast< u16 >(wave * volume);
}

void __Sound_Write_A(u16 a, u8 b) {
  switch (a) {
  case 0: {
    SoundA._3C = sbuffer.apu.Pulse1.Duty;
    u32 v = SoundA._28;
    SoundA._28 = sbuffer.apu.Pulse1.ConstantVolume ^ 1;
    SoundA._14 = sbuffer.apu.Pulse1.EnvelopeLoopCounter;
    if (!SoundA._28) {
      SoundA._2C = sbuffer.apu.Pulse1.EnvelopePeriod_Volume << 8;
    } else {
      SoundA._24 = sbuffer.apu.Pulse1.EnvelopePeriod_Volume + 1;
      if (!v && SoundA._00) {
        SoundA._2C = 0xf00;
        SoundA._20 = 0;
      }
    }
  } break;
  case 1: {
    SoundA._34 = (sbuffer.apu.Pulse1.SweepUnitPeriod + 1) << 1;
    SoundA._39 = sbuffer.apu.Pulse1.SweepUnitNegative;
    SoundA._38 = sbuffer.apu.Pulse1.SweepUnitShiftCount;
    SoundA._30 = 0;
    SoundA._3B = 0;
    SoundA._3A = sbuffer.apu.Pulse1.SweepUnitEnabled;
  } break;
  case 2: {
    SoundA._0C = SoundA._0C & 0xf00 | sbuffer.apu.Pulse1.TimerLow;
    SoundA._08 = __PitchTo32_HVC(SoundA._0C);
  } break;
  case 3: {
    SoundA._0C = (sbuffer.apu.Pulse1.TimerHigh << 8) | sbuffer.apu.Pulse1.TimerLow;
    SoundA._08 = __PitchTo32_HVC(SoundA._0C);
    SoundA._18 = LEN_TABLE_HVC[sbuffer.apu.Pulse1.LengthCounterLow];
    SoundA._10 = SoundA._18 << 2;
    SoundA._04 = 0;
    SoundA._48 = 0;
    SoundA._3E = 0;
    SoundA._2A = 0;
    if (sbuffer.apu.Pulse1Status) {
      SoundA._00 = 1;
    }
    if (!sbuffer.apu.Pulse1.ConstantVolume) {
      SoundA._2C = 0xf00;
    } else {
      SoundA._2C = sbuffer.apu.Pulse1.EnvelopePeriod_Volume << 8;
    }
    SoundA._20 = 0;
    SoundA._30 = 0;
    SoundA._3B = 0;
    SoundA._3A = sbuffer.apu.Pulse1.SweepUnitEnabled;
  } break;
  }
}

int __ProcessSoundA() {
  if (SoundA._3A) {
    if (PHASE_SYNC_FLAG) {
      SoundA._30++;
      if (SoundA._30 % SoundA._34 == 0) {
        SoundA._3B |= 1;
      }
    }

    if (SoundA._3B == 3) {
      SoundA._3B = 0;
      u32 c = SoundA._0C;
      u16 j = SoundA._0C >> SoundA._38;
      if (!SoundA._38) {
        j = 0;
      }
      switch (SoundA._39) {
      case 0: {
        SoundA._0C = SoundA._0C + j;
      } break;
      case 1: {
        if (!SoundA._38) {
          j = 0;
        }
        j = SoundA._0C - j;
        SoundA._0C = j;
        if (j && SoundA._38) {
          SoundA._0C--;
        }
      } break;
      }
      if (SoundA._0C >= 0x800) {
        SoundA._0C = 0x800;
      } else {
        SoundA._08 = __PitchTo32_HVC(SoundA._0C);
      }
    }
  }

  if (!SoundA._00 && !SoundA._3E) {
    return 0;
  }

  if (SoundA._28 && PHASE_SYNC_FLAG) {
    SoundA._20++;
    if ((SoundA._20 % SoundA._24) == 0) {
      switch (SoundA._14) {
      case 0: {
        if (SoundA._2C) {
          SoundA._2C -= 0x100;
        } else {
          SoundA._00 = 0;
        }
      } break;
      case 1: {
        if (SoundA._2C) {
          SoundA._2C -= 0x100;
        } else {
          SoundA._2C = 0xf00;
        }
      } break;
      }
    }
  }

  if (SoundA._14 == 0) {
    if (PHASE_SYNC_FLAG && SoundA._10) {
      SoundA._10--;
    }
    if (SoundA._10 == 0) {
      SoundA._00 = 0;
    }
  }

  if (SoundA._0C <= 7) {
    SoundA._44 = 1;
  } else if (SoundA._0C >= 0x800) {
    SoundA._44 = 1;
  } else {
    SoundA._44 = 0;
  }

  s8 c = __GetWave_Pulse(SoundA._3C, SoundA._04p._04_hi);

  if (SoundA._44) {
    SoundA._04 += SoundA._40;
  } else {
    SoundA._04 += SoundA._08;
    SoundA._40 = SoundA._08;
  }

  if (SoundA._04 & 0xffe00000) {
    SoundA._3B |= 2;
  }

  SoundA._04 &= 0x1fffff;
  if (SoundA._44 == 1 || !SoundA._00) {
    if (SoundA._3E) {
      SoundA._3E--;
    }
  } else {
    if (SoundA._3E < 0x20) {
      SoundA._3E++;
    }
    if (SoundA._3E < 0x20) {
      SoundA._3E++;
    }
  }

  u32 _2a_6 = SoundA._2A >> 8 & 0xff;
  f32 f_2a = (SoundA._2A & 0xff) / 256.f;
  f32 param0 = VOLTABLE_HVCPULSE[_2a_6];
  f32 param1 = VOLTABLE_HVCPULSE[_2a_6 + 1];
  f32 f_sum = param0 * (1.f - f_2a) + param1 * f_2a;

  if (SoundA._2A != SoundA._2C) {
    if (SoundA._2A > SoundA._2C) {
      u16 diff = (SoundA._2A - SoundA._2C) >> 4;
      if (!diff) {
        SoundA._2A = SoundA._2C;
      } else {
        SoundA._2A -= diff;
      }
    } else if (SoundA._2A < SoundA._2C) {
      u16 diff = (SoundA._2C - SoundA._2A) >> 4;
      if (!diff) {
        SoundA._2A = SoundA._2C;
      } else {
        SoundA._2A += diff;
      }
    }
  }

  switch (c) {
  case 0: {
    // almost 1. / 15.6
    f_sum = -0.064103;
  } break;
  case 1: {
    f_sum = -0.064103 + f_sum;
  } break;
  }

  s16 t = f_sum * 4096.f;

  t = (t * SoundA._3E) >> 5;
  return t;
}

void __Sound_Write_B(u16 a, u8 b) {
  a -= 4;
  switch (a) {
  case 0: {
    SoundB._3C = sbuffer.apu.Pulse2.Duty;
    u32 v = SoundB._28;
    SoundB._28 = sbuffer.apu.Pulse2.ConstantVolume ^ 1;
    SoundB._14 = sbuffer.apu.Pulse2.EnvelopeLoopCounter;
    if (!SoundB._28) {
      SoundB._2C = sbuffer.apu.Pulse2.EnvelopePeriod_Volume << 8;
    } else {
      SoundB._24 = sbuffer.apu.Pulse2.EnvelopePeriod_Volume + 1;
      if (!v && SoundB._00) {
        SoundB._2C = 0xf00;
        SoundB._20 = 0;
      }
    }
  } break;
  case 1: {
    SoundB._34 = (sbuffer.apu.Pulse2.SweepUnitPeriod + 1) << 1;
    SoundB._39 = sbuffer.apu.Pulse2.SweepUnitNegative;
    SoundB._38 = sbuffer.apu.Pulse2.SweepUnitShiftCount;
    SoundB._30 = 0;
    SoundB._3B = 0;
    SoundB._3A = sbuffer.apu.Pulse2.SweepUnitEnabled;
  } break;
  case 2: {
    SoundB._0C = SoundB._0C & 0xf00 | sbuffer.apu.Pulse2.TimerLow;
    SoundB._08 = __PitchTo32_HVC(SoundB._0C);
  } break;
  case 3: {
    SoundB._0C = (sbuffer.apu.Pulse2.TimerHigh << 8) | sbuffer.apu.Pulse2.TimerLow;
    SoundB._08 = __PitchTo32_HVC(SoundB._0C);
    SoundB._18 = LEN_TABLE_HVC[sbuffer.apu.Pulse2.LengthCounterLow];
    SoundB._10 = SoundB._18 << 2;
    SoundB._04 = 0;
    SoundB._48 = 0;
    SoundB._3E = 0;
    SoundB._2A = 0;
    if (sbuffer.apu.Pulse2Status) {
      SoundB._00 = 1;
    }
    if (!sbuffer.apu.Pulse2.ConstantVolume) {
      SoundB._2C = 0xf00;
    } else {
      SoundB._2C = sbuffer.apu.Pulse2.EnvelopePeriod_Volume << 8;
    }
    SoundB._20 = 0;
    SoundB._30 = 0;
    SoundB._3B = 0;
    SoundB._3A = sbuffer.apu.Pulse2.SweepUnitEnabled;
  } break;
  }
}

int __ProcessSoundB() {
  if (SoundB._3A) {
    if (PHASE_SYNC_FLAG) {
      SoundB._30++;
      if (SoundB._30 % SoundB._34 == 0) {
        SoundB._3B |= 1;
      }
    }

    if (SoundB._3B == 3) {
      SoundB._3B = 0;
      u32 c = SoundB._0C;
      u16 j = SoundB._0C >> SoundB._38;
      if (!SoundB._38) {
        j = 0;
      }
      switch (SoundB._39) {
      case 0: {
        SoundB._0C = SoundB._0C + j;
      } break;
      case 1: {
        if (!SoundB._38) {
          j = 0;
        }

        SoundB._0C = SoundB._0C - j;
      } break;
      }
      if (SoundB._0C >= 0x800) {
        SoundB._0C = 0x800;
      } else {
        SoundB._08 = __PitchTo32_HVC(SoundB._0C);
      }
    }
  }

  if (!SoundB._00 && !SoundB._3E) {
    return 0;
  }

  if (SoundB._28 && PHASE_SYNC_FLAG) {
    SoundB._20++;
    if ((SoundB._20 % SoundB._24) == 0) {
      switch (SoundB._14) {
      case 0: {
        if (SoundB._2C) {
          SoundB._2C -= 0x100;
        } else {
          SoundB._00 = 0;
        }
      } break;
      case 1: {
        if (SoundB._2C) {
          SoundB._2C -= 0x100;
        } else {
          SoundB._2C = 0xf00;
        }
      } break;
      }
    }
  }

  if (SoundB._14 == 0) {
    if (PHASE_SYNC_FLAG && SoundB._10) {
      SoundB._10--;
    }
    if (SoundB._10 == 0) {
      SoundB._00 = 0;
    }
  }

  if (SoundB._0C <= 7) {
    SoundB._44 = 1;
  } else if (SoundB._0C >= 0x800) {
    SoundB._44 = 1;
  } else {
    SoundB._44 = 0;
  }

  s8 c = __GetWave_Pulse(SoundB._3C, SoundB._04p._04_hi);

  if (SoundB._44) {
    SoundB._04 += SoundB._40;
  } else {
    SoundB._04 += SoundB._08;
    SoundB._40 = SoundB._08;
  }

  if (SoundB._04 & 0xffe00000) {
    SoundB._3B |= 2;
  }

  SoundB._04 &= 0x1fffff;
  if (SoundB._44 == 1 || !SoundB._00) {
    if (SoundB._3E) {
      SoundB._3E--;
    }
  } else {
    if (SoundB._3E < 0x20) {
      SoundB._3E++;
    }
    if (SoundB._3E < 0x20) {
      SoundB._3E++;
    }
  }

  u32 _2a_6 = SoundB._2A >> 8 & 0xff;
  f32 f_2a = (SoundB._2A & 0xff) / 256.f;
  f32 param0 = VOLTABLE_HVCPULSE[_2a_6];
  f32 param1 = VOLTABLE_HVCPULSE[_2a_6 + 1];
  f32 f_sum = param0 * (1.f - f_2a) + param1 * f_2a;

  if (SoundB._2A != SoundB._2C) {
    if (SoundB._2A > SoundB._2C) {
      u16 diff = (SoundB._2A - SoundB._2C) >> 4;
      if (!diff) {
        SoundB._2A = SoundB._2C;
      } else {
        SoundB._2A -= diff;
      }
    } else if (SoundB._2A < SoundB._2C) {
      u16 diff = (SoundB._2C - SoundB._2A) >> 4;
      if (!diff) {
        SoundB._2A = SoundB._2C;
      } else {
        SoundB._2A += diff;
      }
    }
  }

  switch (c) {
  case 0: {
    // almost 1. / 15.6
    f_sum = -0.064103;
  } break;
  case 1: {
    f_sum = -0.064103 + f_sum;
  } break;
  }

  s16 t = f_sum * 4096.f;

  t = (t * SoundB._3E) >> 5;
  return t;
}

u32 NOISE_MASTER = 1;
u32 NOISE_SHIFT = 0x800;

int __PitchTo32_HVC_D(u16 a) { return NOISE_TABLE[a]; }

void __Sound_Write_D(u16 a, u8 b) {
  switch (a) {
  case 0xc: {
    u32 v = SoundD._28;
    SoundD._28 = sbuffer.apu.Noise.ConstantVolume ^ 1;
    SoundD._14 = sbuffer.apu.Noise.LoopEnvelope;
    SoundD._1D =
        static_cast< u8 >(16.f * VOLTABLE_HVCPULSE[sbuffer.apu.Noise.EnvelopePeriod_Volume]);
    if (!SoundD._28) {
      SoundD._1C = SoundD._1D;
    } else {
      SoundD._24 = sbuffer.apu.Noise.EnvelopePeriod_Volume + 1;
      if (!v) {
        SoundD._2A = 0xf;
        SoundD._20 = 0;
      }
    }
  } break;
  case 0xe: {
    SoundD._0C = sbuffer.apu.Noise.Period;
    if (sbuffer.apu.Noise.LoopNoise == 1u) {
      SoundD._0C += 0x10;
      NOISE_SHIFT = 0x40;
      NOISE_MASTER = 1;
    } else {

      NOISE_SHIFT = 0x4000;
    }
    NOISE_PULSE = NOISE_PULSE_TABLE[SoundD._0C & 0xf];
    SoundD._08 = __PitchTo32_HVC_D(SoundD._0C & 0xf);
  } break;
  case 0xf: {
    SoundD._0C = sbuffer.apu.Noise.Period;
    if (sbuffer.apu.Noise.LoopNoise == 1u) {
      SoundD._0C += 0x10;
      NOISE_SHIFT = 0x40;
      NOISE_MASTER = 1;
    } else {
      NOISE_SHIFT = 0x4000;
    }
    NOISE_PULSE = NOISE_PULSE_TABLE[SoundD._0C % 16u];
    SoundD._08 = __PitchTo32_HVC_D(SoundD._0C % 16u);
    SoundD._18 = LEN_TABLE_HVC[sbuffer.apu.Noise.LengthCounterLoad];
    SoundD._10 = SoundD._18 * 4;
    if (sbuffer.apu.NoiseStatus != 0) {
      SoundD._00 = '\x01';
    }
    if (sbuffer.apu.Noise.ConstantVolume == 0) {
      SoundD._1C = static_cast< u8 >(16.f * VOLTABLE_HVCPULSE[15]);
    } else {
      SoundD._1C = SoundD._1D;
    }
    SoundD._2A = 0xf;
    SoundD._20 = 0;
  } break;
  }
}

s8 __GetNoise(u32 a) {
  static u8 data = 0;

  for (int i = 0; i < a >> 16; i++) {
    data = NOISE_MASTER & 3;
    switch (data) {
    case 0:
    case 3: {
      NOISE_MASTER = (NOISE_MASTER >> 1) | NOISE_SHIFT;
    } break;
    case 1:
    case 2: {
      NOISE_MASTER >>= 1;
    } break;
    }
  }

  if (NOISE_MASTER & 1) {
    return static_cast< s32 >(NOISE_PULSE);
  } else {
    return -static_cast< s32 >(NOISE_PULSE);
  }
}

u32 delta_counter;
s8 delta_sign;
s32 bias_move;
s32 bias;
f32 voltage;
f32 voltage_out;

int __ProcessSoundD() {
  s16 ret;
  if (!SoundD._00) {
    if (!SoundD._3E) {
      return 0;
    }
    SoundD._3E--;
    SoundD._1C = SoundD._3E;
  }
  SoundD._04 += SoundD._08;
  ret = __GetNoise(SoundD._04);
  ret *= SoundD._1C;
  SoundD._04p._04_hi = 0;
  SoundD._3E = SoundD._1C;

  if (SoundD._28 && PHASE_SYNC_FLAG) {
    SoundD._20++;
    if (SoundD._20 % SoundD._24 == 0) {
      switch (SoundD._14) {
      case 0: {
        if (SoundD._2A) {
          SoundD._2A--;
        } else {
          SoundD._00 = 0;
        }
      } break;
      case 1: {
        if (SoundD._2A) {
          SoundD._2A--;
        } else {
          SoundD._2A = 0xf;
        }
      } break;
      }
      SoundD._1C = static_cast< u8 >(16.f * VOLTABLE_HVCPULSE[SoundD._2A]);
    }
  }

  if (!SoundD._14) {
    if (PHASE_SYNC_FLAG && SoundD._10) {
      SoundD._10--;
    }
    if (!SoundD._10) {
      SoundD._00 = 0;
    }
  }
  return ret;
}

void MoveDeltaCounter(u8 a) {
  if (a) {
    delta_counter++;
    delta_sign++;
  } else {
    delta_counter--;
    delta_sign--;
  }
}

void MoveBias() {
  s32 oldBias = bias;
  bias += delta_sign * 2;
  if (bias > 0x7f) {
    bias = 0x7f;
  }
  if (bias < 0) {
    bias = 0;
  }
  bias_move += bias - oldBias;
  delta_sign = 0;
}

void WriteBias(u16 a) {
  bias_move = static_cast< s16 >(a) - static_cast< s16 >(bias);
  bias = (a & 0xffff);
}

int MoveVoltage() {
  voltage = 0.9994586f * voltage;
  voltage += bias_move;
  bias_move = 0;
  if (voltage > 127.f) {
    voltage = 127.f;
  }
  if (voltage < -127.f) {
    voltage = -127.f;
  }
  voltage_out += (voltage - voltage_out) / 4.f;

  return static_cast< int >(216.f * voltage_out);
}

u32 __PitchTo32_HVC_E(u16 a) { return DMOD_TABLE[a]; }

void __Sound_Write_E(u16 a, u8 b) {
  switch (a) {
  case 0x10: {
    SoundE._08 = __PitchTo32_HVC_E(sbuffer.apu.DMC.FrequencyIndex);
  } break;
  case 0x11: {
    SoundE._1E = sbuffer.apu.DMC.DirectLoad;
    WriteBias(SoundE._1E);
  } break;
  case 0x12: {
    SoundE._18 = sbuffer.apu.DMC.SampleAddress << 6;
  } break;
  case 0x13: {
    SoundE._10 = sbuffer.apu.DMC.SampleLength << 4;
  } break;
  }
}

void StartE() {
  SoundE._00 = 1;
  SoundE._14 = (SoundE._10 + 1) * 8;
  SoundE._04 = 0;
  SoundE._1A = 0;
  SoundE._20 = 0;
  delta_counter = 0;
  bias_move = 0;
}

int __ProcessSoundE() {
  if (SoundE._00) {
    for (int i = 0; i < 2u; i++) {
      SoundE._04 += SoundE._08;
      if (SoundE._04p._04_hi != SoundE._1A) {
        u32 vv = SoundE._1A & 7;
        u32 offset = (SoundE._1A >> 3 & 0x1fff);
        if (!vv) {
          if (SoundE._18 + offset < 0x2000) {
            SoundE._1C = ROM_TOP_C000[SoundE._18 + offset];
          } else {
            SoundE._1C = ROM_TOP_E000[SoundE._18 + offset - 0x2000];
          }
        }
        MoveDeltaCounter((SoundE._1C << vv) & 0x80);
        SoundE._1A = SoundE._04p._04_hi;
        if (SoundE._14) {
          SoundE._14--;
        }
        if (!SoundE._14) {
          switch (sbuffer.apu.DMC.LoopSample) {
          case 0: {
            SoundE._00 = 0;
            if (sbuffer.apu.DMC.IRQEnable == 1) {
              SoundE._20 = 1;
            }
          } break;
          case 1: {
            SoundE._14 = (SoundE._10 + 1) * 8;
            SoundE._04p._04_hi = 0;
            SoundE._1A = 0;
          } break;
          }
        }
      }
    }
  }
  MoveBias();
  return MoveVoltage();
}

s8 __GetWave_Triangle(s32 a) { return TRIANGLE_TABLE[a]; }

u32 __PitchTo32_HVC_C(u16 a) {
  u32 pitch;
  if (a == 0) {
    return 0;
  } else {
    f32 scalefactor = 16.f;
    return static_cast< u32 >(NES_CLOCK_RATE / scalefactor / (a + 1) / 1000.875f * 32768.f);
  }
}
u32 sample_timer = 0;
u32 abs_timer = 0;

void __Sound_Write_C(u16 a, u8 b) {
  switch (a) {
  case 0x8: {
    u8 v = SoundC._14;
    if (sbuffer.apu.Triangle.LinearCounterReloadVaue) {
      SoundC._24 = sbuffer.apu.Triangle.LinearCounterReloadVaue + 1;
    } else {
      SoundC._24 = 0;
    }

    if (SoundC._00 && SoundC._4A) {
      SoundC._14 = sbuffer.apu.Triangle.LengthCounter;
      SoundC._20 = SoundC._24;
    } else if (SoundC._00) {
      if (v && !sbuffer.apu.Triangle.LengthCounter) {
        SoundC._14 = sbuffer.apu.Triangle.LengthCounter;
        SoundC._20 = SoundC._24;
        SoundC._4A = 0x80;
      } else if (v) {
        SoundC._20 = SoundC._24;
      }
    } else if (SoundC._4A) {
      SoundC._14 = sbuffer.apu.Triangle.LengthCounter;
      SoundC._20 = SoundC._24;
      if (sbuffer.apu.TriangleStatus) {
        SoundC._00 = 1;
      }
    }

    if (!sbuffer.apu.Triangle.LinearCounterReloadVaue && !SoundC._14) {
      SoundC._00 = 0;
    }
  } break;
  case 0xa: {
    SoundC._0C = (sbuffer.apu.Triangle.TimerHigh << 8) | sbuffer.apu.Triangle.TimerLow;
    SoundC._08 = __PitchTo32_HVC_C(SoundC._0C);
  } break;
  case 0xb: {
    sample_timer = 0;
    SoundC._0C = (sbuffer.apu.Triangle.TimerHigh << 8) | sbuffer.apu.Triangle.TimerLow;
    SoundC._18 = LEN_TABLE_HVC[sbuffer.apu.Triangle.LengthCounterLoad];
    SoundC._10 = SoundC._18 * 4;
    if (!SoundC._00 || !SoundC._20) {
      SoundC._04 = 0;
      SoundC._1C = ((127 - bias) >> 3) + 16;
      SoundC._48 = 0;
      SoundC._08 = __PitchTo32_HVC_C(SoundC._0C);
    } else {
      SoundC._08 = __PitchTo32_HVC_C(SoundC._0C);
    }
    SoundC._4A = 60;
    SoundC._14 = sbuffer.apu.Triangle.LengthCounter;
    SoundC._20 = SoundC._24;
    if (sbuffer.apu.TriangleStatus && sbuffer.apu.Triangle.LinearCounterReloadVaue) {
      if (!SoundC._00) {
        SoundC._00 = 1;
      }
    } else {
      SoundC._00 = 0;
    }
  } break;
  }
}

int __ProcessSoundC() {
  static s16 timer = 0;
  sample_timer++;
  abs_timer++;

  u16 v = SoundC._08;
  if (!SoundC._00 || !SoundC._20) {
    if (SoundC._3E) {
      SoundC._3E--;
    }
  } else if (SoundC._3E < 0x20) {
    SoundC._3E++;
  }

  s16 b = static_cast< int >(__GetWave_Triangle(SoundC._04p._04_hi));
  SoundC._1C = ((127 - bias) >> 3) + 0x10;
  b *= SoundC._1C;
  SoundC._04 = SoundC._04 + SoundC._08;
  SoundC._04 &= 0x1fffff;
  SoundC._40 = v;

  if (!SoundC._14) {
    if (PHASE_SYNC_FLAG && SoundC._10) {
      SoundC._10--;
    }
    if (!SoundC._10 && !SoundC._14) {
      SoundC._00 = 0;
    }
    if (SoundC._20 && PHASE_SYNC_FLAG) {
      SoundC._20--;
    }
  }

  if (!SoundC._20 && !SoundC._14) {
    SoundC._00 = 0;
  }

  if (SoundC._4A) {
    SoundC._4A--;
  }

  s16 ret = (b * SoundC._3E) >> 5;
  return ret;
}

u8 DISKFM_TABLE[64];
u8 DISKSUB_TABLE[32];
s16 disksubwave[64];

int __GetWave_Disk(int wave, int index) {
  switch (wave) {
  case 8:
    return DISKFM_TABLE[index];
  case 9:
    if (DISKSUB_TABLE[index] == 4) {
      SoundF.modulationAccumulator = 0;
    } else {
      if (index == SoundF._0C) {
        return static_cast< u8 >(SoundF.modulationAccumulator);
      }
      while (index != SoundF._0C) {
        SoundF._0C++;
        SoundF._0C &= 0x3F;
        SoundF.modulationAccumulator =
            static_cast< u8 >(SoundF.modulationAccumulator) + disksubwave[SoundF._0C];
        if (SoundF.modulationAccumulator >= 0x40) {
          SoundF.modulationAccumulator = static_cast< u8 >(SoundF.modulationAccumulator) - 0x80;
        }
        if (SoundF.modulationAccumulator <= -0x40) {
          SoundF.modulationAccumulator = static_cast< u8 >(SoundF.modulationAccumulator) + 0x80;
        }
      }
    }
    wave = static_cast< u8 >(SoundF.modulationAccumulator);
    if (static_cast< s8 >(wave) > 0x40 || static_cast< s8 >(wave) < -0x20) {
      wave = 0xE0;
    }
    break;
  }
  return wave;
}

void __CreateDiskSubWave() {
  for (int i = 0; i < 32; i++) {
    disksubwave[i * 2] = DISK_SUB_GAIN[DISKSUB_TABLE[i]];
    disksubwave[i * 2 + 1] = DISK_SUB_GAIN[DISKSUB_TABLE[i]];
  }
}

u32 __PitchTo32_DISKFM(u16 v) {
#if VERSION == VERSION_GM8P_00
  return (NES_CLOCK_RATE / 8.f / 262144.f * static_cast< int >(v)) / 500.4375f * 32768.f;
#else
  return (0.85343015f * static_cast< int >(v)) / 500.4375f * 32768.f;
#endif
}

void __Sound_Write_Disk(u16 a, u8 b) {
  static int shiftr = 0;
  sbuffer.buff[a] = b;
  if (a == 0x23) {
    if (!(b & 2)) {
      SoundF._2D = 0;
      SoundF._2C = 0;
    }
  } else if (a >= 0x40 && a <= 0x7f) {
    int index = a;
    DISKFM_TABLE[index - 0x40] = reinterpret_cast< WaveTableEntry* >(&sbuffer.buff[index])->Sample;
  } else {
    switch (a) {
    case 0x80: {
      u32 old_2c = SoundF._2C;
      SoundF._2C = sbuffer.apu.VolumeEnvelopeEnabled;
      SoundF._2E = sbuffer.apu.VolumeEnvelopeIncreasing;
      if (SoundF._2C == 1) {
        SoundF._2D = sbuffer.apu.VolumeEnvelopeGainAndSpeed;
      } else {
        SoundF._28 = (DISK_FRAME_SAMPLE * (sbuffer.apu.VolumeEnvelopeGainAndSpeed + 1)) >> 4;
      }
      if (SoundF._2C != old_2c) {
        SoundF._24 = 0;
      }
    } break;
    case 0x82: {
      SoundF._14 = sbuffer.apu.FrequencyHigh << 8 | sbuffer.apu.FrequencyLow;
      SoundF._10 = __PitchTo32_DISKFM(SoundF._14);
    } break;
    case 0x83: {
      SoundF._14 = sbuffer.apu.FrequencyHigh << 8 | sbuffer.apu.FrequencyLow;
      SoundF._10 = __PitchTo32_DISKFM(SoundF._14);
      if (sbuffer.apu.HighSpeedFrequency) {
        SoundF._04 = 0;
      }
    } break;
    case 0x84: {
      u32 c = SoundF._38;
      SoundF._38 = sbuffer.apu.ModEnvelopeEnabled;
      SoundF._3A = sbuffer.apu.ModEnvelopeIncreasing;
      if (SoundF._38 == 1) {
        SoundF._39 = sbuffer.apu.ModEnvelopeGainAndSpeed;
      } else {
        SoundF._34 = (DISK_FRAME_SAMPLE * (sbuffer.apu.ModEnvelopeGainAndSpeed + 1)) >> 4;
      }
      if (SoundF._38 != c) {
        SoundF._30 = 0;
      }
    } break;
    case 0x85: {
      SoundF._44 = sbuffer.apu.ModCounter;
      if (SoundF._44 & 0x40) {
        SoundF._44 |= 0x80;
      }
    } break;
    case 0x86: {
      SoundF._1C = sbuffer.apu.ModFrequencyHigh << 8 | sbuffer.apu.ModFrequencyLow;
      SoundF._18 = __PitchTo32_DISKFM(SoundF._1C);
    } break;
    case 0x87: {
      u32 c = sbuffer.apu.HaltModTableCounter;
      SoundF._1C = sbuffer.apu.ModFrequencyHigh << 8 | sbuffer.apu.ModFrequencyLow;
      SoundF._18 = __PitchTo32_DISKFM(SoundF._1C);
      SoundF._46 = sbuffer.apu.HaltModTableCounter;
      if (!SoundF._46 && c == 1) {
        __CreateDiskSubWave();
      }
      if (SoundF._46) {
        shiftr = 0;
      }
      SoundF._0C = -1;
      SoundF._08 = 0;
      SoundF.modulationAccumulator = 0;

    } break;
    case 0x88: {
      if (shiftr < 0x20) {
        DISKSUB_TABLE[shiftr & 0x1f] = b & 7;
        if (++shiftr == 0x20) {
          __CreateDiskSubWave();
        }
      }
    } break;
    case 0x89: {
      SoundF._43 = DISKFM_GAINTABLE[sbuffer.apu.WaveTableMasterVolume];
      SoundF._45 = sbuffer.apu.WaveTableWriteEnable;
    } break;
    case 0x8a: {
      SoundF._48 = sbuffer.apu.EnvelopeSpeed;
    } break;
    }
  }
}

u16 __ProcessSoundF() {
  s8 modulation;
  if (SoundF._46) {
    modulation = 0;
  } else if (!SoundF._18) {
    modulation = 0;
  } else {
    modulation = __GetWave_Disk(9, SoundF._08p._08_hi);
    SoundF._08 += SoundF._18;
    SoundF._08 &= 0x3FFFFF;
  }

  int gain = VOLTABLE_DISKFM_SUB[SoundF._39];
  int offset = modulation + SoundF._44;
  if (offset >= 0x40) {
    offset = 0x40;
  }
  if (offset <= -0x20) {
    offset = -0x20;
  }
  gain *= offset;
  if (gain < -0x3FF) {
    gain = -0x3FF;
  }

  u16 sample = static_cast< u8 >(__GetWave_Disk(8, SoundF._04p._04_hi));
  SoundF._04 += static_cast< int >(SoundF._10 * ((1024.f + gain) / 1024.f));
  SoundF._3C = SoundF._1E;
  SoundF._3E = SoundF._10;
  sample *= SoundF._1E;
  sample *= 2;
  sample = (SoundF._40 + sample) >> 1;
  SoundF._40 = sample;
  SoundF._04 &= 0x3FFFFF;

  SoundF._24++;
  if (!SoundF._2C) {
    if (SoundF._28 && SoundF._24 % SoundF._28 == 0) {
      switch (SoundF._2E) {
      case 0:
        if (SoundF._2D) {
          SoundF._2D--;
        }
        break;
      case 1:
        if (SoundF._2D < 32) {
          SoundF._2D++;
        } else {
          SoundF._2D = 32;
        }
        break;
      }
      u8 volume = SoundF._2D;
      if (volume > 32) {
        volume = 32;
      }
      SoundF._1E = VOLTABLE_DISKFM[volume];
    }
  } else {
    u8 volume = SoundF._2D;
    if (volume > 32) {
      volume = 32;
    }
    SoundF._1E = VOLTABLE_DISKFM[volume];
  }
  if (!SoundF._38) {
    SoundF._30++;
    if (SoundF._30 % SoundF._34 == 0) {
      switch (SoundF._3A) {
      case 0:
        if (SoundF._39) {
          SoundF._39--;
        } else {
          SoundF._39 = 0;
        }
        break;
      case 1:
        if (SoundF._39 < 32) {
          SoundF._39++;
        } else {
          SoundF._39 = 32;
        }
        break;
      }
      u8 volume = SoundF._39;
      if (volume > 32) {
        volume = 32;
      }
      SoundF._20 = VOLTABLE_DISKFM_SUB[volume];
    }
  } else {
    u8 volume = SoundF._39;
    if (volume > 32) {
      volume = 32;
    }
    SoundF._20 = VOLTABLE_DISKFM_SUB[volume];
  }
  return sample;
}

s8 __GetWave_PCM(int index) { return SoundP._10[index]; }

int __ProcessSoundP() {
  if (SoundP._0D) {
    SoundP._04 = 0;
    SoundP._0D--;
    SoundP._10 = &PCMH2.samples[PCMH2.offsets[SoundP._0D]];
    SoundP._14 = PCMH2.lengths[SoundP._0D];
    SoundP._08 = (PCMH2.frequencies[SoundP._0D] << 16) / 32028;
    SoundP._0D = 0;
    SoundP._00 = 1;
  }
  if (SoundP._04p._04_hi >= SoundP._14) {
    SoundP._00 = 0;
  }
  if (!SoundP._00) {
    return 0;
  }
  s16 sample = __GetWave_PCM(SoundP._04p._04_hi) << 6;
  SoundP._04 += SoundP._08;
  return sample;
}

void __Sound_Write_HVC(u16 index, u8 value);

OSMessageQueue SoundQ;
OSMessage SoundQbuf[0x1000];

void HS_Event_Reset() {
  OSInitMessageQueue(&SoundQ, SoundQbuf, sizeof(SoundQbuf) / sizeof(SoundQbuf[0]));
}

void HS_Event_Update() {
  union {
    OSMessage raw;
    u8 bytes[sizeof(OSMessage)];
  } message;

  while (1) {
    if (!OSReceiveMessage(&SoundQ, &message.raw, 0)) {
      break;
    }
    __Sound_Write_HVC(message.bytes[0], message.bytes[1]);
  }
}

void HS_Event_Write(u16 a, u8 b) {
  if (a >= 0x5000 && a <= 0x5015) {
    a = (a - 0xf40);
  }

  OSSendMessage(&SoundQ, reinterpret_cast< OSMessage >((a - 0x4000) << 0x18 | b << 0x10), 0);
  switch (a & 0xff) {
  case 3: {
    DUMMY_ACTIVE[0] = 2;
  } break;
  case 7: {
    DUMMY_ACTIVE[1] = 2;
  } break;
  case 0xb: {
    DUMMY_ACTIVE[2] = 2;
  } break;
  case 0xf: {
    DUMMY_ACTIVE[3] = 2;
  } break;
  case 0x15: {
    if (!(b & 1)) {
      DUMMY_ACTIVE[0] = 1;
    }
    if (!(b & 2)) {
      DUMMY_ACTIVE[1] = 1;
    }
    if (!(b & 4)) {
      DUMMY_ACTIVE[2] = 1;
    }
    if (!(b & 8)) {
      DUMMY_ACTIVE[3] = 1;
    }
    if (!(b & 0x10)) {
      DUMMY_ACTIVE[4] = 1;
    }
    if (b & 0x10) {
      DUMMY_ACTIVE[4] = 2;
    }
  } break;
  case 0xc3: {
    DUMMY_ACTIVE[6] = 2;
  } break;
  case 0xc7: {
    DUMMY_ACTIVE[7] = 2;
  } break;
  case 0xcb: {
    DUMMY_ACTIVE[8] = 2;
  } break;
  case 0xd5: {
    if (!(b & 1)) {
      DUMMY_ACTIVE[6] = 1;
    }
    if (!(b & 2)) {
      DUMMY_ACTIVE[7] = 1;
    }
    if (!(b & 4)) {
      DUMMY_ACTIVE[8] = 1;
    }
  } break;
  }
}

void ProcessPhaseCounter() {
  if (PHASE_SYNC_COUNTER != 0) {
    PHASE_SYNC_COUNTER--;
  }
  if (PHASE_SYNC_COUNTER == 0) {
    PHASE_SYNC_FLAG = 1;
    PHASE_SYNC_COUNTER = PHASE_SAMPLE;
  }

  if (PHASE_SYNC_FLAG) {
    if (FRAME_SYNC_COUNTER != 0) {
      FRAME_SYNC_COUNTER--;
    }
    if (FRAME_SYNC_COUNTER == 0) {
      FRAME_SYNC_FLAG = 1;
      FRAME_SYNC_COUNTER = 4;
    }
  }
}

void ForceProcessPhaseCounter() {
  PHASE_SYNC_COUNTER = 0;
  ProcessPhaseCounter();
}

int Sound_Make_HVC(s32 count, s16* samples) {
  static int lastsample;
  int a;
  int b;
  int c;
  int d;
  int e;
  u16 f;
  int p;
  u16 x;
  u16 y;
  u16 z;
  HS_Event_Update();
  for (int i = 0; i < count; ++i) {
    ProcessPhaseCounter();
    a = __ProcessSoundA();
    b = __ProcessSoundB();
    c = __ProcessSoundC();
    d = __ProcessSoundD();
    e = __ProcessSoundE();
    f = __ProcessSoundF();
    p = __ProcessSoundP();
    switch (MMC_MODE) {
    case 1:
      x = __ProcessSoundVRC(0);
      y = __ProcessSoundVRC(1);
      z = __ProcessSoundVRC(2);
      break;
    case 0:
      x = __ProcessSoundMMC5(&SoundX);
      y = __ProcessSoundMMC5(&SoundY);
      z = SoundZ._3E;
      break;
    default:
      x = 0;
      y = 0;
      z = 0;
      break;
    }
    p = static_cast< s16 >(p);
    e = static_cast< s16 >(e);
    int sample = static_cast< s16 >(a) + static_cast< s16 >(b) - static_cast< s16 >(c) -
                 static_cast< s16 >(d) + p + e - static_cast< u16 >(f) + static_cast< u16 >(x) +
                 static_cast< u16 >(y) - static_cast< u16 >(z);
    if (sample >= 0x7FFF) {
      sample = 0x7FFF;
    } else if (sample <= -0x7FFF) {
      sample = -0x7FFF;
    }
    lastsample += (sample - lastsample) / 2;
    samples[i] = lastsample;
    PHASE_SYNC_FLAG = 0;
    FRAME_SYNC_FLAG = 0;
  }
  DUMMY_ACTIVE[0] = 0;
  DUMMY_ACTIVE[1] = 0;
  DUMMY_ACTIVE[2] = 0;
  DUMMY_ACTIVE[3] = 0;
  DUMMY_ACTIVE[4] = 0;
  DUMMY_ACTIVE[5] = 0;
  DUMMY_ACTIVE[6] = 0;
  DUMMY_ACTIVE[7] = 0;
  DUMMY_ACTIVE[8] = 0;
}

int write_pointer = 0x690;
int buffer_remain = 0x690;
s16 sound_loop_buffer[0x2000];

int read_pointer;

void Buffer_Reset() {
  for (int i = 0; i < sizeof(sound_loop_buffer) / sizeof(sound_loop_buffer[0]); i++) {
    sound_loop_buffer[i] = 0;
  }
  read_pointer = 0;
  write_pointer = 0x690;
  buffer_remain = 0x690;
}

int Sound_ReadSample(s16* sample) {
  if (buffer_remain == 0) {
    Buffer_Reset();
    return 0;
  }
  *sample = sound_loop_buffer[read_pointer];
  sound_loop_buffer[read_pointer] = 0;
  if (++read_pointer == 0x2000) {
    read_pointer = 0;
  }
  --buffer_remain;
  return 1;
}

void Sound_Write(u16 event, u8 value, u16 frames) {
  if (event == 0) {
    static s16 samples[16];
    static f32 sampleCarry = 0.f;
    static f32 sampleRate;
    BOOL interrupts = OSDisableInterrupts();
    if (buffer_remain < 0x460) {
#if VERSION == VERSION_GM8P_00
      sampleRate = ((5.f * (0x460 - buffer_remain) + 1680.f) / 1680.f) * SOUND_SAMPLE_RATE;
#else
      sampleRate = SOUND_SAMPLE_RATE * ((5.f * (0x460 - buffer_remain) + 1680.f) / 1680.f);
#endif
      if (sampleRate > 2.5) {
        sampleRate = 2.5f;
      }
    } else if (buffer_remain > 0x690) {
      sampleRate = SOUND_SAMPLE_RATE * (1680.f / ((buffer_remain - 0x690) * 5 + 0x690));
    } else {
      sampleRate = SOUND_SAMPLE_RATE;
    }
    sampleCarry += sampleRate;
    int count = sampleCarry;
    sampleCarry -= count;
    Sound_Make_HVC(count, samples);
    if (count != 0) {
      for (u32 i = 0; i < count; ++i) {
        if (buffer_remain == 0x2000) {
          break;
        }
        sound_loop_buffer[write_pointer] = samples[i];
        if (++write_pointer == 0x2000) {
          write_pointer = 0;
        }
        ++buffer_remain;
      }
    }
    OSRestoreInterrupts(interrupts);
  } else {
    HS_Event_Write(event, value);
  }
}

void Sound_Reset() {
  HS_Event_Reset();
  Jac_bzero(&SoundA, sizeof(SoundA));
  Jac_bzero(&SoundB, sizeof(SoundB));
  Jac_bzero(&SoundC, sizeof(SoundC));
  Jac_bzero(&SoundD, sizeof(SoundD));
  Jac_bzero(&SoundX, sizeof(SoundX));
  Jac_bzero(&SoundY, sizeof(SoundY));
  Jac_bzero(&SoundZ, sizeof(SoundZ));
  Jac_bzero(&SoundM, sizeof(SoundM));
  Jac_bzero(&SoundN, sizeof(SoundN));
  Jac_bzero(&SoundE, sizeof(SoundE));
  Jac_bzero(&SoundF, sizeof(SoundF));
  Jac_bzero(&SoundP, sizeof(SoundP));
  SoundA._3E = 0;
  SoundB._3E = 0;
  SoundC._3E = 0;
  SoundD._3E = 0;
  SoundF._3C = 0;
  SoundP._0E = 0;

  SoundX._3E = 0;
  SoundY._3E = 0;
  SoundZ._3E = 0;

  SoundA._04 = 0;
  SoundB._04 = 0;
  SoundC._04 = 0;
  SoundD._04 = 0;
  SoundE._04 = 0;
  SoundF._04 = 0;
  SoundP._04 = 0;

  SoundA._48 = 0;
  SoundB._48 = 0;
  SoundC._48 = 0;
  SoundX._04 = 0;
  SoundY._04 = 0;
  SoundZ._04 = 0;
  SoundP._0D = 0;
  WriteBias(0);
  Buffer_Reset();
#if VERSION == VERSION_GM8P_00
  OSReport("*******-------- HVC Sound Emulator \n");
  switch (VIGetTvFormat()) {
  case VI_NTSC:
  case VI_MPAL:
  case VI_EURGB60:
    OSReport("*******-------- NTSC/MPAL/EU60 \n");
    SOUND_SAMPLE_RATE = 2;
    NES_CLOCK_RATE = 1789882.75f;
    NES_FRAME_RATE = 60;
    break;
  default:
    OSReport("*******-------- Unknown TV format... \n");
    // Fall through to PAL timing for unknown formats.
  case VI_PAL:
    OSReport("*******-------- PAL50 \n");
    SOUND_SAMPLE_RATE = 2;
    NES_CLOCK_RATE = 1662500.f;
    DISK_FRAME_SAMPLE = 640;
    FRAME_SAMPLE = 640;
    PHASE_SAMPLE = 160;
    NES_FRAME_RATE = 50;
    break;
  }
#endif
}

u8 Sound_Read(u16 reg_addr) {
  if (reg_addr == 0x5015) {
    u8 x, y, z;
    if (!DUMMY_ACTIVE[6]) {
      x = SoundX._00;
    }
    if (!DUMMY_ACTIVE[7]) {
      y = SoundY._00;
    }
    if (!DUMMY_ACTIVE[8]) {
      z = SoundZ._00;
    }
    return z << 2 | y << 1 | (x);
  } else if (reg_addr == 0x4015) {
#if VERSION == VERSION_GM8P_00
    u8 a = SoundA._00;
    if (DUMMY_ACTIVE[0]) {
      a = DUMMY_ACTIVE[0] - 1;
    }
    u8 b = SoundB._00;
    if (DUMMY_ACTIVE[1]) {
      b = DUMMY_ACTIVE[1] - 1;
    }
    u8 c = SoundC._00;
    if (DUMMY_ACTIVE[2]) {
      c = DUMMY_ACTIVE[2] - 1;
    }
    u8 d = SoundD._00;
    if (DUMMY_ACTIVE[3]) {
      d = DUMMY_ACTIVE[3] - 1;
    }
    u8 e = SoundE._00;
    if (DUMMY_ACTIVE[4]) {
      e = DUMMY_ACTIVE[4] - 1;
    }
#else
    u8 a, b, c, d, e;
    if (DUMMY_ACTIVE[0]) {
      a = DUMMY_ACTIVE[0] - 1;
    } else {
      a = SoundA._00;
    }

    if (DUMMY_ACTIVE[1]) {
      b = DUMMY_ACTIVE[1] - 1;
    } else {
      b = SoundB._00;
    }

    if (DUMMY_ACTIVE[2]) {
      c = DUMMY_ACTIVE[2] - 1;
    } else {
      c = SoundC._00;
    }

    if (DUMMY_ACTIVE[3]) {
      d = DUMMY_ACTIVE[3] - 1;
    } else {
      d = SoundD._00;
    }

    if (DUMMY_ACTIVE[4]) {
      e = DUMMY_ACTIVE[4] - 1;
    } else {
      e = SoundE._00;
    }
#endif
    return SoundE._20 << 7 | e << 4 | d << 3 | c << 2 | b << 1 | a;
  } else {
    switch (reg_addr & 0xff) {
    case 0x90:
      return SoundF._2D;
    case 0x92:
      return SoundF._39;
    default:
#if VERSION == VERSION_GM8P_00
      OSReport("Unsupported SoundRead %x\n", reg_addr);
#endif
      return 0;
    }
  }
}

void Sound_SetC000(u8* romTop) { ROM_TOP_C000 = romTop; }
void Sound_SetE000(u8* romTop) { ROM_TOP_E000 = romTop; }

void Sound_SetMMC(u8 mmcMode) {
  FRAME_SAMPLE = 0x280;
  PHASE_SAMPLE = 0xa0;
  beforemode = 0;
  MMC_MODE = mmcMode;
  Sound_Reset();
}

void __Sound_Write_HVC(u16 index, u8 v) {
  sbuffer.buff[index] = v;
  if (index >= 0xc0) {
    switch (MMC_MODE) {
    case 1: {
      __Sound_Write_VRC(index, v);
    } break;
    case 0: {
      __Sound_Write_MMC5(index, v);
    } break;
    }
  } else if (index > 0x17) {
    __Sound_Write_Disk(index, v);
  } else {
    switch (index >> 2) {
    case 0: {
      __Sound_Write_A(index, v);
    } break;
    case 1: {
      __Sound_Write_B(index, v);
    } break;
    case 2: {
      __Sound_Write_C(index, v);
    } break;
    case 3: {
      __Sound_Write_D(index, v);
    } break;
    case 4: {
      __Sound_Write_E(index, v);
    } break;

    case 5: {
      if (index == 0x17) {
        if (v & 0x80) {
          FRAME_SAMPLE = 0x280;
          PHASE_SAMPLE = 0xa0;
          beforemode = 0;
        } else {
#if VERSION == VERSION_GM8P_00
          FRAME_SAMPLE = 32028 / NES_FRAME_RATE;
          PHASE_SAMPLE = FRAME_SAMPLE / 4;
#else
          FRAME_SAMPLE = 0x215;
          PHASE_SAMPLE = 0x85;
#endif
          beforemode = 1;
        }
        ForceProcessPhaseCounter();
      } else {
        if (!(v & 1)) {
          SoundA._00 = 0;
        }
        if (!(v & 2)) {
          SoundB._00 = 0;
        }
        if (!(v & 4)) {
          SoundC._00 = 0;
        }
        if (!(v & 8)) {
          SoundD._00 = 0;
        }
        if (!(v & 0x10)) {
          SoundE._00 = 0;
        }
        if ((v & 0x10) && !SoundE._00) {
          StartE();
        }
      }
    } break;
    }
  }
}

// Unreferenced state and PCM tables retained by the original module.
u32 lbl_1_bss_9E18;
u32 lbl_1_data_1F09C[6] = {0, 0, 0, 0, 0, 0};
u32 _STOP[2] = {0, 0};
void* PCM[9] = {_STOP, _STOP, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};
u32 PCMSIZE[9] = {sizeof(_STOP), sizeof(_STOP), 0, 0, 0, 0, 0, 0, 0};
