#include "HECL/HECL.hpp"

namespace HECL
{
LogVisor::LogModule LogModule("HECL");

void SanitizePath(std::string& path)
{
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
    std::transform(path.begin(), path.end(), path.begin(), [](const char a) -> char {
        static const std::string illegals {"<>?*\"|"};
        if (illegals.find_first_of(a) != std::string::npos)
            return '_';
        return a;
    });
}

void SanitizePath(std::wstring& path)
{
    path.erase(std::remove(path.begin(), path.end(), L'\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), L'\r'), path.end());
    std::transform(path.begin(), path.end(), path.begin(), [](const wchar_t a) -> wchar_t {
        static const std::wstring illegals {L"<>?*\"|"};
        if (illegals.find_first_of(a) != std::wstring::npos)
            return L'_';
        return a;
    });
}

}
