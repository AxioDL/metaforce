#include "CNESEmulator.hpp"
#include "CNESShader.hpp"
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

static urde::MP1::CNESEmulator* EmulatorInst = nullptr;

extern "C"
{

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
 * Portions Copyright (C) 2017 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#define DEBUG_HZ 0
#define DEBUG_MAIN_CALLS 0
#define DEBUG_KEY 0
#define DEBUG_LOAD_INFO 1

#ifndef _WIN32
std::chrono::steady_clock::time_point s_tp = std::chrono::steady_clock::now();
static std::chrono::milliseconds::rep GetTickCount()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - s_tp).count();
}
#endif

static const char *VERSION_STRING = "fixNES Alpha v1.0.5";
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

//static void nesEmuFdsSetup(uint8_t *src, uint8_t *dst);

static int emuFileType = FTYPE_UNK;
static char emuFileName[1024];
static uint8_t *emuNesROM = NULL;
static uint32_t emuNesROMsize = 0;
static char emuSaveName[1024];
static uint8_t *emuPrgRAM = NULL;
static uint32_t emuPrgRAMsize = 0;
//used externally
uint32_t textureImage[0xF000];
bool nesPause = false;
bool ppuDebugPauseFrame = false;
bool doOverscan = false;
bool nesPAL = false;
bool nesEmuNSFPlayback = false;

//static bool inPause = false;
//static bool inOverscanToggle = false;
//static bool inResize = false;
//static bool inDiskSwitch = false;
//static bool inReset = false;

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
static const uint32_t visibleImg = VISIBLE_DOTS*VISIBLE_LINES*4;
//static uint8_t scaleFactor = 2;
static bool emuSaveEnabled = false;
static bool emuFdsHasSideB = false;

//static uint16_t ppuCycleTimer;
uint32_t cpuCycleTimer;
uint32_t vrc7CycleTimer;
//from input.c
extern uint8_t inValReads[8];
//from mapper.c
extern bool mapperUse78A;
//from m32.c
extern bool m32_singlescreen;

static volatile bool emuRenderFrame = false;
extern uint8_t audioExpansion;

//used externally
bool emuSkipVsync = false;
bool emuSkipFrame = false;

//static uint32_t mCycles = 0;

extern bool fdsSwitch;

bool apuCycleURDE();
uint32_t apuGetMaxBufSize();
uint8_t* ppuGetVRAM();

int audioUpdate()
{
    if (!EmulatorInst)
        return 0;
    return EmulatorInst->audioUpdate();
}

}

