#pragma once

#include "Runtime/GCNTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"

#include <zeus/CColor.hpp>

#include <memory>
#include <array>

namespace metaforce {
class CFont {
  static std::array<u8, 65536> sSystemFont;
  static u32 sNumInstances;
  static std::unique_ptr<CTexture> mpTexture;
  float x0_fontSize;
  float x4_scale;
  void TileCopy8(u8* dest, const u8* src);
  void LinearToTile8(u8* dest, const u8* src);
public:
  explicit CFont(float scale);

  void DrawString(const char* str, int x, int y, const zeus::CColor& color);
  u32 StringWidth(const char* str) const;
  u32 CharsWidth(const char* str, u32 len) const;
  u32 CharWidth(const char chr) const;

  static void Shutdown();
};
}