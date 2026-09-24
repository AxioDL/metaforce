#include "MetroidPrime/CNESEmulator.hpp"

#include "Kyoto/Alloc/CMemory.hpp"
#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/Graphics/CGX.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Input/CFinalInput.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "NESemu/ksNes.hpp"
#include <dolphin/gx.h>
#include <dolphin/os/OSModule.h>
#include <musyx/musyx.h>
#include <string.h>

ksNesModule* sNesModule;

static uchar skPasswordEntryText[16] = {0x19, 0x0a, 0x1c, 0x1c, 0xff, 0x20, 0x18, 0x1b,
                                        0x0d, 0xff, 0x19, 0x15, 0x0e, 0x0a, 0x1c, 0x0e};
static uchar skGameOverText[16] = {0x19, 0x0a, 0x1c, 0x1c, 0xff, 0x20, 0x18, 0x1b, 0x0d};

CNESEmulator::EPasswordEntryState CNESEmulator::CheckForPasswordEntryScreen(const uchar* vram) {
  if (memcmp(vram + 0x88, skPasswordEntryText, 16)) {
    return kPES_NotPasswordScreen;
  }

  for (int i = 0; i < 13; ++i) {
    if (vram[0x109 + i] < 0x40 || vram[0x149 + i] < 0x40) {
      return kPES_Entered;
    }
  }
  return kPES_NotEntered;
}

bool CNESEmulator::SetPasswordIntoEntryScreen(uchar* vram, uchar* wram, const uchar* password) {
  if (CheckForPasswordEntryScreen(vram) != kPES_NotEntered) {
    return false;
  }

  uint i;
  for (i = 0; i < 18; ++i) {
    if (password[i]) {
      break;
    }
  }
  if (i == 18) {
    return false;
  }

  uint encodedOffset = 0;
  uint characterOffset = 0;
  uint word;
  for (i = 0; i < 24; ++i) {
    switch (i & 3) {
    case 0:
      word = password[encodedOffset++];
      break;
    case 1:
      word = (word >> 6) | (static_cast< uint >(password[encodedOffset++]) << 2);
      break;
    case 2:
      word = (word >> 6) | (static_cast< uint >(password[encodedOffset++]) << 4);
      break;
    case 3:
      word >>= 6;
      break;
    }

    uchar character = word & 0x3f;
    wram[0x99a + i] = character;
    vram[0x109 + characterOffset] = character;
    ++characterOffset;
    if (characterOffset == 6 || characterOffset == 0x46) {
      ++characterOffset;
    } else if (characterOffset == 13) {
      characterOffset = 64;
    }
  }
  return true;
}

bool CNESEmulator::CheckForGameOver(const uchar* vram, uchar* passwordOut) {
  if (memcmp(vram + 0x14b, skGameOverText, 9)) {
    return false;
  }

  int i;
  uint character;
  uint encodedOffset = 0;
  uint characterOffset = 0;
  uchar password[18];
  for (i = 0; i < 24; ++i) {
    character = vram[0x1a9 + characterOffset];
    ++characterOffset;
    if (characterOffset == 6 || characterOffset == 0x46) {
      ++characterOffset;
    } else if (characterOffset == 13) {
      characterOffset = 64;
    }

    if (character > 0x3f) {
      return false;
    }

    switch (i & 3) {
    case 0:
      password[encodedOffset] = character;
      break;
    case 1:
      password[encodedOffset] |= character << 6;
      ++encodedOffset;
      password[encodedOffset] = character >> 2;
      break;
    case 2:
      password[encodedOffset] |= character << 4;
      ++encodedOffset;
      password[encodedOffset] = character >> 4;
      break;
    case 3:
      password[encodedOffset] |= character << 2;
      ++encodedOffset;
      break;
    }
  }
  memcpy(passwordOut, password, 18);
  return true;
}

CNESEmulator::CNESEmulator()
: x0_dvdRequest(nullptr)
, x4_resultBuffer(static_cast< uchar* >(
      CMemory::Alloc(KS_NES_DRAW_RESULT_BUF_SIZE, IAllocator::kHI_RoundUpLen)))
