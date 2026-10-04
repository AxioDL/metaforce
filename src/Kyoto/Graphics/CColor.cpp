#include "Kyoto/Graphics/CColor.hpp"

const CColor CColor::sBlackColor(0x000000FF);
const CColor CColor::sWhiteColor(0xFFFFFFFF);
const CColor CColor::sGreyColor(0x808080FF);
const CColor CColor::sRedColor(0xFF0000FF);
const CColor CColor::sGreenColor(0x00FF00FF);
const CColor CColor::sBlueColor(0x0000FFFF);
const CColor CColor::sYellowColor(0xFFFF00FF);
const CColor CColor::sPurpleColor(0xA000FFFF);
const CColor CColor::sOrangeColor(0xFF7000FF);

#if VERSION < VERSION_R3IJ_00
const CColor& CColor::Black() { return sBlackColor; }
const CColor& CColor::White() { return sWhiteColor; }
#endif
const CColor& CColor::Grey() { return sGreyColor; }
#if VERSION < VERSION_R3IJ_00
const CColor& CColor::Red() { return sRedColor; }
const CColor& CColor::Green() { return sGreenColor; }
#endif
const CColor& CColor::Blue() { return sBlueColor; }
const CColor& CColor::Yellow() { return sYellowColor; }
#if VERSION < VERSION_R3IJ_00
const CColor& CColor::Purple() { return sPurpleColor; }
#endif
const CColor& CColor::Orange() { return sOrangeColor; }
