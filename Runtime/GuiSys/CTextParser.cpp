#include "CTextParser.hpp"
#include "CFontImageDef.hpp"
#include "CTextExecuteBuffer.hpp"

namespace urde
{

CTextColor CTextParser::ParseColor(const wchar_t* str, int len)
{
    u8 r = GetColorValue(str + 1);
    u8 g = GetColorValue(str + 3);
    u8 b = GetColorValue(str + 5);
    u8 a = 0xff;
    if (len == 9)
        a = GetColorValue(str + 7);
    return CTextColor(r, g, b, a);
}

u8 CTextParser::GetColorValue(const wchar_t* str)
{
    return (FromHex(str[0]) << 4) + FromHex(str[1]);
}

u32 CTextParser::FromHex(wchar_t ch)
{
    if (ch >= L'0' && ch <= L'9')
        return ch - L'0';

    if (ch >= L'A' && ch <= L'F')
        return ch - L'A' + 10;

    if (ch >= L'a' && ch <= L'f')
        return ch - L'a' + 10;

    return 0;
}

s32 CTextParser::ParseInt(const wchar_t* str, int len, bool signVal)
{
    bool neg = false;
    int procCur = 0;
    if (signVal && len && *str == L'-')
    {
        neg = true;
        procCur = 1;
    }

    int val = 0;
    while (len > procCur)
    {
        val *= 10;
        wchar_t ch = str[procCur];
        val += ch - L'0';
        ++procCur;
    }

    return neg ? -val : val;
}

bool CTextParser::Equals(const wchar_t* str, int len, const wchar_t* other)
{
    int i=0;
    for (; *other && i<len ; ++i, ++str, ++other)
    {
        if (*str != *other)
            return false;
    }
    return other[i] == L'\0';
}

bool CTextParser::BeginsWith(const wchar_t* str, int len, const wchar_t* other)
{
    int i=0;
    for (; *other && i<len ; ++i, ++str, ++other)
    {
        if (*str != *other)
            return false;
    }
    return true;
}

void CTextParser::ParseTag(CTextExecuteBuffer& out, const wchar_t* str, int len)
{
    if (BeginsWith(str, len, L"font="))
    {
        TToken<CRasterFont> font = GetFont(str + 5, len - 5);
        out.AddFont(font);
    }
    else if (BeginsWith(str, len, L"image="))
    {
        CFontImageDef image = GetImage(str + 6, len - 6);
        out.AddImage(image);
    }
    else if (BeginsWith(str, len, L"fg-color="))
    {
        CTextColor color = ParseColor(str + 9, len - 9);
        out.AddColor(EColorType::Foreground, color);
    }
    else if (BeginsWith(str, len, L"main-color="))
    {
        CTextColor color = ParseColor(str + 11, len - 11);
        out.AddColor(EColorType::Main, color);
    }
    else if (BeginsWith(str, len, L"geometry-color="))
    {
        CTextColor color = ParseColor(str + 15, len - 15);
        out.AddColor(EColorType::Geometry, color);
    }
    else if (BeginsWith(str, len, L"outline-color="))
    {
        CTextColor color = ParseColor(str + 14, len - 14);
        out.AddColor(EColorType::Outline, color);
    }
    else if (BeginsWith(str, len, L"color"))
    {
        const wchar_t* valCur = str + 7;
        len -= 7;
        int val = str[6] - L'0';
        if (str[7] >= L'0' && str[7] <= L'9')
        {
            ++valCur;
            --len;
            val *= 10;
            val += str[7] - L'0';
        }

        if (Equals(valCur + 10, len - 10, L"no"))
            out.AddRemoveColorOverride(val);
        else
        {
            CTextColor color = ParseColor(str + 10, len - 10);
            out.AddColorOverride(val, color);
        }
    }
    else if (BeginsWith(str, len, L"line-spacing="))
    {
        out.AddLineSpacing(ParseInt(str + 13, len - 13, true) / 100.0);
    }
    else if (BeginsWith(str, len, L"line-extra-space="))
    {
        out.AddLineSpacing(ParseInt(str + 17, len - 17, true));
    }
    else if (BeginsWith(str, len, L"just="))
    {
        if (Equals(str + 5, len - 5, L"left"))
            out.AddJustification(EJustification::Left);
        else if (Equals(str + 5, len - 5, L"center"))
            out.AddJustification(EJustification::Center);
        else if (Equals(str + 5, len - 5, L"right"))
            out.AddJustification(EJustification::Right);
        else if (Equals(str + 5, len - 5, L"full"))
            out.AddJustification(EJustification::Full);
        else if (Equals(str + 5, len - 5, L"nleft"))
            out.AddJustification(EJustification::NLeft);
        else if (Equals(str + 5, len - 5, L"ncenter"))
            out.AddJustification(EJustification::NCenter);
        else if (Equals(str + 5, len - 5, L"nright"))
            out.AddJustification(EJustification::NRight);
    }
    else if (BeginsWith(str, len, L"vjust="))
    {
        if (Equals(str + 6, len - 6, L"top"))
            out.AddVerticalJustification(EVerticalJustification::Top);
        else if (Equals(str + 6, len - 6, L"center"))
            out.AddVerticalJustification(EVerticalJustification::Center);
        else if (Equals(str + 6, len - 6, L"bottom"))
            out.AddVerticalJustification(EVerticalJustification::Bottom);
        else if (Equals(str + 6, len - 6, L"full"))
            out.AddVerticalJustification(EVerticalJustification::Full);
        else if (Equals(str + 6, len - 6, L"ntop"))
            out.AddVerticalJustification(EVerticalJustification::NTop);
        else if (Equals(str + 6, len - 6, L"ncenter"))
            out.AddVerticalJustification(EVerticalJustification::NCenter);
        else if (Equals(str + 6, len - 6, L"nbottom"))
            out.AddVerticalJustification(EVerticalJustification::NBottom);
    }
    else if (Equals(str, len, L"push"))
    {
        out.AddPushState();
    }
    else if (Equals(str, len, L"pop"))
    {
        out.AddPopState();
    }
}

CFontImageDef CTextParser::GetImage(const wchar_t* str, int len)
{
    int commaCount = 0;
    for (int i=0 ; i<len ; ++i)
        if (str[i] == L',')
            ++commaCount;

    if (commaCount)
    {
        std::wstring iterable(str, len);
        size_t tokenPos;
        size_t commaPos;
        commaPos = iterable.find(L',');
        iterable[commaPos] = L'\0';
        tokenPos = commaPos + 1;

        auto AdvanceCommaPos = [&]()
        {
            commaPos = iterable.find(L',', tokenPos);
            if (commaPos == std::wstring::npos)
                commaPos = iterable.size();
            iterable[commaPos] = L'\0';
        };

        auto AdvanceTokenPos = [&]()
        {
            tokenPos = commaPos + 1;
        };

        if (BeginsWith(str, len, L"A"))
        {
            /* Animated texture array */
            AdvanceCommaPos();
            float interval = std::wcstof(&iterable[tokenPos], nullptr);
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

            return CFontImageDef(std::move(texs), interval, zeus::CVector2f(1.f, 1.f));
        }
        else if (BeginsWith(str, len, L"SA"))
        {
            /* Scaled and animated texture array */
            AdvanceCommaPos();
            float interval = std::wcstof(&iterable[tokenPos], nullptr);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float scaleX = std::wcstof(&iterable[tokenPos], nullptr);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float scaleY = std::wcstof(&iterable[tokenPos], nullptr);
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

            return CFontImageDef(std::move(texs), interval, zeus::CVector2f(scaleX, scaleY));
        }
        else if (BeginsWith(str, len, L"SI"))
        {
            /* Scaled single texture */
            AdvanceCommaPos();
            float scaleX = std::wcstof(&iterable[tokenPos], nullptr);
            AdvanceTokenPos();

            AdvanceCommaPos();
            float scaleY = std::wcstof(&iterable[tokenPos], nullptr);
            AdvanceTokenPos();

            AdvanceCommaPos();
            TToken<CTexture> tex = x0_store.GetObj({SBIG('TXTR'),
                GetAssetIdFromString(&iterable[tokenPos])});
            AdvanceTokenPos();

            return CFontImageDef(std::move(tex), zeus::CVector2f(scaleX, scaleY));
        }
    }

    TToken<CTexture> tex = x0_store.GetObj({SBIG('TXTR'), GetAssetIdFromString(str)});
    return CFontImageDef(std::move(tex), zeus::CVector2f(1.f, 1.f));
}

TResId CTextParser::GetAssetIdFromString(const wchar_t* str)
{
    u8 r = GetColorValue(str);
    u8 g = GetColorValue(str + 2);
    u8 b = GetColorValue(str + 4);
    u8 a = GetColorValue(str + 6);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

TToken<CRasterFont> CTextParser::GetFont(const wchar_t* str, int len)
{
    return x0_store.GetObj({SBIG('FONT'), GetAssetIdFromString(str)});
}

void CTextParser::ParseText(CTextExecuteBuffer& out, const wchar_t* str, int len)
{
    for (int b=0, e=0 ; str[e] && (len == -1 || e < len) ;)
    {
        if (str[e] != L'&')
        {
            ++e;
            continue;
        }
        if ((len == -1 || e+1 < len) && str[e+1] != L'&')
        {
            out.AddString(str + b, e - b);
            ++e;
            b = e;

            while (str[e] && (len == -1 || e < len) && str[e] != L';')
                ++e;

            ParseTag(out, str + e, e - b);
            b = e + 1;
        }
        else
        {
            out.AddString(str + b, e + 1 - b);
            e += 2;
            b = e;
        }
    }
}

}
