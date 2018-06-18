/*
 * Copyright (C) 2017 FIX94
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

/*
 * URDE modification to generate RGBA8 framebuffer rather than BGR565
 */

#include <stdio.h>
#include <stdbool.h>
#include <inttypes.h>
#include <string.h>
#include "fixNES/mapper.h"
#include "fixNES/ppu.h"

//certain optimizations were taken from nestopias ppu code,
//thanks to the people from there for all that

//sprite byte 2
#define PPU_SPRITE_PRIO (1<<5)
#define PPU_SPRITE_FLIP_X (1<<6)
#define PPU_SPRITE_FLIP_Y (1<<7)

//2000
#define PPU_INC_AMOUNT (1<<2)
#define PPU_SPRITE_ADDR (1<<3)
#define PPU_BACKGROUND_ADDR (1<<4)
#define PPU_SPRITE_8_16 (1<<5)
#define PPU_FLAG_NMI (1<<7)

//2001
#define PPU_GRAY (1<<0)
#define PPU_BG_8PX (1<<1)
#define PPU_SPRITE_8PX (1<<2)
#define PPU_BG_ENABLE (1<<3)
#define PPU_SPRITE_ENABLE (1<<4)

//2002
#define PPU_FLAG_OVERFLOW (1<<5)
#define PPU_FLAG_SPRITEZERO (1<<6)
#define PPU_FLAG_VBLANK (1<<7)

#define DOTS 341

#define VISIBLE_DOTS 256
#define VISIBLE_LINES 240

#define PPU_VRAM_HORIZONTAL_MASK 0x41F
#define PPU_VRAM_VERTICAL_MASK (~PPU_VRAM_HORIZONTAL_MASK)

#define PPU_DEBUG_ULTRA 0

#define PPU_DEBUG_VSYNC 0

//set or used externally
bool ppu4Screen = false;
bool ppu816Sprite = false;
bool ppuInFrame = false;
bool ppuScanlineDone = false;
uint8_t ppuDrawnXTile = 0;

//from main.c
extern uint32_t textureImage[0xF000];
extern bool nesPause;
extern bool ppuDebugPauseFrame;
extern bool doOverscan;

static uint8_t ppuDoSprites(uint8_t color, uint16_t dot);

// BMF Final 2
static const uint8_t PPU_Pal[192] =
    {
        0x52, 0x52, 0x52, 0x00, 0x00, 0x80, 0x08, 0x00, 0x8A, 0x2C, 0x00, 0x7E, 0x4A, 0x00, 0x4E, 0x50, 0x00, 0x06, 0x44, 0x00, 0x00, 0x26, 0x08, 0x00,
        0x0A, 0x20, 0x00, 0x00, 0x2E, 0x00, 0x00, 0x32, 0x00, 0x00, 0x26, 0x0A, 0x00, 0x1C, 0x48, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xA4, 0xA4, 0xA4, 0x00, 0x38, 0xCE, 0x34, 0x16, 0xEC, 0x5E, 0x04, 0xDC, 0x8C, 0x00, 0xB0, 0x9A, 0x00, 0x4C, 0x90, 0x18, 0x00, 0x70, 0x36, 0x00,
        0x4C, 0x54, 0x00, 0x0E, 0x6C, 0x00, 0x00, 0x74, 0x00, 0x00, 0x6C, 0x2C, 0x00, 0x5E, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0x4C, 0x9C, 0xFF, 0x7C, 0x78, 0xFF, 0xA6, 0x64, 0xFF, 0xDA, 0x5A, 0xFF, 0xF0, 0x54, 0xC0, 0xF0, 0x6A, 0x56, 0xD6, 0x86, 0x10,
        0xBA, 0xA4, 0x00, 0x76, 0xC0, 0x00, 0x46, 0xCC, 0x1A, 0x2E, 0xC8, 0x66, 0x34, 0xC2, 0xBE, 0x3A, 0x3A, 0x3A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xFF, 0xFF, 0xFF, 0xB6, 0xDA, 0xFF, 0xC8, 0xCA, 0xFF, 0xDA, 0xC2, 0xFF, 0xF0, 0xBE, 0xFF, 0xFC, 0xBC, 0xEE, 0xFA, 0xC2, 0xC0, 0xF2, 0xCC, 0xA2,
        0xE6, 0xDA, 0x92, 0xCC, 0xE6, 0x8E, 0xB8, 0xEE, 0xA2, 0xAE, 0xEA, 0xBE, 0xAE, 0xE8, 0xE2, 0xB0, 0xB0, 0xB0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    } ;

static const uint8_t ppuRunCyclesNTSC[5] = { 3, 3, 3, 3, 3 };
static const uint8_t ppuRunCyclesPAL[5] = { 3, 3, 3, 3, 4 };
static const uint8_t ppuOddArrNTSC[2] = { 0, 1 };
static const uint8_t ppuOddArrPAL[2] = { 0, 0 };

static struct
{
    const uint8_t *Pal;
    const uint8_t *RunCycles;
    const uint8_t *OddArr;
    uint32_t BGRLUT[0x200];
    uint8_t TILELUT[0x400][4];
    uint16_t PALRAM2[0x20];
    uint16_t NameTbl[4];
    uint16_t NameTblBak[4];
    uint8_t VRAM[0x1000];
    uint8_t OAM[0x100];
    uint8_t OAM2[0x20];
    uint8_t PALRAM[0x20];
    uint8_t Sprites[8][12];
    uint8_t BGTiles[16];
    uint8_t Reg[8];
    uint8_t Count;
    uint8_t OddNum;
    uint16_t curLine;
    uint16_t LinesTotal;
    uint16_t PreRenderLine;
    uint16_t curDot;
    uint16_t VramAddr;
    uint16_t TmpVramAddr;
    uint8_t ToDraw;
    uint8_t VramReadBuf;
    uint8_t OAMpos;
    uint8_t OAM2pos;
    uint8_t OAMcpPos;
    uint8_t OAMzSpritePos;
    uint8_t FineXScroll;
    uint8_t SpriteOAM2Pos;
    uint8_t SpriteTilePos;
    uint8_t BGRegA;
    uint8_t BGRegB;
    uint8_t BGAttribReg;
    uint8_t BGIndex;
    uint8_t BytesCopied;
    uint8_t lastVal;
    uint8_t CurSpriteLn;
    uint8_t CurSpriteIndex;
    uint8_t CurSpriteByte2;
    uint8_t CurSpriteByte3;
    uint8_t TmpOAMVal;
    bool NextHasZSprite;
    bool FrameDone;
    bool TmpWrite;
    bool NMIallowed;
    bool NMITriggered;
    bool VBlankFlagCleared;
    bool VBlankClearCycle;
    bool CurNMIStat;
    bool OddFrame;
    bool ReadReg2;
    bool OAMoverflow;
    bool SearchY;
    bool LineOverflow;
    bool DoOverscan;
    bool BGEnable;
    bool SprEnable;
} ppu;

