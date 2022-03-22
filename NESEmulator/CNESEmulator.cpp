#include "CNESEmulator.hpp"
#include "CGameState.hpp"
#include "Input/CFinalInput.hpp"
#include "logvisor/logvisor.hpp"
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdbool>
#include <cstring>
#include "malloc.h"
#include <cinttypes>
#include <cctype>
#include <ctime>
#include <cmath>

static metaforce::MP1::CNESEmulator* EmulatorInst = nullptr;

extern "C" {

#include "fixNES/mapper.h"
#include "fixNES/cpu.h"
#include "fixNES/ppu.h"
#include "fixNES/mem.h"
#include "fixNES/input.h"
#include "fixNES/fm2play.h"
#include "fixNES/apu.h"
#include "fixNES/audio_fds.h"
#include "fixNES/audio_vrc7.h"
#include "fixNES/mapper_h/nsf.h"

/*
 * Portions Copyright (C) 2017 - 2019 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#define DEBUG_HZ 0
#define DEBUG_MAIN_CALLS 0
#define DEBUG_KEY 0
#define DEBUG_LOAD_INFO 1

#if 0
#ifndef _WIN32
std::chrono::steady_clock::time_point s_tp = std::chrono::steady_clock::now();
static std::chrono::milliseconds::rep GetTickCount() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s_tp).count();
}
#endif
#endif

const char *VERSION_STRING = "fixNES Alpha v1.2.7";
static char window_title[256];
static char window_title_pause[256];

enum {
  FTYPE_UNK = 0,
  FTYPE_NES,
  FTYPE_NSF,
  FTYPE_FDS,
  FTYPE_QD,
#if ZIPSUPPORT
  FTYPE_ZIP,
#endif
};

static int emuFileType = FTYPE_UNK;
static char emuFileName[1024];
uint8_t *emuNesROM = NULL;
uint32_t emuNesROMsize = 0;
#ifndef __LIBRETRO__
static char emuSaveName[1024];
#endif
uint8_t *emuPrgRAM = NULL;
uint32_t emuPrgRAMsize = 0;
//used externally
#ifdef COL_32BIT
uint32_t textureImage[0xF000];
#define TEXIMAGE_LEN VISIBLE_DOTS*VISIBLE_LINES*4
#ifdef COL_GL_BSWAP
#define GL_TEX_FMT GL_UNSIGNED_INT_8_8_8_8_REV
#else //no REVerse
#define GL_TEX_FMT GL_UNSIGNED_INT_8_8_8_8
#endif
#else //COL_16BIT
uint16_t textureImage[0xF000];
#define TEXIMAGE_LEN VISIBLE_DOTS*VISIBLE_LINES*2
#ifdef COL_GL_BSWAP
#define GL_TEX_FMT GL_UNSIGNED_SHORT_5_6_5_REV
#else //no REVerse
#define GL_TEX_FMT GL_UNSIGNED_SHORT_5_6_5
#endif
#endif
bool nesPause = false;
bool ppuDebugPauseFrame = false;
bool doOverscan = false;
bool nesPAL = false;
bool nesEmuNSFPlayback = false;
uint8_t emuInitialNT = NT_UNKNOWN;

// static bool inPause = false;
// static bool inOverscanToggle = false;
// static bool inResize = false;
// static bool inDiskSwitch = false;
// static bool inReset = false;

#if DEBUG_HZ
static int emuFrameStart = 0;
static int emuTimesCalled = 0;
static int emuTotalElapsed = 0;
#endif
#if DEBUG_MAIN_CALLS
static int emuMainFrameStart = 0;
static int emuMainTimesCalled = 0;
static int emuMainTimesSkipped = 0;
static int emuMainTotalElapsed = 0;
#endif

#define DOTS 341

#define VISIBLE_DOTS 256
#define VISIBLE_LINES 240

static uint32_t linesToDraw = VISIBLE_LINES;
static const uint32_t visibleImg = VISIBLE_DOTS * VISIBLE_LINES * 4;
// static uint8_t scaleFactor = 2;
static bool emuSaveEnabled = false;
static bool emuFdsHasSideB = false;

// static uint16_t ppuCycleTimer;
//static uint16_t ppuCycleTimer;
uint32_t cpuCycleTimer;
uint32_t vrc7CycleTimer;
//from input.c
extern uint8_t inValReads[8];
//from m30.c
extern bool m30_flashable;
extern bool m30_singlescreen;
//from m32.c
extern bool m32_singlescreen;
//from p16c8.c
extern bool m78_m78a;
//from ppu.c
extern bool ppuMapper5;

static volatile bool emuRenderFrame = false;
extern uint8_t audioExpansion;

// used externally
bool emuSkipVsync = false;
bool emuSkipFrame = false;

// static uint32_t mCycles = 0;

extern bool fdsSwitch;

uint32_t apuGetMaxBufSize();
void apuResetPos();
uint8_t* ppuGetVRAM();

int audioUpdate() {
  if (!EmulatorInst)
    return 0;
  return EmulatorInst->audioUpdate();
}
}

namespace metaforce::MP1 {

bool CNESEmulator::EmulatorConstructed = false;
static logvisor::Module Log("CNESEmulator");

#define NESEMUP_ROM_OFFSET 0xa3f8

#define METROID_PAL false
#define METROID_MAPPER 1
#define METROID_SAVE_ENABLED false
#define METROID_TRAINER false
#define METROID_PRG_SIZE (8 * 0x4000)
#define METROID_CHR_SIZE (0 * 0x2000)
#define METROID_PRG_RAM_SIZE 0x2000

CNESEmulator::CNESEmulator() {
  if (EmulatorConstructed)
    Log.report(logvisor::Fatal, FMT_STRING("Attempted constructing more than 1 CNESEmulator"));
  EmulatorConstructed = true;

  CDvdFile NESEmuFile("NESemuP.rel");
  if (NESEmuFile) {
    m_nesEmuPBuf.reset(new u8[0x20000]);
    m_dvdReq = NESEmuFile.AsyncSeekRead(m_nesEmuPBuf.get(), 0x20000, ESeekOrigin::Begin, NESEMUP_ROM_OFFSET);
  } else {
    Log.report(logvisor::Fatal, FMT_STRING("Unable to open NESemuP.rel"));
  }
}

void CNESEmulator::InitializeEmulator() {
  nesPause = false;
  ppuDebugPauseFrame = false;

  puts(VERSION_STRING);
  strcpy(window_title, VERSION_STRING);
  memset(textureImage, 0, visibleImg);
  emuFileType = FTYPE_UNK;
  memset(emuFileName, 0, 1024);
  memset(emuSaveName, 0, 1024);

  nesPAL = METROID_PAL;
  uint8_t mapper = METROID_MAPPER;
  emuSaveEnabled = METROID_SAVE_ENABLED;
  bool trainer = METROID_TRAINER;
  uint32_t prgROMsize = METROID_PRG_SIZE;
  uint32_t chrROMsize = METROID_CHR_SIZE;
  emuPrgRAMsize = METROID_PRG_RAM_SIZE;
  emuPrgRAM = (uint8_t*)malloc(emuPrgRAMsize);
  uint8_t* prgROM = emuNesROM;
  if (trainer) {
    memcpy(emuPrgRAM + 0x1000, prgROM, 0x200);
    prgROM += 512;
  }
  uint8_t* chrROM = NULL;
  if (chrROMsize) {
    chrROM = emuNesROM + prgROMsize;
    if (trainer)
      chrROM += 512;
  }
  apuInitBufs();
  cpuInit();
  ppuInit();
  memInit();
  apuInit();
  inputInit();
  ppuSetNameTblVertical();
#if DEBUG_LOAD_INFO
  printf("Used Mapper: %i\n", mapper);
  printf("PRG: 0x%x bytes PRG RAM: 0x%x bytes CHR: 0x%x bytes\n", prgROMsize, emuPrgRAMsize, chrROMsize);
#endif
  if (!mapperInit(mapper, prgROM, prgROMsize, emuPrgRAM, emuPrgRAMsize, chrROM, chrROMsize)) {
    printf("Mapper init failed!\n");
    return;
  }
#if DEBUG_LOAD_INFO
  printf("Trainer: %i Saving: %i VRAM Mode: Vertical\n", trainer, emuSaveEnabled);
#endif
  sprintf(window_title, "%s NES - %s\n", nesPAL ? "PAL" : "NTSC", VERSION_STRING);

  sprintf(window_title_pause, "%s (Pause)", window_title);
  sprintf(window_title_pause, "%s (Pause)", window_title);
#if DEBUG_HZ
  emuFrameStart = GetTickCount();
#endif
#if DEBUG_MAIN_CALLS
  emuMainFrameStart = GetTickCount();
#endif
  cpuCycleTimer = nesPAL ? 16 : 12;
  vrc7CycleTimer = 432 / cpuCycleTimer;
  // do one scanline per idle loop
  // ppuCycleTimer = nesPAL ? 5 : 4;
  // mainLoopRuns = nesPAL ? DOTS*ppuCycleTimer : DOTS*ppuCycleTimer;
  // mainLoopPos = mainLoopRuns;

//  CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx) {
//    // Nearest-neighbor FTW!
//    m_texture = ctx.newDynamicTexture(VISIBLE_DOTS, linesToDraw, boo::TextureFormat::RGBA8,
//                                      boo::TextureClampMode::ClampToEdgeNearest);
//    if (ctx.platform() == boo::IGraphicsDataFactory::Platform::OpenGL) {
//      Vert verts[4] = {
//          {{-1.f, -1.f, 0.f}, {0.f, 1.f}},
//          {{-1.f, 1.f, 0.f}, {0.f, 0.f}},
//          {{1.f, -1.f, 0.f}, {1.f, 1.f}},
//          {{1.f, 1.f, 0.f}, {1.f, 0.f}},
//      };
//      m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
//    } else {
//      Vert verts[4] = {
//          {{-1.f, 1.f, 0.f}, {0.f, 1.f}},
//          {{-1.f, -1.f, 0.f}, {0.f, 0.f}},
//          {{1.f, 1.f, 0.f}, {1.f, 1.f}},
//          {{1.f, -1.f, 0.f}, {1.f, 0.f}},
//      };
//      m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
//    }
//    m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
//    m_shadBind = CNESShader::BuildShaderDataBinding(ctx, m_vbo, m_uniBuf, m_texture);
//    return true;
//  } BooTrace);

  // double useFreq = 223740;
  double useFreq = apuGetFrequency();
  m_booVoice = CAudioSys::GetVoiceEngine()->allocateNewStereoVoice(useFreq, this);
  m_booVoice->start();
  uint32_t apuBufSz = apuGetMaxBufSize();
  m_audioBufBlock.reset(new u8[apuBufSz * NUM_AUDIO_BUFFERS]);
  memset(m_audioBufBlock.get(), 0, apuBufSz * NUM_AUDIO_BUFFERS);
  for (int i = 0; i < NUM_AUDIO_BUFFERS; ++i)
    m_audioBufs[i] = m_audioBufBlock.get() + apuBufSz * i;

  EmulatorInst = this;
}

void CNESEmulator::DeinitializeEmulator() {
  // printf("\n");
  emuRenderFrame = false;
  m_booVoice->stop();
  m_booVoice.reset();
  apuDeinitBufs();
  if (emuNesROM != NULL) {
    if (!nesEmuNSFPlayback && (audioExpansion & EXP_FDS)) {
      FILE* save = fopen(emuSaveName, "wb");
      if (save) {
        if (emuFdsHasSideB)
          fwrite(emuNesROM, 1, 0x20000, save);
        else
          fwrite(emuNesROM, 1, 0x10000, save);
        fclose(save);
      }
    }
  }
  if (emuPrgRAM != NULL) {
    if (emuSaveEnabled) {
      FILE* save = fopen(emuSaveName, "wb");
      if (save) {
        fwrite(emuPrgRAM, 1, emuPrgRAMsize, save);
        fclose(save);
      }
    }
    free(emuPrgRAM);
  }
  emuPrgRAM = NULL;
  // printf("Bye!\n");

  EmulatorInst = nullptr;
}

CNESEmulator::~CNESEmulator() {
  if (m_dvdReq)
    m_dvdReq->PostCancelRequest();
  if (EmulatorInst)
    DeinitializeEmulator();
  if (emuNesROM) {
    free(emuNesROM);
    emuNesROM = nullptr;
  }
  EmulatorConstructed = false;
}

int CNESEmulator::audioUpdate() {
  int origProcBufs = m_procBufs;

  uint8_t* data = apuGetBuf();
  if (data != NULL && m_procBufs) {
    uint32_t apuBufSz = apuGetMaxBufSize();
    uint32_t remBytes = apuGetBufSize();
    while (remBytes != 0) {
      size_t thisBytes = std::min(remBytes, apuBufSz - m_posInHeadBuf);
      memmove(m_audioBufs[m_headBuf] + m_posInHeadBuf, data, thisBytes);
      data += thisBytes;
      m_posInHeadBuf += thisBytes;
      if (m_posInHeadBuf == apuBufSz) {
        m_posInHeadBuf = 0;
        --m_procBufs;
        ++m_headBuf;
        if (m_headBuf == NUM_AUDIO_BUFFERS)
          m_headBuf = 0;
        // printf("PUSH\n");
      }
      remBytes -= thisBytes;
    }
  }

  // if (!origProcBufs)
  // printf("OVERRUN\n");

  return origProcBufs;
}

static constexpr uint32_t AudioFrameSz = 2 * sizeof(int16_t);

size_t CNESEmulator::supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data) {
  uint32_t remFrames = uint32_t(frames);
  while (remFrames) {
    if (m_posInTailBuf == apuGetMaxBufSize()) {
      ++m_tailBuf;
      if (m_tailBuf == NUM_AUDIO_BUFFERS)
        m_tailBuf = 0;
      m_posInTailBuf = 0;
      ++m_procBufs;
      // printf("POP\n");
    }

    if (m_procBufs == NUM_AUDIO_BUFFERS) {
      memset(data, 0, remFrames * AudioFrameSz);
      // printf("UNDERRUN\n");
      return frames;
    }

    size_t copySz = std::min(apuGetMaxBufSize() - m_posInTailBuf, remFrames * AudioFrameSz);
    memmove(data, m_audioBufs[m_tailBuf] + m_posInTailBuf, copySz);
    data += copySz / sizeof(int16_t);
    m_posInTailBuf += copySz;
    remFrames -= copySz / AudioFrameSz;
  }
  return frames;
}

void CNESEmulator::NesEmuMainLoop(bool forceDraw) {
  // int start = GetTickCount();
  int loopCount = 0;
  do {
    if (emuRenderFrame || nesPause) {
#if DEBUG_MAIN_CALLS
      emuMainTimesSkipped++;
#endif
      // printf("LC RENDER: %d\n", loopCount);
      // TODO TODO
//      m_texture->load(textureImage, visibleImg);
      emuRenderFrame = false;
      break;
    }
    ++loopCount;

    // main CPU clock
    if (!cpuCycle())
      exit(EXIT_SUCCESS);
    // run graphics
    ppuCycle();
    // run audio
    apuCycle();
    // mapper related irqs
    mapperCycle();
    // mCycles++;
    if (ppuDrawDone()) {
      // printf("%i\n",mCycles);
      // mCycles = 0;
#ifndef __LIBRETRO__
      emuRenderFrame = true;
#if 0
            if(fm2playRunning())
                fm2playUpdate();
#endif
#if DEBUG_HZ
      emuTimesCalled++;
      auto end = GetTickCount();
      emuTotalElapsed += end - emuFrameStart;
      if (emuTotalElapsed >= 1000) {
        printf("\r%iHz   ", emuTimesCalled);
        emuTimesCalled = 0;
        emuTotalElapsed = 0;
      }
      emuFrameStart = end;
#endif
      // update audio before drawing
      if (!apuUpdate()) {
        apuResetPos();
        break;
      }
      // glutPostRedisplay();
#if 0
            if(ppuDebugPauseFrame)
            {
                ppuDebugPauseFrame = false;
                nesPause = true;
            }
#endif
#endif
      if (nesEmuNSFPlayback)
        nsfVsync();

      // keep processing frames if audio buffers are underrunning
      if (emuSkipFrame)
        emuRenderFrame = false;

      continue;
    }
  } while (true);

#if 0
    int end = GetTickCount();
    printf("%dms %d %d\n", end - start, loopCount, m_procBufs);
#endif

#if DEBUG_MAIN_CALLS
  emuMainTimesCalled++;
  int end = GetTickCount();
  // printf("%dms\n", end - start);
  emuMainTotalElapsed += end - emuMainFrameStart;
  if (emuMainTotalElapsed >= 1000) {
    printf("\r%i calls, %i skips   ", emuMainTimesCalled, emuMainTimesSkipped);
    fflush(stdout);
    emuMainTimesCalled = 0;
    emuMainTimesSkipped = 0;
    emuMainTotalElapsed = 0;
  }
  emuMainFrameStart = end;
#endif
}

#if 0
static void nesEmuFdsSetup(uint8_t *src, uint8_t *dst)
{
    memcpy(dst, src, 0x38);
    memcpy(dst+0x3A, src+0x38, 2);
    uint16_t cDiskPos = 0x3E;
    uint16_t cROMPos = 0x3A;
    do
    {
        if(src[cROMPos] != 0x03)
            break;
        memcpy(dst+cDiskPos, src+cROMPos, 0x10);
        uint16_t copySize = (*(uint16_t*)(src+cROMPos+0xD))+1;
        cDiskPos+=0x12;
        cROMPos+=0x10;
        memcpy(dst+cDiskPos, src+cROMPos, copySize);
        cDiskPos+=copySize+2;
        cROMPos+=copySize;
    } while(cROMPos < 0xFFDC && cDiskPos < 0xFFFF);
    printf("%04x -> %04x\n", cROMPos, cDiskPos);
}
#endif

struct BitstreamState {
  u8* rPos;
  int position = 0;
  int tmpBuf = 0;
  int decBit = 0;
  BitstreamState(u8* pos) : rPos(pos) {}
  void resetDecBit() { decBit = 0; }
  void runDecBit() {
    if (position == 0) {
      position = 8;
      tmpBuf = *rPos++;
    }
    decBit <<= 1;
    if (tmpBuf & 0x80)
      decBit |= 1;
    tmpBuf <<= 1;
    position--;
  }
};

// Based on https://gist.github.com/FIX94/7593640c5cee6c37e3b23e7fcf8fe5b7
void CNESEmulator::DecryptMetroid(u8* dataIn, u8* dataOut, u32 decLen, u8 decByte, u32 xorLen, u32 xorVal) {
  u32 i, j;
  // simple add obfuscation
  for (i = 0; i < 0x100; i++) {
    dataIn[i] += decByte;
    decByte = dataIn[i];
  }
  // flip the first 0x100 bytes around
  for (i = 0; i < 128; ++i)
    std::swap(dataIn[255 - i], dataIn[i]);
  // set up buffer pointers
  BitstreamState bState(dataIn + 0x100);
  // unscramble buffer
  for (i = 0; i < decLen; i++) {
    bState.resetDecBit();
    bState.runDecBit();
    if (bState.decBit) {
      bState.resetDecBit();
      for (j = 0; j < 8; j++)
        bState.runDecBit();
      dataOut[i] = dataIn[bState.decBit + 0x49];
    } else {
      bState.resetDecBit();
      bState.runDecBit();
      if (bState.decBit) {
        bState.resetDecBit();
        for (j = 0; j < 6; j++)
          bState.runDecBit();
        dataOut[i] = dataIn[bState.decBit + 9];
      } else {
        bState.resetDecBit();
        bState.runDecBit();
        if (bState.decBit) {
          bState.resetDecBit();
          for (j = 0; j < 3; j++)
            bState.runDecBit();
          dataOut[i] = dataIn[bState.decBit + 1];
        } else
          dataOut[i] = dataIn[bState.decBit];
      }
    }
  }
  // do checksum fixups
  unsigned int xorTmpVal = 0;
  for (i = 0; i < xorLen; i++) {
    xorTmpVal ^= dataOut[i];
    for (j = 0; j < 8; j++) {
      if (xorTmpVal & 1) {
        xorTmpVal >>= 1;
        xorTmpVal ^= xorVal;
      } else
        xorTmpVal >>= 1;
    }
  }
  // write in calculated checksum
  dataOut[xorLen - 1] = u8((xorTmpVal >> 8) & 0xFF);
  dataOut[xorLen - 2] = u8(xorTmpVal & 0xFF);
}

void CNESEmulator::ProcessUserInput(const CFinalInput& input, int) {
  if (input.ControllerIdx() != 0)
    return;

  if (GetPasswordEntryState() != EPasswordEntryState::NotPasswordScreen) {
    // Don't swap A/B
    inValReads[BUTTON_A] = input.DA() || input.DSpecialKey(aurora::SpecialKey::Enter) ||
                           input.DMouseButton(EMouseButton::Primary);
    inValReads[BUTTON_B] = input.DB() || input.DSpecialKey(aurora::SpecialKey::Esc);
  } else {
    // Prime controls (B jumps, A shoots)
    inValReads[BUTTON_B] = input.DA() || input.DY() || input.DMouseButton(EMouseButton::Primary);
    inValReads[BUTTON_A] = input.DB() || input.DX() || input.DKey(' ');
  }

  inValReads[BUTTON_UP] = input.DDPUp() || input.DLAUp();
  inValReads[BUTTON_DOWN] = input.DDPDown() || input.DLADown();
  inValReads[BUTTON_LEFT] = input.DDPLeft() || input.DLALeft();
  inValReads[BUTTON_RIGHT] = input.DDPRight() || input.DLARight();
  inValReads[BUTTON_SELECT] = input.DZ() || input.DSpecialKey(aurora::SpecialKey::Tab);
  inValReads[BUTTON_START] = input.DStart() || input.DSpecialKey(aurora::SpecialKey::Esc);
}

bool CNESEmulator::CheckForGameOver(const u8* vram, u8* passwordOut) {
  // "PASS WORD"
  if (memcmp(vram + 0x14B, "\x19\xa\x1c\x1c\xff\x20\x18\x1b\xd", 9))
    return false;

  int chOff = 0;
  int encOff = 0;
  u8 pwOut[18];
  for (int i = 0; i < 24; ++i) {
    u8 chName = vram[0x1A9 + chOff];
    ++chOff;
    if (chOff == 0x6 || chOff == 0x46)
      ++chOff; // mid-line space
    else if (chOff == 0xd)
      chOff = 64; // 2nd line

    if (chName > 0x3f)
      return false;

    switch (i & 0x3) {
    case 0:
      pwOut[encOff] = chName;
      break;
    case 1:
      pwOut[encOff] |= chName << 6;
      ++encOff;
      pwOut[encOff] = chName >> 2;
      break;
    case 2:
      pwOut[encOff] |= chName << 4;
      ++encOff;
      pwOut[encOff] = chName >> 4;
      break;
    case 3:
      pwOut[encOff] |= chName << 2;
      ++encOff;
      break;
    default:
      break;
    }
  }

  if (passwordOut)
    memmove(passwordOut, pwOut, 18);
  return true;
}

CNESEmulator::EPasswordEntryState CNESEmulator::CheckForPasswordEntryScreen(const u8* vram) {
  // "PASS WORD PLEASE"
  if (memcmp(vram + 0x88, "\x19\xa\x1c\x1c\xff\x20\x18\x1b\xd\xff\x19\x15\xe\xa\x1c\xe", 16))
    return EPasswordEntryState::NotPasswordScreen;

  for (int i = 0; i < 13; ++i)
    if (vram[0x109 + i] < 0x40 || vram[0x149 + i] < 0x40)
      return EPasswordEntryState::Entered;

  return EPasswordEntryState::NotEntered;
}

bool CNESEmulator::SetPasswordIntoEntryScreen(u8* vram, u8* wram, const u8* password) {
  if (CheckForPasswordEntryScreen(vram) != EPasswordEntryState::NotEntered)
    return false;

  int i;
  for (i = 0; i < 18; ++i)
    if (password[i])
      break;
  if (i == 18)
    return false;

  int encOff = 0;
  int chOff = 0;
  u32 lastWord = 0;
  for (i = 0; i < 24; ++i) {
    switch (i & 0x3) {
    case 0:
      lastWord = password[encOff];
      ++encOff;
      break;
    case 1:
      lastWord = (lastWord >> 6) | (u32(password[encOff]) << 2);
      ++encOff;
      break;
    case 2:
      lastWord = (lastWord >> 6) | (u32(password[encOff]) << 4);
      ++encOff;
      break;
    case 3:
      lastWord = (lastWord >> 6);
      break;
    default:
      break;
    }

    u8 chName = u8(lastWord & 0x3f);
    wram[0x99a + i] = chName;
    vram[0x109 + chOff] = chName;
    ++chOff;
    if (chOff == 0x6 || chOff == 0x46)
      ++chOff; // mid-line space
    else if (chOff == 0xd)
      chOff = 64; // 2nd line
  }

  return true;
}

void CNESEmulator::Update() {
  if (!EmulatorInst) {
    if (m_dvdReq && m_dvdReq->IsComplete()) {
      m_dvdReq.reset();
      emuNesROMsize = 0x20000;
      emuNesROM = (uint8_t*)malloc(emuNesROMsize);
      DecryptMetroid(m_nesEmuPBuf.get(), emuNesROM);
      m_nesEmuPBuf.reset();
      InitializeEmulator();
    }
  } else {
    if (nesPause) {
      DeinitializeEmulator();
      InitializeEmulator();
      return;
    }

    bool gameOver = CheckForGameOver(ppuGetVRAM(), x21_passwordFromNES);
    x34_passwordEntryState = CheckForPasswordEntryScreen(ppuGetVRAM());
    if (x34_passwordEntryState == EPasswordEntryState::NotEntered && x38_passwordPending) {
      SetPasswordIntoEntryScreen(ppuGetVRAM(), emuPrgRAM, x39_passwordToNES);
      x38_passwordPending = false;
    }
    if (gameOver && !x20_gameOver)
      for (int i = 0; i < 3; ++i) // Three draw loops to ensure password display
        NesEmuMainLoop(true);
    else
      NesEmuMainLoop();
    x20_gameOver = gameOver;
  }
}

static const float NESAspect = VISIBLE_DOTS / float(VISIBLE_LINES);

void CNESEmulator::Draw(const zeus::CColor& mulColor, bool filtering) {
  if (!EmulatorInst)
    return;

  float widthFac = NESAspect / CGraphics::GetViewportAspect();

  Uniform uniform = {zeus::CMatrix4f{}, mulColor};
  uniform.m_matrix[0][0] = widthFac;
//  m_uniBuf->load(&uniform, sizeof(Uniform));
//
//  CGraphics::SetShaderDataBinding(m_shadBind);
//  CGraphics::DrawArray(0, 4);
}

void CNESEmulator::LoadPassword(const u8* state) {
  memmove(x39_passwordToNES, state, 18);
  x38_passwordPending = true;
}

} // namespace metaforce::MP1
