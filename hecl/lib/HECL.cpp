#include "HECL/HECL.hpp"

namespace HECL
{
unsigned VerbosityLevel = 0;
LogVisor::LogModule LogModule("HECL");

void SanitizePath(std::string& path)
{
    if (path.empty())
        return;
    path.erase(std::remove(path.begin(), path.end(), '\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), '\r'), path.end());
    std::string::iterator p1 = path.begin();
    bool ic = false;
    std::transform(path.begin(), path.end(), path.begin(), [&](const char a) -> char {
        ++p1;
        static const std::string illegals {"<>?*\"|"};
        if (illegals.find_first_of(a) != std::string::npos)
        {
            ic = false;
            return '_';
        }

        if (ic)
        {
            ic = false;
            return a;
        }
        if (a == '\\' && (p1 == path.end() || *p1 != '\\'))
        {
            ic = true;
            return '/';
        }
        return a;
    });
}

void SanitizePath(std::wstring& path)
{
    if (path.empty())
        return;
    path.erase(std::remove(path.begin(), path.end(), L'\n'), path.end());
    path.erase(std::remove(path.begin(), path.end(), L'\r'), path.end());
    std::wstring::iterator p1 = path.begin();
    bool ic = false;
    std::transform(path.begin(), path.end(), path.begin(), [&](const wchar_t a) -> wchar_t {
        ++p1;
        static const std::wstring illegals {L"<>?*\"|"};
        if (illegals.find_first_of(a) != std::wstring::npos)
        {
            ic = false;
            return L'_';
        }

        if (ic)
        {
            ic = false;
            return a;
        }
        if (a == L'\\' && (p1 == path.end() || *p1 != L'\\'))
        {
            ic = true;
            return L'/';
        }
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
    const SystemChar* lastCompExt = path.getLastComponentExt();
    if (!lastCompExt || HECL::StrCmp(lastCompExt, _S("blend")))
        return false;
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

HECL::DirectoryEnumerator::DirectoryEnumerator(const HECL::SystemChar* path, Mode mode,
                                               bool sizeSort, bool reverse)
{
    HECL::Sstat theStat;
    if (HECL::Stat(path, &theStat) || !S_ISDIR(theStat.st_mode))
        return;

#if _WIN32
    HECL::SystemString wc(path);
    wc += _S("/*");
    WIN32_FIND_DATAW d;
    HANDLE dir = FindFirstFileW(wc.c_str(), &d);
    if (dir == INVALID_HANDLE_VALUE)
        return;
    switch (mode)
    {
    case Mode::Native:
        do
        {
            if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                continue;
            HECL::SystemString fp(path);
            fp += _S('/');
            fp += d.cFileName;
            HECL::Sstat st;
            if (HECL::Stat(fp.c_str(), &st))
                continue;

            size_t sz = 0;
            bool isDir = false;
            if (S_ISDIR(st.st_mode))
                isDir = true;
            else if (S_ISREG(st.st_mode))
                sz = st.st_size;
            else
                continue;

            m_entries.push_back(std::move(Entry(std::move(fp), d.cFileName, sz, isDir)));
        } while (FindNextFileW(dir, &d));
        break;
    case Mode::DirsThenFilesSorted:
    case Mode::DirsSorted:
    {
        std::map<HECL::SystemString, Entry, CaseInsensitiveCompare> sort;
        do
        {
            if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                continue;
            HECL::SystemString fp(path);
            fp +=_S('/');
            fp += d.cFileName;
            HECL::Sstat st;
            if (HECL::Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode))
                continue;
            sort.emplace(std::make_pair(d.cFileName, Entry(std::move(fp), d.cFileName, 0, true)));
        } while (FindNextFileW(dir, &d));

        if (reverse)
            for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                m_entries.push_back(std::move(it->second));
        else
            for (auto& e : sort)
                m_entries.push_back(std::move(e.second));

        if (mode == Mode::DirsSorted)
            break;
        FindClose(dir);
        dir = FindFirstFileW(wc.c_str(), &d);
    }
    case Mode::FilesSorted:
    {
        if (mode == Mode::FilesSorted)
            m_entries.clear();

        if (sizeSort)
        {
            std::map<size_t, Entry> sort;
            do
            {
                if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                    continue;
                HECL::SystemString fp(path);
                fp += _S('/');
                fp += d.cFileName;
                HECL::Sstat st;
                if (HECL::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
                    continue;
                sort.emplace(std::make_pair(st.st_size, Entry(std::move(fp), d.cFileName, st.st_size, false)));
            } while (FindNextFileW(dir, &d));

            if (reverse)
                for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                    m_entries.push_back(std::move(it->second));
            else
                for (auto& e : sort)
                    m_entries.push_back(std::move(e.second));
        }
        else
        {
            std::map<HECL::SystemString, Entry, CaseInsensitiveCompare> sort;
            do
            {
                if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                    continue;
                HECL::SystemString fp(path);
                fp += _S('/');
                fp += d.cFileName;
                HECL::Sstat st;
                if (HECL::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
                    continue;
                sort.emplace(std::make_pair(d.cFileName, Entry(std::move(fp), d.cFileName, st.st_size, false)));
            } while (FindNextFileW(dir, &d));

            if (reverse)
                for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                    m_entries.push_back(std::move(it->second));
            else
                for (auto& e : sort)
                    m_entries.push_back(std::move(e.second));
        }

        break;
    }
    }
    FindClose(dir);

#else

    DIR* dir = opendir(path);
    if (!dir)
        return;
    const dirent* d;
    switch (mode)
    {
    case Mode::Native:
        while ((d = readdir(dir)))
        {
            if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                continue;
            HECL::SystemString fp(path);
            fp += '/';
            fp += d->d_name;
            HECL::Sstat st;
            if (HECL::Stat(fp.c_str(), &st))
                continue;

            size_t sz = 0;
            bool isDir = false;
            if (S_ISDIR(st.st_mode))
                isDir = true;
            else if (S_ISREG(st.st_mode))
                sz = st.st_size;
            else
                continue;

            m_entries.push_back(std::move(Entry(std::move(fp), d->d_name, sz, isDir)));
        }
        break;
    case Mode::DirsThenFilesSorted:
    case Mode::DirsSorted:
    {
        std::map<HECL::SystemString, Entry, CaseInsensitiveCompare> sort;
        while ((d = readdir(dir)))
        {
            if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                continue;
            HECL::SystemString fp(path);
            fp += '/';
            fp += d->d_name;
            HECL::Sstat st;
            if (HECL::Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode))
                continue;
            sort.emplace(std::make_pair(d->d_name, Entry(std::move(fp), d->d_name, 0, true)));
        }

        if (reverse)
            for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                m_entries.push_back(std::move(it->second));
        else
            for (auto& e : sort)
                m_entries.push_back(std::move(e.second));

        if (mode == Mode::DirsSorted)
            break;
        rewinddir(dir);
    }
    case Mode::FilesSorted:
    {
        if (mode == Mode::FilesSorted)
            m_entries.clear();

        if (sizeSort)
        {
            std::map<size_t, Entry> sort;
            while ((d = readdir(dir)))
            {
                if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    continue;
                HECL::SystemString fp(path);
                fp += '/';
                fp += d->d_name;
                HECL::Sstat st;
                if (HECL::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
                    continue;
                sort.emplace(std::make_pair(st.st_size, Entry(std::move(fp), d->d_name, st.st_size, false)));
            }

            if (reverse)
                for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                    m_entries.push_back(std::move(it->second));
            else
                for (auto& e : sort)
                    m_entries.push_back(std::move(e.second));
        }
        else
        {
            std::map<HECL::SystemString, Entry, CaseInsensitiveCompare> sort;
            while ((d = readdir(dir)))
            {
                if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    continue;
                HECL::SystemString fp(path);
                fp += '/';
                fp += d->d_name;
                HECL::Sstat st;
                if (HECL::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
                    continue;
                sort.emplace(std::make_pair(d->d_name, Entry(std::move(fp), d->d_name, st.st_size, false)));
            }

            if (reverse)
                for (auto it=sort.crbegin() ; it != sort.crend() ; ++it)
                    m_entries.push_back(std::move(it->second));
            else
                for (auto& e : sort)
                    m_entries.push_back(std::move(e.second));
        }

        break;
    }
    }
    closedir(dir);

#endif
}

}