extern bool nesPAL;
void ppuInit()
{
    memset(ppu.PALRAM2,0,0x40);
    memset(ppu.NameTbl,0,8);
    memset(ppu.NameTblBak,0,8);
    memset(ppu.VRAM,0,0x1000);
    memset(ppu.OAM,0,0x100);
    memset(ppu.OAM2,0xFF,0x20);
    memset(ppu.PALRAM,0,0x20);
    int32_t i;
    for(i = 0; i < 8; i++)
        memset(ppu.Sprites[i],0,12);
    memset(ppu.BGTiles,0,0x10);
    memset(ppu.Reg,0,8);
    ppu.Pal = PPU_Pal;
    //ppuCycles = 0;
    //start out being in vblank
    ppu.Reg[2] |= PPU_FLAG_VBLANK;
    ppu.LinesTotal = nesPAL ? 312 : 262;
    ppu.PreRenderLine = ppu.LinesTotal - 1;
    ppu.curLine = ppu.LinesTotal - 11;
    ppu.curDot = 0;
    ppu.VramAddr = 0;
    ppu.TmpVramAddr = 0;
    ppu.ToDraw = 0;
    ppu.VramReadBuf = 0;
    ppu.OAMpos = 0;
    ppu.OAM2pos = 0;
    ppu.OAMcpPos = 0;
    ppu.OAMzSpritePos = 0;
    ppu.FineXScroll = 0;
    ppu.SpriteOAM2Pos = 0;
    ppu.SpriteTilePos = 0;
    ppu.BGRegA = 0;
    ppu.BGRegB = 0;
    ppu.BGAttribReg = 0;
    ppu.BGIndex = 0;
    ppu.BytesCopied = 0;
    ppu.lastVal = 0;
    ppu.CurSpriteLn = 0;
    ppu.CurSpriteIndex = 0;
    ppu.CurSpriteByte2 = 0;
    ppu.CurSpriteByte3 = 0;
    ppu.TmpOAMVal = 0xFF;
    ppu.NextHasZSprite = false;
    ppu.FrameDone = false;
    ppu.TmpWrite = false;
    ppu.NMIallowed = false;
    ppu.NMITriggered = false;
    ppu.VBlankFlagCleared = false;
    ppu.VBlankClearCycle = false;
    ppu.CurNMIStat = false;
    ppu.OddFrame = false;
    ppu.ReadReg2 = false;
    ppu.OAMoverflow = false;
    ppu.SearchY = false;
    ppu.LineOverflow = false;
    ppu.DoOverscan = false;
    ppu.BGEnable = false;
    ppu.SprEnable = false;
    //generate full BGR LUT
    uint8_t rtint = nesPAL ? (1<<6) : (1<<5);
    uint8_t gtint = nesPAL ? (1<<5) : (1<<6);
    uint8_t btint = (1<<7);
    for(i = 0; i < 0x200; i++)
    {
        uint8_t palpos = (i&0x3F)*3;
        uint8_t r = ppu.Pal[palpos];
        uint8_t g = ppu.Pal[palpos+1];
        uint8_t b = ppu.Pal[palpos+2];
        if((i & 0xF) <= 0xD)
        {
            //reduce red
            if((i>>1) & btint) r = (uint8_t)(((float)r)*0.75f);
            if((i>>1) & gtint) r = (uint8_t)(((float)r)*0.75f);
            //reduce green
            if((i>>1) & rtint) g = (uint8_t)(((float)g)*0.75f);
            if((i>>1) & btint) g = (uint8_t)(((float)g)*0.75f);
            //reduce blue
            if((i>>1) & rtint) b = (uint8_t)(((float)b)*0.75f);
            if((i>>1) & gtint) b = (uint8_t)(((float)b)*0.75f);
        }
        //save new color into LUT
        ppu.BGRLUT[i] =
            (0xFFu<<24) //Alpha
            | (b<<16) //Blue
            | (g<<8) //Green
            | r; //Red
    }
    //tile LUT from nestopia
    for (i = 0; i < 0x400; ++i)
    {
        ppu.TILELUT[i][0] = (i & 0xC0) ? (i >> 6 & 0xC) | (i >> 6 & 0x3) : 0;
        ppu.TILELUT[i][1] = (i & 0x30) ? (i >> 6 & 0xC) | (i >> 4 & 0x3) : 0;
        ppu.TILELUT[i][2] = (i & 0x0C) ? (i >> 6 & 0xC) | (i >> 2 & 0x3) : 0;
        ppu.TILELUT[i][3] = (i & 0x03) ? (i >> 6 & 0xC) | (i >> 0 & 0x3) : 0;
    }
    ppu.Count = 0;
    ppu.RunCycles = nesPAL ? ppuRunCyclesPAL : ppuRunCyclesNTSC;
    ppu.OddNum = 0;
    ppu.OddArr = nesPAL ? ppuOddArrPAL : ppuOddArrNTSC;
}

extern uint8_t m5_exMode;
extern uint8_t m5exGetAttrib(uint16_t addr);
static inline uint16_t ppuGetVramTbl(uint16_t tblStart)
{
    return ppu.NameTbl[(tblStart>>10)&3];
}
extern bool nesEmuNSFPlayback;

//tile load from nestopia
static void loadTiles()
{
    uint8_t* src[] =
        {
            ppu.TILELUT[ppu.BGRegA | (ppu.BGAttribReg & 3) << 8],
            ppu.TILELUT[ppu.BGRegB | (ppu.BGAttribReg & 3) << 8]
        };

    uint8_t* dst = ppu.BGTiles+ppu.BGIndex;
    ppu.BGIndex ^= 8;

    dst[0] = src[0][0];
    dst[1] = src[1][0];
    dst[2] = src[0][1];
    dst[3] = src[1][1];
    dst[4] = src[0][2];
    dst[5] = src[1][2];
    dst[6] = src[0][3];
    dst[7] = src[1][3];
}