, x8_work(nullptr)
, xc_state(nullptr)
, x10_module(nullptr)
, x14_bss(nullptr)
, x18_chrRam(nullptr)
, x1c_bbRam(nullptr)
, x20_gameOver(false)
, x34_passwordEntryState(kPES_NotPasswordScreen)
, x38_passwordPending(false) {
  sNesModule =
      static_cast< ksNesModule* >(CMemory::Alloc(sizeof(ksNesModule), IAllocator::kHI_RoundUpLen));
  x8_work = static_cast< ksNesCommonWorkObj* >(
      CMemory::Alloc(sizeof(ksNesCommonWorkObj), IAllocator::kHI_RoundUpLen));
  x8_work->prg_size = 0x20000;
  x8_work->nesromp =
      static_cast< uchar* >(CMemory::Alloc(x8_work->prg_size, IAllocator::kHI_RoundUpLen));
  x8_work->chr_to_i8_buf_size = 0x8000;
  x8_work->chr_to_u8_bufp = static_cast< uchar* >(
      CMemory::Alloc(x8_work->chr_to_i8_buf_size, IAllocator::kHI_RoundUpLen));
  x8_work->result_bufp = x4_resultBuffer.get();
  xc_state = static_cast< ksNesStateObj* >(
      CMemory::Alloc(sizeof(ksNesStateObj), IAllocator::kHI_RoundUpLen));
  x18_chrRam = static_cast< uchar* >(CMemory::Alloc(0x2000, IAllocator::kHI_RoundUpLen));
  x1c_bbRam = static_cast< uchar* >(CMemory::Alloc(0x8000, IAllocator::kHI_RoundUpLen));

  char filename[] = "NESemuP.rel";
  CDvdFile file(filename);
  uint size = (file.Length() + 31) & ~31;
  x10_module = static_cast< OSModuleHeader* >(CMemory::Alloc(size, IAllocator::kHI_RoundUpLen));
  x0_dvdRequest = file.SyncRead(x10_module, size);
}

CNESEmulator::~CNESEmulator() {
  if (x0_dvdRequest.null()) {
    uint stream = sNesModule->getAudioStream();
    sndStreamMixParameter(stream, 0, 64, 64, 0);
    sndStreamDeactivate(stream);
    sndStreamFree(stream);
    reinterpret_cast< void (*)() >(x10_module->epilog)();
    OSUnlink(&x10_module->info);
  }

  CMemory::Free(x8_work->nesromp);
  CMemory::Free(x8_work->chr_to_u8_bufp);
  CMemory::Free(x8_work);
  CMemory::Free(xc_state);
  CMemory::Free(x18_chrRam);
  CMemory::Free(x1c_bbRam);
  CMemory::Free(sNesModule);
  CMemory::Free(x10_module);
  if (x14_bss != nullptr) {
    CMemory::Free(x14_bss);
  }
}

void CNESEmulator::ProcessUserInput(const CFinalInput& input, int controller) {
  static const uint skButtonUp = 0x08000000;
  static const uint skButtonDown = 0x04000000;
  static const uint skButtonLeft = 0x02000000;
  static const uint skButtonRight = 0x01000000;
  static const uint skButtonA = 0x80000000;
  static const uint skButtonB = 0x40000000;
  static const uint skButtonSelect = 0x20000000;
  static const uint skButtonStart = 0x10000000;

  if (!x0_dvdRequest.null()) {
    return;
  }

  uint buttons = 0;
  if (input.DDPUp() || input.ALAUp() > 0.4f) {
    buttons |= skButtonUp;
  } else if (input.DDPDown() || input.ALADown() > 0.4f) {
    buttons |= skButtonDown;
  }
  if (input.DDPLeft() || input.ALALeft() > 0.4f) {
    buttons |= skButtonLeft;
  } else if (input.DDPRight() || input.ALARight() > 0.4f) {
    buttons |= skButtonRight;
  }

  if (GetPasswordEntryState() != kPES_NotPasswordScreen || input.ControllerNumber() != 0) {
    if (input.DA()) {
      buttons |= skButtonA;
    }
    if (input.DB()) {
      buttons |= skButtonB;
    }
  } else {
    if (input.DB() || input.DX()) {
      buttons |= skButtonA;
    }
    if (input.DA() || input.DY()) {
      buttons |= skButtonB;
    }
  }
  if (input.DZ()) {
    buttons |= skButtonSelect;
  }
  if (input.DStart()) {
    buttons |= skButtonStart;
  }

  switch (controller == 4 ? input.ControllerNumber() : controller) {
  case 0:
    x8_work->pads[0] = buttons;
    break;
  case 1:
    x8_work->pads[2] = buttons;
    break;
  case 2:
    x8_work->pads[1] = buttons;
    break;
  case 3:
    x8_work->pads[3] = buttons;
    break;
  }
}

void CNESEmulator::Update() {
  if (!x0_dvdRequest.null()) {
    if (x0_dvdRequest->IsComplete()) {
      x14_bss = CMemory::Alloc(x10_module->bssSize, IAllocator::kHI_RoundUpLen);
      OSLink(&x10_module->info, x14_bss);
      reinterpret_cast< void (*)() >(x10_module->prolog)();
      sNesModule->initAudio();
      sNesModule->reset(x8_work, xc_state, 0, x18_chrRam, x1c_bbRam);
      x8_work->pads[0] = 0;
      x8_work->pads[2] = 0;
      x8_work->pads[1] = 0;
      x8_work->pads[3] = 0;
      x0_dvdRequest = nullptr;
    } else {
      return;
    }
  }

  x20_gameOver = CheckForGameOver(xc_state->ppu_nametable_ram, x21_password);
  x34_passwordEntryState = CheckForPasswordEntryScreen(xc_state->ppu_nametable_ram);
  if (x34_passwordEntryState == kPES_NotEntered && x38_passwordPending) {
    SetPasswordIntoEntryScreen(xc_state->ppu_nametable_ram, x1c_bbRam, x39_passwordToLoad);
    x38_passwordPending = false;
  }
  sNesModule->emuFrame(x8_work, xc_state, 0);
}

