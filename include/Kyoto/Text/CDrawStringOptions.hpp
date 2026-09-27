#ifndef _CDRAWSTRINGOPTIONS
#define _CDRAWSTRINGOPTIONS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/reserved_vector.hpp"

class CDrawStringOptions {
public:
  CDrawStringOptions();

  void SetTextDirection(ETextDirection dir) { mDirection = dir; }
  ETextDirection GetTextDirection() const { return mDirection; }
  void SetPaletteEntry(int idx, uint color) { mColors[idx] = color; }
  uint GetPaletteEntry(int idx) const { return mColors[idx]; }

private:
  ETextDirection mDirection;
  rstl::reserved_vector< u32, 16 > mColors;
#if VERSION >= VERSION_GM8P_00
  int mExtraCharacterSpacing;
#endif
};

#endif // _CDRAWSTRINGOPTIONS
