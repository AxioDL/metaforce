#ifndef __URDE_CTEXTPARSER_HPP__
#define __URDE_CTEXTPARSER_HPP__

#include "CGuiTextSupport.hpp"

namespace urde
{
class CTextExecuteBuffer;
class CFontImageDef;

class CTextParser
{
    IObjectStore& x0_store;

    static CTextColor ParseColor(const char16_t* str, int len);
    static u8 GetColorValue(const char16_t* str);
    static u32 FromHex(char16_t ch);
    static s32 ParseInt(const char16_t* str, int len, bool signVal);
    static ResId GetAssetIdFromString(const char16_t* str);
    static bool Equals(const char16_t* str, int len, const char16_t* other);
    static bool BeginsWith(const char16_t* str, int len, const char16_t* other);
    void ParseTag(CTextExecuteBuffer& out, const char16_t* str, int len);
    CFontImageDef GetImage(const char16_t* str, int len);
    TToken<CRasterFont> GetFont(const char16_t* str, int len);

public:
    CTextParser(IObjectStore& store) : x0_store(store) {}
    void ParseText(CTextExecuteBuffer& out, const char16_t* str, int len);
};

}

#endif // __URDE_CTEXTPARSER_HPP__
