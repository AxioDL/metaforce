#pragma once

#include <utility>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/GuiSys/CGuiTextSupport.hpp"

namespace urde {
class CFontImageDef;
class CTextExecuteBuffer;

class CTextParser {
  IObjectStore& x0_store;

  static CTextColor ParseColor(const char16_t* str, int len);
  static u8 GetColorValue(const char16_t* str);
  static u32 FromHex(char16_t ch);
  static s32 ParseInt(const char16_t* str, int len, bool signVal);
  static CAssetId GetAssetIdFromString(const char16_t* str, int len,
                                       const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap);
  static bool Equals(const char16_t* str, int len, const char16_t* other);
  static bool BeginsWith(const char16_t* str, int len, const char16_t* other);
  void ParseTag(CTextExecuteBuffer& out, const char16_t* str, int len,
                const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap);
  CFontImageDef GetImage(const char16_t* str, int len, const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap);
  TToken<CRasterFont> GetFont(const char16_t* str, int len);

public:
  CTextParser(IObjectStore& store) : x0_store(store) {}
  void ParseText(CTextExecuteBuffer& out, const char16_t* str, int len,
                 const std::vector<std::pair<CAssetId, CAssetId>>* txtrMap);
};

} // namespace urde
