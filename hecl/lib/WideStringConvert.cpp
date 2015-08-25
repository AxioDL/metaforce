#include "HECL/HECL.hpp"

namespace HECL
{

std::string WideToUTF8(const std::wstring& src)
{
    std::string retval;
    retval.reserve(src.length());
    for (wchar_t ch : src)
    {
        char mb[4];
        int c = std::wctomb(mb, ch);
        retval.append(mb, c);
    }
    return retval;
}

std::wstring UTF8ToWide(const std::string& src)
{
    std::wstring retval;
    retval.reserve(src.length());
    const char* buf = src.c_str();
    while (*buf)
    {
        wchar_t wc;
        buf += std::mbtowc(&wc, buf, MB_CUR_MAX);
        retval += wc;
    }
    return retval;
}

}
