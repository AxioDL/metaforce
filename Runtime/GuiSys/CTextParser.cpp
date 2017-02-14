#include "CTextParser.hpp"
#include "CFontImageDef.hpp"
#include "CTextExecuteBuffer.hpp"

namespace urde
{

static float u16stof(char16_t* str)
{
    char cstr[16];
    int i;
    for (i=0 ; i<15 && str[i] != u'\0' ; ++i)
        cstr[i] = str[i];
    cstr[i] = '\0';
    return strtof(cstr, nullptr);
}

CTextColor CTextParser::ParseColor(const char16_t* str, int len)
{
    u8 r = GetColorValue(str + 1);
    u8 g = GetColorValue(str + 3);
    u8 b = GetColorValue(str + 5);
    u8 a = 0xff;
    if (len == 9)
        a = GetColorValue(str + 7);
    CTextColor ret;
    ret.fromRGBA8(r, g, b, a);
    return ret;
}

u8 CTextParser::GetColorValue(const char16_t* str)
{
    return (FromHex(str[0]) << 4) + FromHex(str[1]);
}

u32 CTextParser::FromHex(char16_t ch)
{
    if (ch >= u'0' && ch <= u'9')
        return ch - u'0';

    if (ch >= u'A' && ch <= u'F')
        return ch - u'A' + 10;

    if (ch >= u'a' && ch <= u'f')
        return ch - u'a' + 10;

    return 0;
}

s32 CTextParser::ParseInt(const char16_t* str, int len, bool signVal)
{
    bool neg = false;
    int procCur = 0;
    if (signVal && len && *str == u'-')
    {
        neg = true;
        procCur = 1;
    }

    int val = 0;
    while (len > procCur)
    {
        val *= 10;
        wchar_t ch = str[procCur];
        val += ch - u'0';
        ++procCur;
    }

    return neg ? -val : val;
}

bool CTextParser::Equals(const char16_t* str, int len, const char16_t* other)
{
    int i=0;
    for (; *other && i<len ; ++i, ++str, ++other)
    {
        if (*str != *other)
            return false;
    }
    return other[i] == u'\0';
}

bool CTextParser::BeginsWith(const char16_t* str, int len, const char16_t* other)
{
    int i=0;
    for (; *other && i<len ; ++i, ++str, ++other)
    {
        if (*str != *other)
            return false;
    }
    return true;
}

void CTextParser::ParseTag(CTextExecuteBuffer& out, const char16_t* str, int len)
{
    if (BeginsWith(str, len, u"font="))
    {
        TToken<CRasterFont> font = GetFont(str + 5, len - 5);
        out.AddFont(font);
    }
    else if (BeginsWith(str, len, u"image="))
    {
        CFontImageDef image = GetImage(str + 6, len - 6);
        out.AddImage(image);
    }
    else if (BeginsWith(str, len, u"fg-color="))
    {
        CTextColor color = ParseColor(str + 9, len - 9);
        out.AddColor(EColorType::Foreground, color);
    }
    else if (BeginsWith(str, len, u"main-color="))
    {
        CTextColor color = ParseColor(str + 11, len - 11);
        out.AddColor(EColorType::Main, color);
    }
    else if (BeginsWith(str, len, u"geometry-color="))
    {
        CTextColor color = ParseColor(str + 15, len - 15);
        out.AddColor(EColorType::Geometry, color);
    }
    else if (BeginsWith(str, len, u"outline-color="))
    {
        CTextColor color = ParseColor(str + 14, len - 14);
        out.AddColor(EColorType::Outline, color);
    }
    else if (BeginsWith(str, len, u"color"))
    {
        const char16_t* valCur = str + 7;
        len -= 7;
        int val = str[6] - u'0';
        if (str[7] >= u'0' && str[7] <= u'9')
        {
            ++valCur;
            --len;
            val *= 10;
            val += str[7] - u'0';
        }

        if (Equals(valCur + 10, len - 10, u"no"))
            out.AddRemoveColorOverride(val);
        else
        {
            CTextColor color = ParseColor(str + 10, len - 10);
            out.AddColorOverride(val, color);
        }
    }
    else if (BeginsWith(str, len, u"line-spacing="))
    {
        out.AddLineSpacing(ParseInt(str + 13, len - 13, true) / 100.0);
    }
    else if (BeginsWith(str, len, u"line-extra-space="))
    {
        out.AddLineSpacing(ParseInt(str + 17, len - 17, true));
    }
    else if (BeginsWith(str, len, u"just="))
    {
        if (Equals(str + 5, len - 5, u"left"))
            out.AddJustification(EJustification::Left);
        else if (Equals(str + 5, len - 5, u"center"))
            out.AddJustification(EJustification::Center);
        else if (Equals(str + 5, len - 5, u"right"))
            out.AddJustification(EJustification::Right);
        else if (Equals(str + 5, len - 5, u"full"))
            out.AddJustification(EJustification::Full);
        else if (Equals(str + 5, len - 5, u"nleft"))
            out.AddJustification(EJustification::NLeft);
        else if (Equals(str + 5, len - 5, u"ncenter"))
            out.AddJustification(EJustification::NCenter);
        else if (Equals(str + 5, len - 5, u"nright"))
            out.AddJustification(EJustification::NRight);
    }
    else if (BeginsWith(str, len, u"vjust="))
    {
        if (Equals(str + 6, len - 6, u"top"))
            out.AddVerticalJustification(EVerticalJustification::Top);
        else if (Equals(str + 6, len - 6, u"center"))
            out.AddVerticalJustification(EVerticalJustification::Center);
        else if (Equals(str + 6, len - 6, u"bottom"))
            out.AddVerticalJustification(EVerticalJustification::Bottom);
        else if (Equals(str + 6, len - 6, u"full"))
            out.AddVerticalJustification(EVerticalJustification::Full);
        else if (Equals(str + 6, len - 6, u"ntop"))
            out.AddVerticalJustification(EVerticalJustification::NTop);
        else if (Equals(str + 6, len - 6, u"ncenter"))
            out.AddVerticalJustification(EVerticalJustification::NCenter);
        else if (Equals(str + 6, len - 6, u"nbottom"))
            out.AddVerticalJustification(EVerticalJustification::NBottom);
    }
    else if (Equals(str, len, u"push"))
    {
        out.AddPushState();
    }
    else if (Equals(str, len, u"pop"))
    {
        out.AddPopState();
    }
}

CFontImageDef CTextParser::GetImage(const char16_t* str, int len)
{
    int commaCount = 0;
    for (int i=0 ; i<len ; ++i)
        if (str[i] == u',')
            ++commaCount;

    if (commaCount)
    {
        std::u16string iterable(str, len);
        size_t tokenPos;
        size_t commaPos;
        commaPos = iterable.find(u',');
        iterable[commaPos] = u'\0';
        tokenPos = commaPos + 1;

        auto AdvanceCommaPos = [&]()
        {
            commaPos = iterable.find(u',', tokenPos);
            if (commaPos == std::u16string::npos)
                commaPos = iterable.size();
            iterable[commaPos] = u'\0';
        };

        auto AdvanceTokenPos = [&]()
        {
            tokenPos = commaPos + 1;
        };

        if (BeginsWith(str, len, u"A"))
        {
            /* Animated texture array */
            AdvanceCommaPos();
            float interval = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            std::vector<TToken<CTexture>> texs;
            texs.reserve(commaCount - 1);
            do
            {
                AdvanceCommaPos();
                texs.push_back(x0_store.GetObj({SBIG('TXTR'),
                    GetAssetIdFromString(&iterable[tokenPos])}));
                AdvanceTokenPos();
            } while (commaPos != iterable.size());

            return CFontImageDef(texs, interval, zeus::CVector2f(1.f, 1.f));
        }
        else if (BeginsWith(str, len, u"SA"))
        {
            /* Scaled and animated texture array */
            AdvanceCommaPos();
            float interval = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float cropX = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float cropY = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            std::vector<TToken<CTexture>> texs;
            texs.reserve(commaCount - 3);
            do
            {
                AdvanceCommaPos();
                texs.push_back(x0_store.GetObj({SBIG('TXTR'),
                    GetAssetIdFromString(&iterable[tokenPos])}));
                AdvanceTokenPos();
            } while (commaPos != iterable.size());

            return CFontImageDef(texs, interval, zeus::CVector2f(cropX, cropY));
        }
        else if (BeginsWith(str, len, u"SI"))
        {
            /* Scaled single texture */
            AdvanceCommaPos();
            float cropX = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float cropY = u16stof(&iterable[tokenPos]);
            AdvanceTokenPos();

            AdvanceCommaPos();
            TToken<CTexture> tex = x0_store.GetObj({SBIG('TXTR'),
                GetAssetIdFromString(&iterable[tokenPos])});
            AdvanceTokenPos();

            return CFontImageDef(tex, zeus::CVector2f(cropX, cropY));
        }
    }

    TToken<CTexture> tex = x0_store.GetObj({SBIG('TXTR'), GetAssetIdFromString(str)});
    return CFontImageDef(tex, zeus::CVector2f(1.f, 1.f));
}

ResId CTextParser::GetAssetIdFromString(const char16_t* str)
{
    u8 r = GetColorValue(str);
    u8 g = GetColorValue(str + 2);
    u8 b = GetColorValue(str + 4);
    u8 a = GetColorValue(str + 6);
    return ((r << 24) | (g << 16) | (b << 8) | a) & 0xffffffff;
}

TToken<CRasterFont> CTextParser::GetFont(const char16_t* str, int len)
{
    return x0_store.GetObj({SBIG('FONT'), GetAssetIdFromString(str)});
}

void CTextParser::ParseText(CTextExecuteBuffer& out, const char16_t* str, int len)
{
    int b=0, e=0;
    for (b=0, e=0 ; str[e] && (len == -1 || e < len) ;)
    {
        if (str[e] != u'&')
        {
            ++e;
            continue;
        }
        if ((len == -1 || e+1 < len) && str[e+1] != u'&')
        {
            out.AddString(str + b, e - b);
            ++e;
            b = e;

            while (str[e] && (len == -1 || e < len) && str[e] != u';')
                ++e;

            ParseTag(out, str + b, e - b);
            b = e + 1;
        }
        else
        {
            out.AddString(str + b, e + 1 - b);
            e += 2;
            b = e;
        }
    }

    if (e > b)
        out.AddString(str + b, e - b);
}

}
