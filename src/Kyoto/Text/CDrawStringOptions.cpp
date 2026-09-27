#include "Kyoto/Text/CDrawStringOptions.hpp"

static const uint col = 0;

CDrawStringOptions::CDrawStringOptions()
: mDirection(kTD_Horizontal)
, mColors(col)
#if VERSION >= VERSION_GM8P_00
, mExtraCharacterSpacing(0)
#endif
{}
