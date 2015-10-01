#include "HECL/HECL.hpp"

namespace HECL
{
unsigned VerbosityLevel = 0;
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

bool IsPathPNG(const HECL::ProjectPath& path)
{
    FILE* fp = HECL::Fopen(path.getAbsolutePath().c_str(), _S("rb"));
    if (!fp)
        return false;
    uint32_t buf;
    if (fread(&buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    buf = HECL::SBig(buf);
    if (buf == 0x89504e47)
        return true;
    return false;
}

bool IsPathBlend(const HECL::ProjectPath& path)
{
    FILE* fp = HECL::Fopen(path.getAbsolutePath().c_str(), _S("rb"));
    if (!fp)
        return false;
    uint32_t buf;
    if (fread(&buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    buf = HECL::SLittle(buf);
    if (buf == 0x4e454c42 || buf == 0x88b1f)
        return true;
    return false;
}

bool IsPathYAML(const HECL::ProjectPath& path)
{
    const SystemChar* lastCompExt = path.getLastComponentExt();
    if (!lastCompExt)
        return false;
    if (!HECL::StrCmp(lastCompExt, _S("yaml")) ||
        !HECL::StrCmp(lastCompExt, _S("yml")))
        return true;
    return false;
}

}