static void grabNextSpriteTilesP1()
{
    ppu.TmpOAMVal = ppu.OAM2[ppu.SpriteOAM2Pos++];
    ppu.CurSpriteLn = ppu.TmpOAMVal;
}
static void grabNextSpriteTilesP2()
{
    ppu.TmpOAMVal = ppu.OAM2[ppu.SpriteOAM2Pos++];
    ppu.CurSpriteIndex = ppu.TmpOAMVal;
}
static void grabNextSpriteTilesP3()
{
    ppu.TmpOAMVal = ppu.OAM2[ppu.SpriteOAM2Pos++];
    ppu.CurSpriteByte2 = ppu.TmpOAMVal;
}
static void grabNextSpriteTilesP4()
{
    ppu.TmpOAMVal = ppu.OAM2[ppu.SpriteOAM2Pos++];
    ppu.CurSpriteByte3 = ppu.TmpOAMVal;
}
static void grabNextSpriteTilesP5(uint16_t line)
{
    uint8_t cSprLn = ppu.CurSpriteLn, cSprInd = ppu.CurSpriteIndex, cSprB2 = ppu.CurSpriteByte2;
    uint8_t cSpriteAdd = 0; //used to select which 8 by 16 tile
    bool spr816 = ppu.Reg[0] & PPU_SPRITE_8_16;
    uint8_t cSpriteY = (line - cSprLn)&(spr816 ? 15 : 7);
    if(cSpriteY > 7) //8 by 16 select
    {
        cSpriteAdd = 16;
        cSpriteY &= 7;
    }
    uint16_t chrROMSpriteAdd = 0;
    if(spr816)
    {
        chrROMSpriteAdd = ((cSprInd & 1) << 12);
        cSprInd &= ~1;
    }
    else if(ppu.Reg[0] & PPU_SPRITE_ADDR)
        chrROMSpriteAdd = 0x1000;
    if(cSprB2 & PPU_SPRITE_FLIP_Y)
    {
        cSpriteY ^= 7;
        if(spr816)
            cSpriteAdd ^= 16; //8 by 16 select
    }
    /* write processed values into internal draw buffer */
    mapperChrMode = 1;
    uint8_t p0 = mapperChrGet8(((chrROMSpriteAdd+(cSprInd<<4)+cSpriteY+cSpriteAdd)&0xFFF) | chrROMSpriteAdd);
    uint8_t p1 = mapperChrGet8(((chrROMSpriteAdd+(cSprInd<<4)+cSpriteY+8+cSpriteAdd)&0xFFF) | chrROMSpriteAdd);
    if ((p0 | p1) && (cSprLn < VISIBLE_LINES)) //sprite contains data and is on a valid line, so process
    {
        //pixels
        uint8_t sprTilePos = ppu.SpriteTilePos;
        uint8_t a = (cSprB2 & PPU_SPRITE_FLIP_X) ? 7 : 0;
        uint16_t p = (p0 >> 1 & 0x0055) | (p1 << 0 & 0x00AA) | (p0 << 8 & 0x5500) | (p1 << 9 & 0xAA00);
        ppu.Sprites[sprTilePos][( a^=6 )] = ( p       ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=2 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=6 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=2 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=7 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=2 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=6 )] = ( p >>= 2 ) & 0x3;
        ppu.Sprites[sprTilePos][( a^=2 )] = ( p >>= 2 );
        //x position
        ppu.Sprites[sprTilePos][8] = ppu.CurSpriteByte3;
        //is zero sprite (compare to 4 because OAM2 was just read, 4 is next position)
        ppu.Sprites[sprTilePos][9] = (ppu.SpriteOAM2Pos == 4 && ppu.NextHasZSprite) ? 0x3 : 0x0;
        //priority
        ppu.Sprites[sprTilePos][10] = (cSprB2 & PPU_SPRITE_PRIO) ? 0x3 : 0x0;
        //palette
        ppu.Sprites[sprTilePos][11] = (cSprB2&3)<<2 | 0x10;
        //increase pos
        ppu.SpriteTilePos = sprTilePos+1;
    }
}

static void updateBGTileAddress()
{
    if((ppu.VramAddr & 0x1F) == 0x1F)
        ppu.VramAddr ^= 0x41F;
    else
        ppu.VramAddr++;
}
static void updateBGHoriAddress()
{
    ppu.VramAddr = (ppu.VramAddr & (~PPU_VRAM_HORIZONTAL_MASK)) | (ppu.TmpVramAddr & PPU_VRAM_HORIZONTAL_MASK);
}
static void updateBGVertAddress()
{
    ppu.VramAddr = (ppu.VramAddr & (~PPU_VRAM_VERTICAL_MASK)) | (ppu.TmpVramAddr & PPU_VRAM_VERTICAL_MASK);
}
static void updateBGYAddress()
{
    /* update Y position for writes */
    if((ppu.VramAddr & 0x7000) != (7<<12))
        ppu.VramAddr += (1<<12);
    else switch(ppu.VramAddr & 0x3E0)
        {
        default:      ppu.VramAddr = (ppu.VramAddr & 0xFFF) + (1 << 5); break;
        case (29<<5): ppu.VramAddr ^= 0x800;
        case (31<<5): ppu.VramAddr &= 0xC1F; break;
        }
}

static void updateBGRegsA(uint16_t dot)
{
    /* MMC5 Scroll Related */
    if(dot == 320)
        ppuDrawnXTile = 0;
    else
        ppuDrawnXTile++;
    if(m5_exMode == 1)
    {
        /* MMC5 Ex Mode 1 has different Attribute for every Tile */
        ppu.BGAttribReg = m5exGetAttrib(ppu.VramAddr);
    }
    else
    {
        uint16_t cPpuTbl = ppuGetVramTbl(ppu.VramAddr);
        /* Select new BG Background Attribute */
        uint8_t cAttrib = ((ppu.VramAddr>>4)&0x38) | ((ppu.VramAddr>>2)&7);
        uint16_t attributeAddr = cPpuTbl | (0x3C0 | cAttrib);
        ppu.BGAttribReg = mapperVramGet8(attributeAddr) >> ((ppu.VramAddr & 0x2) | (ppu.VramAddr >> 4 & 0x4));
    }
}
static void updateBGRegsB()
{
    uint16_t cPpuTbl = ppuGetVramTbl(ppu.VramAddr);
    /* Select new BG Tiles */
    uint16_t chrROMBG = (ppu.Reg[0] & PPU_BACKGROUND_ADDR) ? 0x1000 : 0;
    uint16_t workAddr = cPpuTbl | (ppu.VramAddr & 0x3FF);
    uint8_t curBGtileReg = mapperVramGet8(workAddr);
    uint8_t curTileY = (ppu.VramAddr>>12)&7;
    uint16_t curBGTile = chrROMBG+(curBGtileReg<<4)+curTileY;
    mapperChrMode = 0;
    uint8_t tmp = mapperChrGet8(curBGTile);
    ppu.BGRegB = tmp >> 0 & 0x55; ppu.BGRegA = tmp >> 1 & 0x55;
    tmp = mapperChrGet8(curBGTile+8);
    ppu.BGRegA |= tmp << 0 & 0xAA; ppu.BGRegB |= tmp << 1 & 0xAA;
}

static void spriteEvalInit()
{
    ppu.OAMzSpritePos = ppu.OAMpos;
    //OAM Bug in 2C02
    if(ppu.OAMpos & 0xF8)
        memcpy(ppu.OAM,ppu.OAM + (ppu.OAMpos & 0xF8), 8);
}

static void spriteEvalA()
{
    ppu.TmpOAMVal = ppu.OAM[(ppu.OAMpos+ppu.OAMcpPos)&0xFF];
    //printf("%i %i %i %02x\n", ppu.OAMpos,  ppu.OAMcpPos, (ppu.OAMpos+ppu.OAMcpPos)&0xFF, ppu.TmpOAMVal);
}

