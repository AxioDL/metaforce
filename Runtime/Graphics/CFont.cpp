#include "Runtime/Graphics/CFont.hpp"

#include "Runtime/Graphics/CGraphics.hpp"

namespace metaforce {
/* TODO: Custom I8 font */
std::array<u8, 65536> CFont::sSystemFont = {
    /* Omitted due to copyright issues */};

u32 CFont::sNumInstances = 0;
std::unique_ptr<CTexture> CFont::mpTexture;

CFont::CFont(float scale) : x0_fontSize(16.f * scale), x4_scale(scale) {
  if (sNumInstances == 0) {
    mpTexture = std::make_unique<CTexture>(ETexelFormat::I8, 256, 256, 1, "Font Texture");
    u8* fontData = new u8[(mpTexture->GetBitDepth() * mpTexture->GetWidth() * mpTexture->GetHeight()) / 8];
    memcpy(fontData, sSystemFont.data(), sSystemFont.size());
    // u8* textureData = mpTexture->GetBitMapData();
    // LinearToTile8(textureData, fontData);
    delete[] fontData;
    // mpTexture->UnLock();
  }
  ++sNumInstances;
}

void CFont::Shutdown() {
  mpTexture.reset();
}

void CFont::TileCopy8(u8* dest, const u8* src) {
  for (u32 i = 0; i < 4; ++i) {
    dest[0] = src[0];
    dest[1] = src[1];
    dest[2] = src[2];
    dest[3] = src[3];
    dest[4] = src[4];
    dest[5] = src[5];
    dest[6] = src[6];
    dest[7] = src[7];
    src += 256;
    dest += 8;
  }
}

void CFont::LinearToTile8(u8* dest, const u8* src) {
  int iVar1 = 0;
  int iVar2 = 0;
  for (size_t y = 0; y < 256; y += 4) {
    iVar2 = iVar1;
    for (size_t x = 0; x < 256; x += 8) {
      TileCopy8(dest, src + iVar2);
      dest += 32;
      iVar2 += 8;
    }
    iVar1 += 1024;
  }
}

void CFont::DrawString(const char* str, int x, int y, const zeus::CColor& col) {
  // bool bVar2 = CGraphics::BeginRender2D(*mpTexture.get());
  // char chr = *str;
  // while (chr != 0) {
  //   u32 cellSize = static_cast<u32>(16.f * x4_scale);
  //   ++str;
  //   CGraphics::DoRender2D(*mpTexture, x, y, (chr & 0xf) * 16, 0xf0, 0x10, cellSize, cellSize, col);
  //   chr = *str;
  // }
  // CGraphics::EndRender2D(bVar2);
}

u32 CFont::StringWidth(const char* str) const {
  u32 width = 0;
  char chr = *str;
  while (chr != 0) {
    ++str;
    width += static_cast<u32>(15.f * x4_scale);
    chr = *str;
  }

  return width;
}

u32 CFont::CharsWidth(const char* str, u32 len) const { return len * (15.f * x4_scale); }

u32 CFont::CharWidth(const char chr) const { return 15.f * x4_scale; }
} // namespace metaforce