static void DrawNesTexture(int left, int top, int width, int height, const CColor& color) {
  const CViewport viewport = CGraphics::GetViewport();
  const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
  CGraphics::SetOrtho(-viewport.mWidth / 2, viewport.mWidth / 2, viewport.mHeight / 2,
                      -viewport.mHeight / 2, 0.f, 10.f);
  CGraphics::SetModelMatrix(CTransform4f::Identity());

  float x = left - viewport.mWidth / 2;
  float z = top - viewport.mHeight / 2;
  static const GXVtxDescList vtxDesc[] = {
      {GX_VA_POS, GX_DIRECT}, {GX_VA_TEX0, GX_DIRECT}, {GX_VA_NULL, GX_NONE}};
  CGX::SetVtxDescv(vtxDesc);
  CGraphics::DisableAllLights();
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetZMode(true, GX_ALWAYS, false);
  CGX::SetTevKColor(GX_KCOLOR0, color.GetGXColor());
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  GXSetCullMode(GX_CULL_NONE);

  CGX::Begin(GX_TRIANGLESTRIP, GX_VTXFMT0, 4);
  GXPosition3f32(x, 1.f, z);
  GXTexCoord2f32(0.f, 0.f);
  GXPosition3f32(x + width, 1.f, z);
  GXTexCoord2f32(1.f, 0.f);
  GXPosition3f32(x, 1.f, z + height);
  GXTexCoord2f32(0.f, 1.f);
  GXPosition3f32(x + width, 1.f, z + height);
  GXTexCoord2f32(1.f, 1.f);
  CGX::End();

  GXSetCullMode(GX_CULL_FRONT);
  CGraphics::SetProjectionState(projection);
}

void CNESEmulator::Draw(const CColor& color, bool enableFiltering) {
  if (!x0_dvdRequest.null()) {
    return;
  }

  GXSetCullMode(GX_CULL_BACK);
#if defined(TARGET_PC)
  GXCreateFrameBuffer(640, 480);
#endif
  GXSetViewport(0.f, 0.f, 640.f, 480.f, 0.1f, 100.f);
  sNesModule->draw(x8_work, xc_state);
#if defined(TARGET_PC)
  GXRestoreFrameBuffer();
  CGraphics::SetDepthRange(CGraphics::GetDepthNear(), CGraphics::GetDepthFar());
#endif
  sNesModule->drawInit(x8_work);
  GXSetClipMode(GX_CLIP_ENABLE);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_NRM, GX_NRM_XYZ, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
  GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
  for (int i = 1; i <= 7; ++i) {
    GXSetVtxAttrFmt(GX_VTXFMT0, static_cast< GXAttr >(GX_VA_TEX0 + i), GX_TEX_ST, GX_F32, 0);
  }
  GXSetTevOp(GX_TEVSTAGE0, GX_REPLACE);
  GXSetTexCoordScaleManually(GX_TEXCOORD0, false, 0, 0);
  CGX::ResetGXStatesFull();
  CGX::SetNumChans(0);
  CGX::SetNumTexGens(1);
  CGX::SetNumTevStages(1);
  CGX::SetNumIndStages(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, false, GX_PTIDENTITY);
  CGX::SetTevDirect(GX_TEVSTAGE0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGraphics::SetAlphaCompare(kAF_Always, 0, kAO_And, kAF_Always, 0);
  gpRender->SetModelMatrix(CTransform4f::Identity());
  CGraphics::SetViewPointMatrix(CTransform4f::Identity());
  gpRender->SetBlendMode_Replace();

  GXTexObj texture;
  GXTexFilter filter = enableFiltering ? GX_LINEAR : GX_NEAR;
  GXInitTexObj(&texture, x4_resultBuffer.get(), 256, 228, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texture, filter, filter, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  CTexture::InvalidateTexmap(GX_TEXMAP0);
  GXInvalidateTexAll();
  GXLoadTexObj(&texture, GX_TEXMAP0);

  int left, top, width, height;
  CGraphics::GetViewport(left, top, width, height);
  CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
  GXSetPixelFmt(gpRender->IsRGBA6Current() ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR);
  DrawNesTexture(left, top + height, width, -height, color);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

CNESEmulator::EPasswordEntryState CNESEmulator::GetPasswordEntryState() const {
  return x34_passwordEntryState;
}

void CNESEmulator::LoadPassword(const uchar* password) {
  memcpy(x39_passwordToLoad, password, 18);
  x38_passwordPending = true;
}

bool CNESEmulator::IsGameOver() const { return x20_gameOver; }

const uchar* CNESEmulator::GetPassword() const { return x21_password; }