static void spriteEvalB(uint16_t line)
{
    if(!ppu.OAMoverflow)
    {
        if(ppu.SearchY)
        {
            uint8_t cSpriteLn = ppu.TmpOAMVal;
            uint8_t cSpriteAdd = (ppu.Reg[0] & PPU_SPRITE_8_16) ? 16 : 8;
            if(cSpriteLn <= line && (cSpriteLn+cSpriteAdd) > line)
            {
                ppu.SearchY = false;
                if(ppu.OAM2pos == 0 && ppu.OAMpos == ppu.OAMzSpritePos)
                    ppu.NextHasZSprite = true;
                if(ppu.OAM2pos != 0x20)
                {
                    ppu.OAM2[ppu.OAM2pos+ppu.OAMcpPos] = ppu.TmpOAMVal;
                    //printf("Copying sprite with line %i at line %i pos oam %i oam2 %i\n", cSpriteLn, ppu.curLine, ppu.OAMpos, ppu.OAM2pos);
                }
                else
                {
                    //if(!(PPU_Reg[2] & PPU_FLAG_OVERFLOW) && !ppu.SpriteOverflow)
                    //	printf("Overflow with line %i at line %i pos oam %i oam2 %i\n", cSpriteLn, ppu.curLine, ppu.OAMpos, ppu.OAM2pos);
                    ppu.Reg[2] |= PPU_FLAG_OVERFLOW;
                    ppu.LineOverflow = true;
                }
                ppu.BytesCopied++;
                if(ppu.OAMcpPos < 3)
                    ppu.OAMcpPos++;
                else
                {
                    ppu.OAMcpPos = 0;
                    ppu.OAMpos += 4;
                    if(ppu.OAMpos == 0)
                        ppu.OAMoverflow = true;
                }
            }
            else //no matching sprite, increase N
            {
                ppu.OAMpos += 4;
                if(ppu.OAM2pos == 0x20 && !ppu.LineOverflow) /* nes sprite overflow bug */
                {
                    if(ppu.OAMcpPos < 3)
                        ppu.OAMcpPos++;
                    else
                        ppu.OAMcpPos = 0;
                }
                if(ppu.OAMpos == 0)
                    ppu.OAMoverflow = true;
            }
        }
        else
        {
            if(ppu.OAM2pos != 0x20)
                ppu.OAM2[ppu.OAM2pos+ppu.OAMcpPos] = ppu.TmpOAMVal;
            if(ppu.OAMcpPos < 3)
                ppu.OAMcpPos++;
            else
            {
                ppu.OAMcpPos = 0;
                ppu.OAMpos += 4;
                if(ppu.OAMpos == 0)
                    ppu.OAMoverflow = true;
            }
            if(ppu.BytesCopied < 3) //we still have to read from this
                ppu.BytesCopied++;
            else //Back to next sprite
            {
                if(ppu.OAM2pos != 0x20)
                    ppu.OAM2pos += 4;
                ppu.BytesCopied = 0;
                ppu.OAMcpPos = 0;
                ppu.SearchY = true;
                if(ppu.LineOverflow) //overflow sprite copied, end
                    ppu.OAMoverflow = true;
            }
        }
    }
    else
        ppu.OAMpos += 4;
    if(ppu.OAM2pos == 0x20)
        ppu.TmpOAMVal = ppu.OAM2[0];
}

static void clearSpriteWorkRegs()
{
    ppu.OAMcpPos = 0;
    ppu.BytesCopied = 0;
    ppu.OAMoverflow = false;
    ppu.SearchY = true;
    ppu.LineOverflow = false;
}

static void resetOAMPos()
{
    ppu.OAMpos = 0;
}

static void ppuCheckLine241(uint16_t dot)
{
    /* VBlank start at first dot after post-render line */
    /* Though results are better when starting it a bit later */
    if(dot == 2)
    {
        ppu.NMITriggered = false;
        if(!ppu.ReadReg2)
            ppu.Reg[2] |= PPU_FLAG_VBLANK;
#if PPU_DEBUG_VSYNC
        printf("PPU Start VBlank\n");
#endif
    }
    else if(dot == 4 && ppu.Reg[2] & PPU_FLAG_VBLANK)
        ppu.NMIallowed = true;
    ppu.ReadReg2 = false;
}

static void ppuLastDot(uint16_t *dot, uint16_t *line, bool curPicOutStat)
{
    *dot = 0;
    (*line)++;
    if(*line == 241) /* done drawing this frame */
        ppu.FrameDone = true;
    else if(*line == ppu.LinesTotal) /* Wrap back down after pre-render line */
        *line = 0;
    ppu.curLine = *line;
    /* For MMC5 Scanline Detect */
    if((*line <= VISIBLE_LINES) && curPicOutStat)
    {
        ppuInFrame = true;
        ppuScanlineDone = true;
    }
    else
        ppuInFrame = false;
    ppu.DoOverscan = (doOverscan && (*line < 8 || *line >= 232));
    ppu.NextHasZSprite = false; //reset
    ppu.OAMzSpritePos = 0; // reset
    ppu.SpriteOAM2Pos = 0; //reset
    ppu.ToDraw = ppu.SpriteTilePos;
    ppu.SpriteTilePos = 0; //reset
    ppu.OAM2pos = 0; //reset
    ppu.TmpOAMVal = 0xFF;
    memset(ppu.OAM2, ppu.TmpOAMVal, 0x20);
    //printf("Line done\n");
}

