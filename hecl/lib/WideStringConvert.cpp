#include <utf8proc.h>
#include "HECL/HECL.hpp"

namespace HECL
{

std::string WideToUTF8(const std::wstring& src)
{
    std::string retval;
    retval.reserve(src.length());
    for (wchar_t ch : src)
    {
        utf8proc_uint8_t mb[4];
        utf8proc_ssize_t c = utf8proc_encode_char(utf8proc_int32_t(ch), mb);
        if (c < 0)
        {
            LogModule.report(LogVisor::Warning, "invalid UTF-8 character while encoding");
            return retval;
        }
        retval.append(reinterpret_cast<char*>(mb), c);
    }
    return retval;
}

std::wstring UTF8ToWide(const std::string& src)
{
    std::wstring retval;
    retval.reserve(src.length());
    const utf8proc_uint8_t* buf = reinterpret_cast<const utf8proc_uint8_t*>(src.c_str());
    while (*buf)
    {
        utf8proc_int32_t wc;
        utf8proc_ssize_t len = utf8proc_iterate(buf, -1, &wc);
        if (len < 0)
        {
            LogModule.report(LogVisor::Warning, "invalid UTF-8 character while decoding");
            return retval;
        }
        buf += len;
        retval += wchar_t(wc);
    }
    return retval;
}

}