namespace urde::MP1
{

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

CNESEmulator::CNESEmulator()
{
    if (EmulatorConstructed)
        Log.report(logvisor::Fatal, "Attempted constructing more than 1 CNESEmulator");
    EmulatorConstructed = true;

    CDvdFile NESEmuFile("NESemuP.rel");
    if (NESEmuFile)
    {
        m_nesEmuPBuf.reset(new u8[0x20000]);
        m_dvdReq = NESEmuFile.AsyncSeekRead(m_nesEmuPBuf.get(), 0x20000, ESeekOrigin::Begin, NESEMUP_ROM_OFFSET);
    }
    else
    {
        Log.report(logvisor::Fatal, "Unable to open NESemuP.rel");
    }
}

void CNESEmulator::InitializeEmulator()
{
    nesPause = false;
    ppuDebugPauseFrame = false;

    puts(VERSION_STRING);
    strcpy(window_title, VERSION_STRING);
    memset(textureImage,0,visibleImg);
    emuFileType = FTYPE_UNK;
    memset(emuFileName,0,1024);
    memset(emuSaveName,0,1024);

    nesPAL = METROID_PAL;
    uint8_t mapper = METROID_MAPPER;
    emuSaveEnabled = METROID_SAVE_ENABLED;
    bool trainer = METROID_TRAINER;
    uint32_t prgROMsize = METROID_PRG_SIZE;
    uint32_t chrROMsize = METROID_CHR_SIZE;
    emuPrgRAMsize = METROID_PRG_RAM_SIZE;
    emuPrgRAM = (uint8_t*)malloc(emuPrgRAMsize);
    uint8_t *prgROM = emuNesROM;
    if(trainer)
    {
        memcpy(emuPrgRAM+0x1000,prgROM,0x200);
        prgROM += 512;
    }
    uint8_t *chrROM = NULL;
    if(chrROMsize)
    {
        chrROM = emuNesROM+prgROMsize;
        if(trainer) chrROM += 512;
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
    if(!mapperInit(mapper, prgROM, prgROMsize, emuPrgRAM, emuPrgRAMsize, chrROM, chrROMsize))
    {
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
    //do one scanline per idle loop
    //ppuCycleTimer = nesPAL ? 5 : 4;
    //mainLoopRuns = nesPAL ? DOTS*ppuCycleTimer : DOTS*ppuCycleTimer;
    //mainLoopPos = mainLoopRuns;

    CGraphics::CommitResources([this](boo::IGraphicsDataFactory::Context& ctx)
    {
        // Nearest-neighbor FTW!
        m_texture = ctx.newDynamicTexture(VISIBLE_DOTS, linesToDraw,
                                          boo::TextureFormat::RGBA8,
                                          boo::TextureClampMode::ClampToEdgeNearest);
        Vert verts[4] =
        {
            {{-1.f, -1.f, 0.f}, {0.f, 1.f}},
            {{-1.f,  1.f, 0.f}, {0.f, 0.f}},
            {{ 1.f, -1.f, 0.f}, {1.f, 1.f}},
            {{ 1.f,  1.f, 0.f}, {1.f, 0.f}},
        };
        m_vbo = ctx.newStaticBuffer(boo::BufferUse::Vertex, verts, sizeof(Vert), 4);
        m_uniBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(Uniform), 1);
        m_shadBind = CNESShader::BuildShaderDataBinding(ctx, m_vbo, m_uniBuf, m_texture);
        return true;
    } BooTrace);

    //double useFreq = 223740;
    double useFreq = apuGetFrequency();
    m_booVoice = CAudioSys::GetVoiceEngine()->allocateNewStereoVoice(useFreq, this);
    m_booVoice->start();
    uint32_t apuBufSz = apuGetMaxBufSize();
    m_audioBufBlock.reset(new u8[apuBufSz * NUM_AUDIO_BUFFERS]);
    for (int i=0 ; i<NUM_AUDIO_BUFFERS ; ++i)
        m_audioBufs[i] = m_audioBufBlock.get() + apuBufSz * i;

    EmulatorInst = this;
}

void CNESEmulator::DeinitializeEmulator()
{
    //printf("\n");
    emuRenderFrame = false;
    m_booVoice->stop();
    m_booVoice.reset();
    apuDeinitBufs();
    if(emuNesROM != NULL)
    {
        if(!nesEmuNSFPlayback && (audioExpansion&EXP_FDS))
        {
            FILE *save = fopen(emuSaveName, "wb");
            if(save)
            {
                if(emuFdsHasSideB)
                    fwrite(emuNesROM,1,0x20000,save);
                else
                    fwrite(emuNesROM,1,0x10000,save);
                fclose(save);
            }
        }
    }
    if(emuPrgRAM != NULL)
    {
        if(emuSaveEnabled)
        {
            FILE *save = fopen(emuSaveName, "wb");
            if(save)
            {
                fwrite(emuPrgRAM,1,emuPrgRAMsize,save);
                fclose(save);
            }
        }
        free(emuPrgRAM);
    }
    emuPrgRAM = NULL;
    //printf("Bye!\n");

    EmulatorInst = nullptr;
}

CNESEmulator::~CNESEmulator()
{
    if (m_dvdReq)
        m_dvdReq->PostCancelRequest();
    if (EmulatorInst)
        DeinitializeEmulator();
    if (emuNesROM)
    {
        free(emuNesROM);
        emuNesROM = nullptr;
    }
    EmulatorConstructed = false;
}

int CNESEmulator::audioUpdate()
{
    int origProcBufs = m_procBufs;

    uint8_t *data = apuGetBuf();
    if(data != NULL && m_procBufs)
    {
        --m_procBufs;
        memmove(m_audioBufs[m_headBuf], data, apuGetMaxBufSize());
        //printf("PUSH\n");
        ++m_headBuf;
        if (m_headBuf == NUM_AUDIO_BUFFERS)
            m_headBuf = 0;
    }

    //if (!origProcBufs)
        //printf("OVERRUN\n");

    return origProcBufs;
}

static constexpr size_t AudioFrameSz = 2 * sizeof(int16_t);

size_t CNESEmulator::supplyAudio(boo::IAudioVoice& voice, size_t frames, int16_t* data)
{
    size_t remFrames = frames;
    while (remFrames)
    {
        if (m_posInBuf == apuGetMaxBufSize())
        {
            ++m_tailBuf;
            if (m_tailBuf == NUM_AUDIO_BUFFERS)
                m_tailBuf = 0;
            m_posInBuf = 0;
            ++m_procBufs;
            //printf("POP\n");
        }

        if (m_procBufs == NUM_AUDIO_BUFFERS)
        {
            memset(data, 0, remFrames * AudioFrameSz);
            //printf("UNDERRUN\n");
            return frames;
        }

        size_t copySz = std::min(apuGetMaxBufSize() - m_posInBuf, remFrames * AudioFrameSz);
        memmove(data, m_audioBufs[m_tailBuf] + m_posInBuf, copySz);
        data += copySz / sizeof(int16_t);
        m_posInBuf += copySz;
        remFrames -= copySz / AudioFrameSz;
    }
    return frames;
}

#define CATCHUP_SKIP 0
#if CATCHUP_SKIP
static int catchupFrames = 0;
#endif

void CNESEmulator::NesEmuMainLoop(bool forceDraw)
{
    //int start = GetTickCount();
    int loopCount = 0;
    do
    {
        if(emuRenderFrame || nesPause)
        {
#if DEBUG_MAIN_CALLS
            emuMainTimesSkipped++;
#endif
            //printf("LC RENDER: %d\n", loopCount);
            m_texture->load(textureImage, visibleImg);
            emuRenderFrame = false;
#if CATCHUP_SKIP
            if (catchupFrames)
                --catchupFrames;
            else
#endif
                break;
        }
        ++loopCount;

        //main CPU clock
        if(!cpuCycle())
            exit(EXIT_SUCCESS);
        //run graphics
        ppuCycle();
        //run audio
        apuCycle();
        //mapper related irqs
        mapperCycle();
        //mCycles++;
        if(ppuDrawDone())
        {
            //printf("%i\n",mCycles);
            //mCycles = 0;
#ifndef __LIBRETRO__
            emuRenderFrame = true;
#if 0
            if(fm2playRunning())
                fm2playUpdate();
#endif
#if (WINDOWS_BUILD && DEBUG_HZ)
            emuTimesCalled++;
            DWORD end = GetTickCount();
            emuTotalElapsed += end - emuFrameStart;
            if(emuTotalElapsed >= 1000)
            {
                printf("\r%iHz   ", emuTimesCalled);
                emuTimesCalled = 0;
                emuTotalElapsed = 0;
            }
            emuFrameStart = end;
#endif
            //update audio before drawing
            if(!apuUpdate())
                break;
            //glutPostRedisplay();
#if 0
            if(ppuDebugPauseFrame)
            {
                ppuDebugPauseFrame = false;
                nesPause = true;
            }
#endif
#endif
            if(nesEmuNSFPlayback)
                nsfVsync();
            continue;
        }

#if 0
        if (!forceDraw && (loopCount % 10000) == 0 && GetTickCount() - start >= 14)
        {
#if CATCHUP_SKIP
            if (catchupFrames < 50)
                ++catchupFrames;
#endif
            break;
        }
#endif
    }
    while(true);

#if 0
    int end = GetTickCount();
    printf("%dms %d %d\n", end - start, loopCount, m_procBufs);
#endif

#if DEBUG_MAIN_CALLS
    emuMainTimesCalled++;
    int end = GetTickCount();
    //printf("%dms\n", end - start);
    emuMainTotalElapsed += end - emuMainFrameStart;
    if(emuMainTotalElapsed >= 1000)
    {
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

void CNESEmulator::DecompressROM(u8* dataIn, u8* dataOut, u32 dataOutLen, u8 descrambleSeed,
                                 u32 checkDataLen, u32 checksumMagic)
{
    for (int i=0 ; i<256 ; ++i)
    {
        descrambleSeed += dataIn[i];
        dataIn[i] = descrambleSeed;
    }

    for (int i=0 ; i<128 ; ++i)
        std::swap(dataIn[255 - i], dataIn[i]);

    struct BitstreamState
    {
        u32 x0_remBits = 0;
        u32 x4_lastByte;
        u8* x8_ptr;
        BitstreamState(u8* ptr) : x8_ptr(ptr) {}
    } bState = {dataIn + 256};

    u8* dataOutAlpha = dataOut;
    while (dataOutLen != 0)
    {
        u32 r12 = 0;

        if (bState.x0_remBits == 0)
        {
            bState.x4_lastByte = *bState.x8_ptr;
            bState.x8_ptr += 1;
            bState.x0_remBits = 8;
        }

        u32 r11 = r12 << 1;
        if (bState.x4_lastByte & 0x80)
            r11 |= 1;

        bState.x4_lastByte <<= 1;
        bState.x0_remBits -= 1;

        if (r11 == 1)
        {
            r12 = 0;

            for (int i=0 ; i<4 ; ++i)
            {
                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r12 <<= 1;
                if (bState.x4_lastByte & 0x80)
                    r12 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;

                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r12 <<= 1;
                if (bState.x4_lastByte & 0x80)
                    r12 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;
            }

            *dataOutAlpha++ = dataIn[r12 + 73];
        }
        else
        {
            r12 = 0;

            if (bState.x0_remBits == 0)
            {
                bState.x4_lastByte = *bState.x8_ptr;
                bState.x8_ptr += 1;
                bState.x0_remBits = 8;
            }

            r11 = r12 << 1;
            if (bState.x4_lastByte & 0x80)
                r11 |= 1;

            bState.x4_lastByte <<= 1;
            bState.x0_remBits -= 1;

            if (r11 == 1)
            {
                r12 = 0;

                for (int i=0 ; i<3 ; ++i)
                {
                    if (bState.x0_remBits == 0)
                    {
                        bState.x4_lastByte = *bState.x8_ptr;
                        bState.x8_ptr += 1;
                        bState.x0_remBits = 8;
                    }

                    r12 <<= 1;
                    if (bState.x4_lastByte & 0x80)
                        r12 |= 1;

                    bState.x4_lastByte <<= 1;
                    bState.x0_remBits -= 1;

                    if (bState.x0_remBits == 0)
                    {
                        bState.x4_lastByte = *bState.x8_ptr;
                        bState.x8_ptr += 1;
                        bState.x0_remBits = 8;
                    }

                    r12 <<= 1;
                    if (bState.x4_lastByte & 0x80)
                        r12 |= 1;

                    bState.x4_lastByte <<= 1;
                    bState.x0_remBits -= 1;
                }

                *dataOutAlpha++ = dataIn[r12 + 9];
            }
            else
            {
                r12 = 0;

                if (bState.x0_remBits == 0)
                {
                    bState.x4_lastByte = *bState.x8_ptr;
                    bState.x8_ptr += 1;
                    bState.x0_remBits = 8;
                }

                r11 = r12 << 1;
                if (bState.x4_lastByte & 0x80)
                    r11 |= 1;

                bState.x4_lastByte <<= 1;
                bState.x0_remBits -= 1;

                if (r11 == 1)
                {
                    r12 = 0;

                    for (int i=0 ; i<3 ; ++i)
                    {
                        if (bState.x0_remBits == 0)
                        {
                            bState.x4_lastByte = *bState.x8_ptr;
                            bState.x8_ptr += 1;
                            bState.x0_remBits = 8;
                        }

                        r12 <<= 1;
                        if (bState.x4_lastByte & 0x80)
                            r12 |= 1;

                        bState.x4_lastByte <<= 1;
                        bState.x0_remBits -= 1;
                    }

                    *dataOutAlpha++ = dataIn[r12 + 1];
                }
                else
                {
                    *dataOutAlpha++ = dataIn[0];
                }
            }
        }

        --dataOutLen;
    }

    u32 tmpWord = 0;
    for (int i=0 ; i<checkDataLen ; ++i)
    {
        tmpWord ^= dataOut[i];
        for (int j=0 ; j<8 ; ++j)
        {
            if (tmpWord & 0x1)
                tmpWord = (tmpWord >> 1) ^ checksumMagic;
            else
                tmpWord = (tmpWord >> 1);
        }
    }

    dataOut[checkDataLen - 1] = (tmpWord >> 8) & 0xff;
    dataOut[checkDataLen - 2] = tmpWord & 0xff;
}

void CNESEmulator::ProcessUserInput(const CFinalInput& input, int)
{
    if (input.ControllerIdx() != 0)
        return;

    if (GetPasswordEntryState() != EPasswordEntryState::NotPasswordScreen)
    {
        // Don't swap A/B
        inValReads[BUTTON_A] = input.DA();
        inValReads[BUTTON_B] = input.DB();
    }
    else
    {
        // Prime controls (B jumps, A shoots)
        inValReads[BUTTON_B] = input.DA() | input.DY();
        inValReads[BUTTON_A] = input.DB() | input.DX();
    }

    inValReads[BUTTON_UP] = input.DDPUp() | input.DLAUp();
    inValReads[BUTTON_DOWN] = input.DDPDown() | input.DLADown();
    inValReads[BUTTON_LEFT] = input.DDPLeft() | input.DLALeft();
    inValReads[BUTTON_RIGHT] = input.DDPRight() | input.DLARight();
    inValReads[BUTTON_SELECT] = input.DZ();
    inValReads[BUTTON_START] = input.DStart();
}

bool CNESEmulator::CheckForGameOver(const u8* vram, u8* passwordOut)
{
    // "PASS WORD"
    if (memcmp(vram + 0x14B, "\x19\xa\x1c\x1c\xff\x20\x18\x1b\xd", 9))
        return false;

    int chOff = 0;
    int encOff = 0;
    u8 pwOut[18];
    for (int i=0 ; i<24 ; ++i)
    {
        u8 chName = vram[0x1A9 + chOff];
        ++chOff;
        if (chOff == 0x6 || chOff == 0x46)
            ++chOff; // mid-line space
        else if (chOff == 0xd)
            chOff = 64; // 2nd line

        if (chName > 0x3f)
            return false;

        switch (i & 0x3)
        {
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

CNESEmulator::EPasswordEntryState CNESEmulator::CheckForPasswordEntryScreen(const u8* vram)
{
    // "PASS WORD PLEASE"
    if (memcmp(vram + 0x88, "\x19\xa\x1c\x1c\xff\x20\x18\x1b\xd\xff\x19\x15\xe\xa\x1c\xe", 16))
        return EPasswordEntryState::NotPasswordScreen;

    for (int i=0 ; i<13 ; ++i)
        if (vram[0x109 + i] < 0x40 || vram[0x149 + i] < 0x40)
            return EPasswordEntryState::Entered;

    return EPasswordEntryState::NotEntered;
}

bool CNESEmulator::SetPasswordIntoEntryScreen(u8* vram, u8* wram, const u8* password)
{
    if (CheckForPasswordEntryScreen(vram) != EPasswordEntryState::NotEntered)
        return false;

    int i;
    for (i=0 ; i<18 ; ++i)
        if (password[i])
            break;
    if (i == 18)
        return false;

    int encOff = 0;
    int chOff = 0;
    u32 lastWord = 0;
    for (i=0 ; i<24 ; ++i)
    {
        switch (i & 0x3)
        {
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

void CNESEmulator::Update()
{
    if (!EmulatorInst)
    {
        if (m_dvdReq && m_dvdReq->IsComplete())
        {
            m_dvdReq.reset();
            emuNesROMsize = 0x20000;
            emuNesROM = (uint8_t*)malloc(emuNesROMsize);
            DecompressROM(m_nesEmuPBuf.get(), emuNesROM);
            m_nesEmuPBuf.reset();
            InitializeEmulator();
        }
    }
    else
    {
        if (nesPause)
        {
            DeinitializeEmulator();
            InitializeEmulator();
            return;
        }

        bool gameOver = CheckForGameOver(ppuGetVRAM(), x21_passwordFromNES);
        x34_passwordEntryState = CheckForPasswordEntryScreen(ppuGetVRAM());
        if (x34_passwordEntryState == EPasswordEntryState::NotEntered && x38_passwordPending)
        {
            SetPasswordIntoEntryScreen(ppuGetVRAM(), emuPrgRAM, x39_passwordToNES);
            x38_passwordPending = false;
        }
        if (gameOver && !x20_gameOver)
            for (int i=0 ; i<3 ; ++i) // Three draw loops to ensure password display
                NesEmuMainLoop(true);
        else
            NesEmuMainLoop();
        x20_gameOver = gameOver;
    }
}

static const float NESAspect = VISIBLE_DOTS / float(VISIBLE_LINES);

void CNESEmulator::Draw(const zeus::CColor& mulColor, bool filtering)
{
    if (!EmulatorInst)
        return;

    float aspect = g_Viewport.x8_width / float(g_Viewport.xc_height);
    float widthFac = NESAspect / aspect;

    Uniform uniform = {zeus::CMatrix4f{}, mulColor};
    uniform.m_matrix[0][0] = widthFac;
    m_uniBuf->load(&uniform, sizeof(Uniform));

    CGraphics::SetShaderDataBinding(m_shadBind);
    CGraphics::DrawArray(0, 4);
}

void CNESEmulator::LoadPassword(const u8* state)
{
    memmove(x39_passwordToNES, state, 18);
    x38_passwordPending = true;
}

}