void ppuCycle()
{
    //try and make the pic out stat, cur dot and cur line local variables to reduce the time it gets pulled from bss!
    size_t drawPos;
    uint16_t dot = ppu.curDot, line = ppu.curLine, cPalIdx;
    uint8_t curCol;
    //these 2 stats are set by cpu, no need to have them in loop
    ppu.CurNMIStat = !!(ppu.Reg[0] & PPU_FLAG_NMI);
    bool picOutStat = (ppu.Reg[1] & (PPU_BG_ENABLE | PPU_SPRITE_ENABLE)) != 0;
    ppu.VBlankClearCycle = false;
    //get value of how often ppu has to run
    uint8_t ppuLoop = ppu.RunCycles[ppu.Count];
    ppu.Count = (ppu.Count+1)%5;
    if(nesEmuNSFPlayback)
    {
        while(ppuLoop--)
        {
            if(line == ppu.PreRenderLine)
            {
                /* Mini-switch for basic vblank handle */
                switch(dot)
                {
                case 0: /* VBlank ends at first dot of the pre-render line */
                    ppu.Reg[2] &= ~(PPU_FLAG_SPRITEZERO | PPU_FLAG_OVERFLOW);
                    goto add_dot_nsf;
                case 2: /* Though results are better when clearing it a bit later */
#if PPU_DEBUG_VSYNC
                    printf("PPU End VBlank\n");
#endif
                    ppu.Reg[2] &= ~(PPU_FLAG_VBLANK);
                    goto add_dot_nsf;
                case 7:
                    ppu.NMIallowed = false;
                    goto add_dot_nsf;
                case 339:
                    ppu.OddFrame = ppu.OddArr[ppu.OddNum^=1];
                    if(ppu.OddFrame && (ppu.Reg[1] & PPU_BG_ENABLE))
                    {
                        ppuLastDot(&dot,&line,picOutStat);
                        break;
                    }
                    goto add_dot_nsf;
                case 340:
                    ppuLastDot(&dot,&line,picOutStat);
                    break;
                default:
                add_dot_nsf:
                    dot++;
                    break;
                }
            }
            else
            {
                if(line == 241)
                    ppuCheckLine241(dot);
                if(dot == 340)
                    ppuLastDot(&dot,&line,picOutStat);
                else
                    dot++;
            }
        }
    }
    else
    {
        while(ppuLoop--)
        {
            if(line == ppu.PreRenderLine || line < VISIBLE_LINES)
            {
                switch(dot)
                {
                case 0: /* VBlank ends at first dot of the pre-render line */
                    if(line == ppu.PreRenderLine)
                    {
                        ppu.Reg[2] &= ~(PPU_FLAG_SPRITEZERO | PPU_FLAG_OVERFLOW);
                        goto add_dot;
                    }
                    //else if line < VISIBLE_LINES
                    ppu.BGEnable = (ppu.Reg[1] & PPU_BG_8PX) && (ppu.Reg[1] & PPU_BG_ENABLE);
                    ppu.SprEnable = (ppu.Reg[1] & PPU_SPRITE_8PX) && (ppu.Reg[1] & PPU_SPRITE_ENABLE);
                    goto do_render_pixel;
                case 2: /* Though results are better when clearing it a bit later */
                    if(line == ppu.PreRenderLine)
                    {
#if PPU_DEBUG_VSYNC
                        printf("PPU End VBlank\n");
#endif
                        ppu.Reg[2] &= ~(PPU_FLAG_VBLANK);
                        goto add_dot;
                    }
                    //else if line < VISIBLE_LINES
                    goto do_render_pixel;
                case 7:
                    if(line == ppu.PreRenderLine)
                    {
                        ppu.NMIallowed = false;
                        goto add_dot;
                    }
                    //else if line < VISIBLE_LINES
                    goto do_render_pixel;
                case 1: case 3: case 5:
                case 6: case 9: case 10:
                case 11: case 13: case 14: case 15:
                case 17: case 18: case 19: case 21:
                case 22: case 23: case 25: case 26:
                case 27: case 29: case 30: case 31:
                case 33: case 34: case 35: case 37:
                case 38: case 39: case 41: case 42:
                case 43: case 45: case 46: case 47:
                case 49: case 50: case 51: case 53:
                case 54: case 55: case 57: case 58:
                case 59: case 61: case 62: case 63:
                    if(line < VISIBLE_LINES) //needs a lot of optimization, takes a lot of cpu atm
                    {
                        do_render_pixel:
                        /* Grab color to render from BG and Sprites */
                        curCol = ppu.BGEnable ? ppu.BGTiles[(dot + ppu.FineXScroll) & 15] : 0;
                        if(ppu.SprEnable) curCol = ppuDoSprites(curCol, dot);
                        /* Draw current dot on screen */
                        drawPos = (dot)+(line<<8);
                        if(ppu.DoOverscan) /* Draw clipped area as black */
                            textureImage[drawPos] = 0xFF000000;
                        else
                        {
                            if(picOutStat) //use color from bg or sprite input
                                cPalIdx = ppu.PALRAM2[curCol&0x1F];
                            else if((ppu.VramAddr & 0x3F00) == 0x3F00) //bg and sprite disabled but address within PALRAM
                                cPalIdx = ppu.PALRAM2[ppu.VramAddr&0x1F];
                            else //bg and sprite disabled and address not within PALRAM
                                cPalIdx = ppu.PALRAM[0];
                            textureImage[drawPos] = ppu.BGRLUT[cPalIdx];
                        }
                    }
                    goto add_dot;
                case 4: case 12: case 20: case 28:
                case 36: case 44: case 52: case 60:
                    updateBGRegsB();
                    if(line < VISIBLE_LINES)
                        goto do_render_pixel;
                    goto add_dot;
                case 8:
                    if(picOutStat) updateBGTileAddress();
                    loadTiles(); updateBGRegsA(dot);
                    if(line < VISIBLE_LINES)
                    {
                        ppu.BGEnable = (ppu.Reg[1] & PPU_BG_ENABLE);
                        ppu.SprEnable = (ppu.Reg[1] & PPU_SPRITE_ENABLE);
                        goto do_render_pixel;
                    }
                    goto add_dot;
                case 16: case 24: case 32:
                case 40: case 48: case 56:
                    if(picOutStat) updateBGTileAddress();
                    loadTiles(); updateBGRegsA(dot);
                    if(line < VISIBLE_LINES)
                        goto do_render_pixel;
                    goto add_dot;
                case 64:
                    if(picOutStat) updateBGTileAddress();
                    loadTiles(); updateBGRegsA(dot);
                    if(line < VISIBLE_LINES)
                    {
                        if(picOutStat)
                        {
                            spriteEvalInit();
                            spriteEvalA();
                        }
                        goto do_render_pixel;
                    }
                    goto add_dot;
                case 65: case 67: case 69: case 71:
                case 73: case 75: case 77: case 79:
                case 81: case 83: case 85: case 87:
                case 89: case 91: case 93: case 95:
                case 97: case 99: case 101: case 103:
                case 105: case 107: case 109: case 111:
                case 113: case 115: case 117: case 119:
                case 121: case 123: case 125: case 127:
                case 129: case 131: case 133: case 135:
                case 137: case 139: case 141: case 143:
                case 145: case 147: case 149: case 151:
                case 153: case 155: case 157: case 159:
                case 161: case 163: case 165: case 167:
                case 169: case 171: case 173: case 175:
                case 177: case 179: case 181: case 183:
                case 185: case 187: case 189: case 191:
                case 193: case 195: case 197: case 199:
                case 201: case 203: case 205: case 207:
                case 209: case 211: case 213: case 215:
                case 217: case 219: case 221: case 223:
                case 225: case 227: case 229: case 231:
                case 233: case 235: case 237: case 239:
                case 241: case 243: case 245: case 247:
                case 249: case 253: case 255:
                do_sprite_eval_b:
                    if(line < VISIBLE_LINES)
                    {
                        if(picOutStat) spriteEvalB(line);
                        goto do_render_pixel;
                    }
                    goto add_dot;
                case 66: case 70: case 74: case 78:
                case 82: case 86: case 90: case 94:
                case 98: case 102: case 106: case 110:
                case 114: case 118: case 122: case 126:
                case 130: case 134: case 138: case 142:
                case 146: case 150: case 154: case 158:
                case 162: case 166: case 170: case 174:
                case 178: case 182: case 186: case 190:
                case 194: case 198: case 202: case 206:
                case 210: case 214: case 218: case 222:
                case 226: case 230: case 234: case 238:
                case 242: case 246: case 250: case 254:
                do_sprite_eval_a:
                    if(line < VISIBLE_LINES)
                    {
                        if(picOutStat) spriteEvalA();
                        goto do_render_pixel;
                    }
                    goto add_dot;
                case 68: case 76: case 84: case 92:
                case 100: case 108: case 116: case 124:
                case 132: case 140: case 148: case 156:
                case 164: case 172: case 180: case 188:
                case 196: case 204: case 212: case 220:
                case 228: case 236: case 244: case 252:
                    updateBGRegsB();
                    goto do_sprite_eval_a;
                case 72: case 80: case 88: case 96:
                case 104: case 112: case 120: case 128:
                case 136: case 144: case 152: case 160:
                case 168: case 176: case 184: case 192:
                case 200: case 208: case 216: case 224:
                case 232: case 240: case 248:
                    if(picOutStat) updateBGTileAddress();
                    loadTiles(); updateBGRegsA(dot);
                    goto do_sprite_eval_a;
                case 251:
                    if(picOutStat) updateBGYAddress();
                    goto do_sprite_eval_b;
                case 256:
                    if(line < VISIBLE_LINES)
                        clearSpriteWorkRegs();
                    loadTiles(); updateBGRegsA(dot);
                    if(picOutStat)
                    {
                        updateBGTileAddress();
                        goto do_spritetiles_P1;
                    }
                    goto add_dot;
                case 257:
                    if(picOutStat)
                    {
                        updateBGHoriAddress();
                        goto do_spritetiles_P2;
                    }
                    goto add_dot;
                case 258: case 266: case 274: case 306:
                case 314:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        do_spritetiles_P3:
                        grabNextSpriteTilesP3();
                    }
                    goto add_dot;
                case 259: case 267: case 275: case 307:
                case 315:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        do_spritetiles_P4:
                        grabNextSpriteTilesP4();
                    }
                    goto add_dot;
                case 260: case 268: case 276: case 308:
                case 316:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        do_spritetiles_P5:
                        grabNextSpriteTilesP5(line);
                    }
                    goto add_dot;
                case 261: case 262: case 263: case 269:
                case 270: case 271: case 277: case 278:
                case 279: case 309: case 310: case 311:
                case 317: case 318: case 321: case 322:
                case 323: case 325: case 326: case 327:
                case 329: case 330: case 331: case 333:
                case 334: case 335: case 337: case 338:
                    if(picOutStat && line < VISIBLE_LINES) resetOAMPos();
                    goto add_dot;
                case 264: case 272: case 312:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        do_spritetiles_P1:
                        grabNextSpriteTilesP1();
                    }
                    goto add_dot;
                case 265: case 273: case 305: case 313:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        do_spritetiles_P2:
                        grabNextSpriteTilesP2();
                    }
                    goto add_dot;
                case 280: case 288: case 296: case 304:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                        grabNextSpriteTilesP1();
                    }
                    goto add_dot;
                case 281: case 289: case 297:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                        grabNextSpriteTilesP2();
                    }
                    goto add_dot;
                case 282: case 290: case 298:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                        goto do_spritetiles_P3;
                    }
                    goto add_dot;
                case 283: case 291: case 299:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                        goto do_spritetiles_P4;
                    }
                    goto add_dot;
                case 284: case 292: case 300:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                        goto do_spritetiles_P5;
                    }
                    goto add_dot;
                case 285: case 286: case 287: case 293:
                case 294: case 295: case 301: case 302:
                case 303:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        else if(line == ppu.PreRenderLine) updateBGVertAddress();
                    }
                    goto add_dot;
                case 319:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        ppu.TmpOAMVal = ppu.OAM2[0];
                    }
                    goto add_dot;
                case 320:
                    if(picOutStat && line < VISIBLE_LINES) resetOAMPos();
                    updateBGRegsA(dot);
                    goto add_dot;
                case 324:
                case 332:
                    if(picOutStat && line < VISIBLE_LINES) resetOAMPos();
                    updateBGRegsB();
                    goto add_dot;
                case 328:
                    ppu.BGIndex = 0;
                case 336:
                    if(picOutStat)
                    {
                        if(line < VISIBLE_LINES) resetOAMPos();
                        updateBGTileAddress();
                    }
                    loadTiles(); updateBGRegsA(dot);
                    goto add_dot;
                case 339:
                    if(line == ppu.PreRenderLine)
                    {
                        ppu.OddFrame = ppu.OddArr[ppu.OddNum^=1];
                        if(ppu.OddFrame && (ppu.Reg[1] & PPU_BG_ENABLE))
                        {
                            ppuLastDot(&dot,&line,picOutStat);
                            break;
                        }
                    }
                    else if(picOutStat && line < VISIBLE_LINES)
                        resetOAMPos();
                    goto add_dot;
                case 340:
                    if(picOutStat && line < VISIBLE_LINES) resetOAMPos();
                    updateBGRegsB();
                    ppuLastDot(&dot,&line,picOutStat);
                    break;
                default:
                add_dot:
                    dot++;
                    break;
                }
            }
            else
            {
                if(line == 241)
                    ppuCheckLine241(dot);
                if(dot == 340)
                    ppuLastDot(&dot,&line,picOutStat);
                else
                    dot++;
            }
        }
    }
    ppu.curDot = dot;
}

