#include "HECL/HECL.hpp"

namespace HECL
{
LogVisor::LogModule LogModule("HECL");

template <class T>
inline void replaceAll(T& str, const T& from, const T& to)
{
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
}

void SanitizePath(std::string& path)
{
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
    replaceAll<std::string>(path, "<>:\"|?*", "_");
}

void SanitizePath(std::wstring& path)
{
    path.erase(std::remove(path.begin(), path.end(), L'\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), L'\r'), path.end());
    replaceAll<std::wstring>(path, L"<>:\"|?*", L"_");
}

}
