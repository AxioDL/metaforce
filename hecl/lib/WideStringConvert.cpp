#include "HECL/HECL.hpp"
#include <locale>
#include <codecvt>

namespace HECL
{

std::string WideToUTF8(const std::wstring& src)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.to_bytes(src);
}

std::wstring UTF8ToWide(const std::string& src)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
    return conv.from_bytes(src);
}

}