void ppuPrintcurLineDot()
{
    printf("%i %i %02x\n", ppu.curLine, ppu.curDot, ppu.OAMpos);
}

static uint8_t ppuDoSprites(uint8_t color, uint16_t dot)
{
    uint8_t i;
    for(i = 0; i < ppu.ToDraw; i++)
    {
        uint16_t x = dot - ppu.Sprites[i][8];
        if (x > 7) continue;
        //required pixel
        x = ppu.Sprites[i][x];
        if (x)
        {
            if ((color & ppu.Sprites[i][9]) && dot < 255)
            {
                ppu.Reg[2] |= PPU_FLAG_SPRITEZERO;
#if PPU_DEBUG_ULTRA
                printf("Zero sprite hit at x %i y %i cSpriteDot %i "
							"table %04x color %02x sprCol %02x\n", dot, ppu.curLine, cSpriteDot, ppuGetVramTbl((ppu.Reg[0]&3)<<10), color, sprCol);
#endif
                //if(ppu.curLine < 224)
                //	ppuDebugPauseFrame = true;
            }
            if (!(color & ppu.Sprites[i][10]))
                color = ppu.Sprites[i][11] | x;

            break;
        }
    }
    return color;
}

bool ppuDrawDone()
{
    if(ppu.FrameDone)
    {
        //printf("%i\n",ppuCycles);
        //ppuCycles = 0;
        ppu.FrameDone = false;
        return true;
    }
    return false;
}

void ppuSet8(uint8_t reg, uint8_t val)
{
    ppu.lastVal = val;
    if(reg == 0)
    {
        ppu.Reg[0] = val;
        ppu.TmpVramAddr &= ~0xC00;
        ppu.TmpVramAddr |= ((val&3)<<10);
        ppu816Sprite = (val & PPU_SPRITE_8_16) != 0;
        //printf("%d %d %d\n", (PPU_Reg[0] & PPU_BACKGROUND_ADDR) != 0, (PPU_Reg[0] & PPU_SPRITE_ADDR) != 0, (PPU_Reg[0] & PPU_SPRITE_8_16) != 0);
    }
    else if(reg == 1)
    {
        if((ppu.Reg[1]^val)&(PPU_GRAY|0xE0))
        {
            uint8_t i;
            for(i = 0; i < 0x20; i++)
                ppu.PALRAM2[i] = (ppu.PALRAM[i]&((val&PPU_GRAY)?0x30:0x3F))|((val&0xE0)<<1);
        }
        ppu.BGEnable = (ppu.curDot >= 8 || (val & PPU_BG_8PX)) && (val & PPU_BG_ENABLE);
        ppu.SprEnable = (ppu.curDot >= 8 || (val & PPU_SPRITE_8PX)) && (val & PPU_SPRITE_ENABLE);
        ppu.Reg[1] = val;
    }
    else if(reg == 3)
    {
#if PPU_DEBUG_ULTRA
        printf("ppu.OAMpos at line %i dot %i was %02x set to %02x\n", ppu.curLine, ppu.curDot, ppu.OAMpos, val);
#endif
        ppu.OAMpos = val;
    }
    else if(reg == 4)
    {
#if PPU_DEBUG_ULTRA
        printf("Setting OAM at line %i dot %i addr %02x to %02x\n", ppu.curLine, ppu.curDot, ppu.OAMpos, val);
#endif
        ppu.OAM[ppu.OAMpos++] = val;
    }
    else if(reg == 5)
    {
#if PPU_DEBUG_ULTRA
        printf("ppuScrollWrite (%d) %02x pc %04x\n", ppu.TmpWrite, val, cpuGetPc());
#endif
        if(!ppu.TmpWrite)
        {
            ppu.TmpWrite = true;
            ppu.FineXScroll = val&7;
            ppu.TmpVramAddr &= ~0x1F;
            ppu.TmpVramAddr |= ((val>>3)&0x1F);
        }
        else
        {
            ppu.TmpWrite = false;
            ppu.TmpVramAddr &= ~0x73E0;
            ppu.TmpVramAddr |= ((val&7)<<12) | ((val>>3)<<5);
        }
    }
    else if(reg == 6)
    {
#if PPU_DEBUG_ULTRA
        printf("ppu.VramAddrWrite (%d) %02x pc %04x\n", ppu.TmpWrite, val, cpuGetPc());
#endif
        if(!ppu.TmpWrite)
        {
            ppu.TmpWrite = true;
            ppu.TmpVramAddr &= 0xFF;
            ppu.TmpVramAddr |= ((val&0x3F)<<8);
        }
        else
        {
            ppu.TmpWrite = false;
            ppu.TmpVramAddr &= ~0xFF;
            ppu.TmpVramAddr |= val;
            ppu.VramAddr = ppu.TmpVramAddr;
            //For MMC3 IRQ (Shadow Read)
            if((ppu.VramAddr&0x3FFF) < 0x2000)
                mapperChrGet8(ppu.VramAddr&0x3FFF);
        }
    }
    else if(reg == 7)
    {
        uint16_t writeAddr = (ppu.VramAddr & 0x3FFF);
        if(writeAddr < 0x2000)
        {
            mapperChrSet8(writeAddr, val);
        }
        else if(writeAddr < 0x3F00)
        {
            uint16_t workAddr = ppuGetVramTbl(writeAddr) | (writeAddr & 0x3FF);
            //printf("ppuVRAMwrite %04x %02x\n", workAddr, val);
            mapperVramSet8(workAddr, val);
        }
        else
        {
            uint8_t palRamAddr = (writeAddr&0x1F);
            if((palRamAddr&3) == 0)
            {
                ppu.PALRAM[palRamAddr^0x10] = val;
                ppu.PALRAM2[palRamAddr^0x10] = (val&((ppu.Reg[1]&PPU_GRAY)?0x30:0x3F))|((ppu.Reg[1]&0xE0)<<1);
            }
            ppu.PALRAM[palRamAddr] = val;
            ppu.PALRAM2[palRamAddr] = (val&((ppu.Reg[1]&PPU_GRAY)?0x30:0x3F))|((ppu.Reg[1]&0xE0)<<1);
        }
        ppu.VramAddr += (ppu.Reg[0] & PPU_INC_AMOUNT) ? 32 : 1;
        //For MMC3 IRQ (Shadow Read)
        if((ppu.VramAddr&0x3FFF) < 0x2000)
            mapperChrGet8(ppu.VramAddr&0x3FFF);
    }
    else if(reg != 2)
    {
        ppu.Reg[reg] = val;
        //printf("ppuSet8 odd %d ppu.curDot %i ppu.curLine %i %04x %02x\n", ppu.OddFrame, ppu.curDot, ppu.curLine, reg, val);
    }
}

