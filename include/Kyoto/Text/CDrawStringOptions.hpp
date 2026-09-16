#ifndef _CDRAWSTRINGOPTIONS
#define _CDRAWSTRINGOPTIONS

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Text/TextCommon.hpp"
#include "rstl/reserved_vector.hpp"

class CDrawStringOptions {
public:
  CDrawStringOptions();

  void SetTextDirection(ETextDirection dir) { x0_direction = dir; }
  ETextDirection GetTextDirection() const { return x0_direction; }
  void SetPaletteEntry(int idx, uint color) { x4_colors[idx] = color; }
  uint GetPaletteEntry(int idx) const { return x4_colors[idx]; }

private:
  ETextDirection x0_direction;
  rstl::reserved_vector< u32, 16 > x4_colors;
#if VERSION >= VERSION_GM8P_00 && VERSION != VERSION_GM8E_02
  int x48_extraCharacterSpacing;
#endif
};

#endif // _CDRAWSTRINGOPTIONS
