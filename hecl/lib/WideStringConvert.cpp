#include "HECL/HECL.hpp"

namespace HECL
{

std::string WideToUTF8(const std::wstring& src)
{
#if _WIN32
    int len = WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0, nullptr, nullptr);
    std::string retval(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, src.c_str(), src.size(), &retval[0], len, nullptr, nullptr);
    return retval;
#else
    std::string retval;
    retval.reserve(src.length());
    std::mbstate_t state = {};
    for (wchar_t ch : src)
    {
        char mb[MB_LEN_MAX];
        int c = std::wcrtomb(mb, ch, &state);
        retval.append(mb, c);
    }
    return retval;
#endif
}

std::wstring UTF8ToWide(const std::string& src)
{
#if _WIN32
    int len = MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), nullptr, 0);
    std::wstring retval(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, src.c_str(), src.size(), &retval[0], len);
    return retval;
#else
    std::wstring retval;
    retval.reserve(src.length());
    const char* buf = src.c_str();
    std::mbstate_t state = {};
    while (*buf)
    {
        wchar_t wc;
        buf += std::mbrtowc(&wc, buf, MB_LEN_MAX, &state);
        retval += wc;
    }
    return retval;
#endif
}

}