uint8_t ppuGet8(uint8_t reg)
{
    uint8_t ret = ppu.lastVal;
    if(reg == 2)
    {
        ret = ppu.Reg[reg];
        ppu.Reg[reg] &= ~PPU_FLAG_VBLANK;
        if(ret & PPU_FLAG_VBLANK)
        {
            ppu.VBlankFlagCleared = true;
            ppu.VBlankClearCycle = true;
        }
        ppu.TmpWrite = false;
        ppu.ReadReg2 = true;
    }
    else if(reg == 4)
    {
        if(ppu.Reg[2] & PPU_FLAG_VBLANK || (ppu.Reg[1] & (PPU_BG_ENABLE | PPU_SPRITE_ENABLE)) == 0)
            ret = ppu.OAM[ppu.OAMpos];
        else
            ret = ppu.TmpOAMVal;
        //printf("Cycle %i line %i val %02x\n", ppu.curDot, ppu.curLine, ret);
    }
    else if(reg == 7)
    {
        uint16_t writeAddr = (ppu.VramAddr & 0x3FFF);
        if(writeAddr < 0x2000)
        {
            ret = ppu.VramReadBuf;
            mapperChrMode = 2;
            ppu.VramReadBuf = mapperChrGet8(writeAddr);
        }
        else if(writeAddr < 0x3F00)
        {
            ret = ppu.VramReadBuf;
            uint16_t workAddr = ppuGetVramTbl(writeAddr) | (writeAddr & 0x3FF);
            ppu.VramReadBuf = mapperVramGet8(workAddr);
            //printf("ppuVRAMread pc %04x addr %04x ret %02x\n", cpuGetPc(), workAddr, ret);
        }
        else
        {
            uint8_t palRamAddr = (writeAddr&0x1F);
            if((palRamAddr&3) == 0)
                palRamAddr &= ~0x10;
            ret = ppu.PALRAM[palRamAddr]&((ppu.Reg[1]&PPU_GRAY)?0x30:0x3F);
            //shadow read
            uint16_t workAddr = ppuGetVramTbl(writeAddr) | (writeAddr & 0x3FF);
            ppu.VramReadBuf = mapperVramGet8(workAddr);
        }
        ppu.VramAddr += (ppu.Reg[0] & PPU_INC_AMOUNT) ? 32 : 1;
        //For MMC3 IRQ (Shadow Read)
        if((ppu.VramAddr&0x3FFF) < 0x2000)
            mapperChrGet8(ppu.VramAddr&0x3FFF);
    }
    //if(ret & PPU_FLAG_VBLANK)
    //printf("ppuGet8 %04x:%02x\n",reg,ret);
    ppu.lastVal = ret;
    return ret;
}

bool ppuNMI()
{
    if(ppu.VBlankFlagCleared && !ppu.VBlankClearCycle)
    {
        ppu.VBlankFlagCleared = false;
        ppu.NMIallowed = false;
    }
    if(ppu.CurNMIStat && ppu.NMIallowed)
    {
        if(ppu.NMITriggered == false)
        {
            ppu.NMITriggered = true;
            return true;
        }
        else
            return false;
    }
    ppu.NMITriggered = false;
    return false;
}

void ppuDumpMem()
{
    FILE *f = fopen("PPU_VRAM.bin","wb");
    if(f)
    {
        fwrite(ppu.VRAM,1,0x1000,f);
        fclose(f);
    }
    f = fopen("PPU_OAM.bin","wb");
    if(f)
    {
        fwrite(ppu.OAM,1,0x100,f);
        fclose(f);
    }
    f = fopen("PPU_Sprites.bin","wb");
    if(f)
    {
        fwrite(ppu.Sprites,1,0x20,f);
        fclose(f);
    }
}

uint16_t ppuGetCurVramAddr()
{
    return ppu.VramAddr;
}

void ppuSetNameTblSingleLower()
{
    ppu.NameTbl[0] = 0; ppu.NameTbl[1] = 0; ppu.NameTbl[2] = 0; ppu.NameTbl[3] = 0;
}

void ppuSetNameTblSingleUpper()
{
    ppu.NameTbl[0] = 0x400; ppu.NameTbl[1] = 0x400; ppu.NameTbl[2] = 0x400; ppu.NameTbl[3] = 0x400;
}

void ppuSetNameTblVertical()
{
    ppu.NameTbl[0] = 0; ppu.NameTbl[1] = 0x400; ppu.NameTbl[2] = 0; ppu.NameTbl[3] = 0x400;
}

void ppuSetNameTblHorizontal()
{
    ppu.NameTbl[0] = 0; ppu.NameTbl[1] = 0; ppu.NameTbl[2] = 0x400; ppu.NameTbl[3] = 0x400;
}

void ppuSetNameTbl4Screen()
{
    ppu4Screen = true;
    ppu.NameTbl[0] = 0; ppu.NameTbl[1] = 0x400; ppu.NameTbl[2] = 0x800; ppu.NameTbl[3] = 0xC00;
}

