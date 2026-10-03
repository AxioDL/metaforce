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

const CColor& CColor::Black() { return sBlackColor; }
#if VERSION < VERSION_R3IJ_00
const CColor& CColor::White() { return sWhiteColor; }
#endif
const CColor& CColor::Grey() { return sGreyColor; }
const CColor& CColor::Red() { return sRedColor; }
const CColor& CColor::Green() { return sGreenColor; }
const CColor& CColor::Blue() { return sBlueColor; }
const CColor& CColor::Yellow() { return sYellowColor; }
const CColor& CColor::Purple() { return sPurpleColor; }
const CColor& CColor::Orange() { return sOrangeColor; }