void ppuSetNameTblCustom(uint16_t addrA, uint16_t addrB, uint16_t addrC, uint16_t addrD)
{
    //printf("%04x %04x %04x %04x\n", addrA, addrB, addrC, addrD);
    //ppuPrintppu.curLineDot();
    ppu.NameTbl[0] = addrA; ppu.NameTbl[1] = addrB; ppu.NameTbl[2] = addrC; ppu.NameTbl[3] = addrD;
}

void ppuBackUpTbl()
{
    ppu.NameTblBak[0] = ppu.NameTbl[0]; ppu.NameTblBak[1] = ppu.NameTbl[1];
    ppu.NameTblBak[2] = ppu.NameTbl[2]; ppu.NameTblBak[3] = ppu.NameTbl[3];
}

void ppuRestoreTbl()
{
    ppu.NameTbl[0] = ppu.NameTblBak[0]; ppu.NameTbl[1] = ppu.NameTblBak[1];
    ppu.NameTbl[2] = ppu.NameTblBak[2]; ppu.NameTbl[3] = ppu.NameTblBak[3];
}

//64x12 1BPP "Track"
static const uint8_t ppuNSFTextTrack[96] =
    {
        0x0C, 0x1C, 0x03, 0xD8, 0x7C, 0x71, 0xC0, 0x00, 0x0C, 0x1C, 0x07, 0xF8, 0xFE, 0x73, 0x80, 0x00,
        0x0C, 0x1C, 0x06, 0x39, 0xE2, 0x77, 0x00, 0x00, 0x0C, 0x1C, 0x07, 0x39, 0xC0, 0x7E, 0x00, 0x00,
        0x0C, 0x1C, 0x07, 0xF9, 0xC0, 0x7C, 0x00, 0x00, 0x0C, 0x1C, 0x01, 0xF9, 0xC0, 0x7C, 0x00, 0x00,
        0x0C, 0x1E, 0x60, 0x38, 0xE2, 0x7E, 0x00, 0x00, 0x0C, 0x1F, 0xE3, 0xF8, 0xFE, 0x77, 0x00, 0x00,
        0x0C, 0x1D, 0xC3, 0xF0, 0x3C, 0x73, 0xC0, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
        0xFF, 0xC0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x70, 0x00, 0x00,
    };

//128x12 1BPP "0123456789/"
static const uint8_t ppuNsfTextRest[192] =
    {
        0x0E, 0x1F, 0xF7, 0xF8, 0xF8, 0x03, 0x9F, 0x01, 0xF0, 0x60, 0x1F, 0x0F, 0x83, 0x00, 0x00, 0x00,
        0x3F, 0x9F, 0xF7, 0xF9, 0xFC, 0x03, 0x9F, 0xC3, 0xF8, 0x70, 0x3F, 0x8F, 0xC3, 0x00, 0x00, 0x00,
        0x3B, 0x83, 0x83, 0x80, 0x0E, 0x7F, 0xC1, 0xE7, 0x1C, 0x70, 0x71, 0xC0, 0xE1, 0x80, 0x00, 0x00,
        0x71, 0xC3, 0x81, 0xC0, 0x0E, 0x7F, 0xC0, 0xE7, 0x1C, 0x30, 0x71, 0xC0, 0x71, 0x80, 0x00, 0x00,
        0x79, 0xC3, 0x80, 0xE0, 0x0E, 0x63, 0x80, 0xE7, 0x1C, 0x38, 0x71, 0xC7, 0x70, 0xC0, 0x00, 0x00,
        0x7D, 0xC3, 0x80, 0x70, 0x7E, 0x33, 0x81, 0xE7, 0x1C, 0x18, 0x3F, 0x8F, 0xF0, 0xC0, 0x00, 0x00,
        0x77, 0xC3, 0x80, 0x70, 0x7C, 0x13, 0x9F, 0xC7, 0xF8, 0x1C, 0x1F, 0x1C, 0x70, 0x60, 0x00, 0x00,
        0x73, 0xC3, 0x80, 0x38, 0x0E, 0x1B, 0x9F, 0x87, 0x70, 0x1C, 0x31, 0x9C, 0x70, 0x60, 0x00, 0x00,
        0x71, 0xC3, 0x80, 0x38, 0x0E, 0x0B, 0x9C, 0x07, 0x00, 0x0C, 0x71, 0xDC, 0x70, 0x30, 0x00, 0x00,
        0x3B, 0x9F, 0x80, 0x38, 0x0E, 0x0F, 0x9C, 0x03, 0x80, 0x0E, 0x71, 0xDC, 0x70, 0x30, 0x00, 0x00,
        0x3F, 0x8F, 0x83, 0xF1, 0xFC, 0x07, 0x9F, 0xC1, 0xF9, 0xFE, 0x3F, 0x8F, 0xE0, 0x18, 0x00, 0x00,
        0x0E, 0x03, 0x81, 0xE0, 0xF8, 0x03, 0x9F, 0xC0, 0xF9, 0xFE, 0x1F, 0x07, 0xC0, 0x18, 0x00, 0x00,
    };

static void ppuDrawRest(uint8_t curX, uint8_t sym)
{
    uint8_t i, j;
    for(i = 0; i < 12; i++)
    {
        for(j = 0; j < 10; j++)
        {
            size_t drawPos = (j+curX)+((i+9)*256);
            uint8_t xSel = (j+(sym*10));
            if(ppuNsfTextRest[((11-i)<<4)+(xSel>>3)]&(0x80>>(xSel&7)))
                textureImage[drawPos] = 0xFFFFFFFF; //White
            else
                textureImage[drawPos] = 0xFF000000; //Black
        }
    }
}

void ppuDrawNSFTrackNum(uint8_t cTrack, uint8_t trackTotal)
{
    memset(textureImage,0,0xB400);
    uint8_t curX = 4;
    //draw "Track"
    uint8_t i, j;
    for(i = 0; i < 12; i++)
    {
        for(j = 0; j < 50; j++)
        {
            size_t drawPos = (j+curX)+((i+9)*256);
            if(ppuNSFTextTrack[((11-i)<<3)+(j>>3)]&(0x80>>(j&7)))
                textureImage[drawPos] = 0xFFFFFFFF; //White
            else
                textureImage[drawPos] = 0xFF000000; //Black
        }
    }
    //"Track" len+space
    curX+=60;
    //draw current num
    if(cTrack > 99)
    {
        ppuDrawRest(curX, (cTrack/100)%10);
        curX+=10;
    }
    if(cTrack > 9)
    {
        ppuDrawRest(curX, (cTrack/10)%10);
        curX+=10;
    }
    ppuDrawRest(curX, cTrack%10);
    curX+=10;
    //draw the "/"
    ppuDrawRest(curX, 10);
    curX+=10;
    //draw total num
    if(trackTotal > 99)
    {
        ppuDrawRest(curX, (trackTotal/100)%10);
        curX+=10;
    }
    if(trackTotal > 9)
    {
        ppuDrawRest(curX, (trackTotal/10)%10);
        curX+=10;
    }
    ppuDrawRest(curX, trackTotal%10);
    curX+=10;
}

uint8_t ppuVRAMGet8(uint16_t addr)
{
    return ppu.VRAM[addr&0xFFF];
}

void ppuVRAMSet8(uint16_t addr, uint8_t val)
{
    ppu.VRAM[addr&0xFFF] = val;
}

uint8_t* ppuGetVRAM()
{
    return ppu.VRAM;
}
