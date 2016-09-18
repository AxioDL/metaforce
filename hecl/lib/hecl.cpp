#include "hecl/hecl.hpp"
#include <thread>
#include <mutex>
#include <unordered_map>

#ifdef WIN32
#include <windows.h>
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400
#endif
#include <shlobj.h>
#endif

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#ifdef __linux__
#include <mntent.h>
#endif

namespace hecl
{
unsigned VerbosityLevel = 0;
logvisor::Module LogModule("hecl");
static const std::string Illegals {"<>?\"|"};

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
        if (Illegals.find_first_of(a) != std::string::npos)
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

static const std::wstring WIllegals {L"<>?*\"|"};

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
        if (WIllegals.find_first_of(a) != std::wstring::npos)
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

static std::mutex PathsMutex;
static std::unordered_map<std::thread::id, ProjectPath> PathsInProgress;

bool ResourceLock::InProgress(const ProjectPath& path)
{
    std::unique_lock<std::mutex> lk(PathsMutex);
    for (const auto& p : PathsInProgress)
        if (p.second == path)
            return true;
    return false;
}

bool ResourceLock::SetThreadRes(const ProjectPath& path)
{
    std::unique_lock<std::mutex> lk(PathsMutex);
    if (PathsInProgress.find(std::this_thread::get_id()) != PathsInProgress.cend())
        LogModule.report(logvisor::Fatal, "multiple resource locks on thread");

    for (const auto& p : PathsInProgress)
        if (p.second == path)
            return false;

    PathsInProgress[std::this_thread::get_id()] = path;
    return true;
}

void ResourceLock::ClearThreadRes()
{
    std::unique_lock<std::mutex> lk(PathsMutex);
    PathsInProgress.erase(std::this_thread::get_id());
}

bool IsPathPNG(const hecl::ProjectPath& path)
{
    FILE* fp = hecl::Fopen(path.getAbsolutePath().c_str(), _S("rb"));
    if (!fp)
        return false;
    uint32_t buf;
    if (fread(&buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    buf = hecl::SBig(buf);
    if (buf == 0x89504e47)
        return true;
    return false;
}

bool IsPathBlend(const hecl::ProjectPath& path)
{
    hecl::ProjectPath usePath;
    if (path.getPathType() == hecl::ProjectPath::Type::Glob)
        usePath = path.getWithExtension(_S(".blend"), true);
    else
        usePath = path;

    const SystemChar* lastCompExt = usePath.getLastComponentExt();
    if (!lastCompExt || hecl::StrCmp(lastCompExt, _S("blend")))
        return false;
    FILE* fp = hecl::Fopen(usePath.getAbsolutePath().c_str(), _S("rb"));
    if (!fp)
        return false;
    uint32_t buf;
    if (fread(&buf, 1, 4, fp) != 4)
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    buf = hecl::SLittle(buf);
    if (buf == 0x4e454c42 || buf == 0x88b1f)
        return true;
    return false;
}

bool IsPathYAML(const hecl::ProjectPath& path)
{
    const SystemChar* lastCompExt = path.getLastComponentExt();
    if (!lastCompExt)
        return false;
    if (!hecl::StrCmp(lastCompExt, _S("yaml")) ||
        !hecl::StrCmp(lastCompExt, _S("yml")))
        return true;
    return false;
}

hecl::DirectoryEnumerator::DirectoryEnumerator(const hecl::SystemChar* path, Mode mode,
                                               bool sizeSort, bool reverse, bool noHidden)
{
    hecl::Sstat theStat;
    if (hecl::Stat(path, &theStat) || !S_ISDIR(theStat.st_mode))
        return;

#if _WIN32
    hecl::SystemString wc(path);
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
            if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
                continue;
            hecl::SystemString fp(path);
            fp += _S('/');
            fp += d.cFileName;
            hecl::Sstat st;
            if (hecl::Stat(fp.c_str(), &st))
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
        std::map<hecl::SystemString, Entry, CaseInsensitiveCompare> sort;
        do
        {
            if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                continue;
            if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
                continue;
            hecl::SystemString fp(path);
            fp +=_S('/');
            fp += d.cFileName;
            hecl::Sstat st;
            if (hecl::Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode))
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
            std::multimap<size_t, Entry> sort;
            do
            {
                if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                    continue;
                if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
                    continue;
                hecl::SystemString fp(path);
                fp += _S('/');
                fp += d.cFileName;
                hecl::Sstat st;
                if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
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
            std::map<hecl::SystemString, Entry, CaseInsensitiveCompare> sort;
            do
            {
                if (!wcscmp(d.cFileName, _S(".")) || !wcscmp(d.cFileName, _S("..")))
                    continue;
                if (noHidden && (d.cFileName[0] == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
                    continue;
                hecl::SystemString fp(path);
                fp += _S('/');
                fp += d.cFileName;
                hecl::Sstat st;
                if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
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
            if (noHidden && d->d_name[0] == '.')
                continue;
            hecl::SystemString fp(path);
            fp += '/';
            fp += d->d_name;
            hecl::Sstat st;
            if (hecl::Stat(fp.c_str(), &st))
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
        std::map<hecl::SystemString, Entry, CaseInsensitiveCompare> sort;
        while ((d = readdir(dir)))
        {
            if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                continue;
            if (noHidden && d->d_name[0] == '.')
                continue;
            hecl::SystemString fp(path);
            fp += '/';
            fp += d->d_name;
            hecl::Sstat st;
            if (hecl::Stat(fp.c_str(), &st) || !S_ISDIR(st.st_mode))
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
            std::multimap<size_t, Entry> sort;
            while ((d = readdir(dir)))
            {
                if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    continue;
                if (noHidden && d->d_name[0] == '.')
                    continue;
                hecl::SystemString fp(path);
                fp += '/';
                fp += d->d_name;
                hecl::Sstat st;
                if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
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
            std::map<hecl::SystemString, Entry, CaseInsensitiveCompare> sort;
            while ((d = readdir(dir)))
            {
                if (!strcmp(d->d_name, ".") || !strcmp(d->d_name, ".."))
                    continue;
                if (noHidden && d->d_name[0] == '.')
                    continue;
                hecl::SystemString fp(path);
                fp += '/';
                fp += d->d_name;
                hecl::Sstat st;
                if (hecl::Stat(fp.c_str(), &st) || !S_ISREG(st.st_mode))
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

#define FILE_MAXDIR 768

static std::pair<hecl::SystemString, std::string>
NameFromPath(const hecl::SystemString& path)
{
    hecl::SystemUTF8View utf8(path);
    if (utf8.str().size() == 1 && utf8.str()[0] == '/')
        return {path, "/"};
    size_t lastSlash = utf8.str().rfind('/');
    if (lastSlash != std::string::npos)
        return {path, std::string(utf8.str().cbegin() + lastSlash + 1, utf8.str().cend())};
    else
        return {path, utf8.str()};
}

std::vector<std::pair<hecl::SystemString, std::string>> GetSystemLocations()
{
    std::vector<std::pair<hecl::SystemString, std::string>> ret;
#ifdef WIN32
    /* Add the drive names to the listing (as queried by blender) */
    {
        wchar_t wline[FILE_MAXDIR];
        wchar_t* name;
        __int64 tmp;
        int i;

        tmp = GetLogicalDrives();

        for (i = 0; i < 26; i++)
        {
            if ((tmp >> i) & 1)
            {
                wline[0] = L'A' + i;
                wline[1] = L':';
                wline[2] = L'/';
                wline[3] = L'\0';
                name = nullptr;

                /* Flee from horrible win querying hover floppy drives! */
                if (i > 1)
                {
                    /* Try to get volume label as well... */
                    if (GetVolumeInformationW(wline, wline + 4, FILE_MAXDIR - 4, nullptr, nullptr, nullptr, nullptr, 0))
                    {
                        size_t labelLen = wcslen(wline + 4);
                        _snwprintf(wline + 4 + labelLen, FILE_MAXDIR - 4 - labelLen, L" (%.2s)", wline);
                        name = wline + 4;
                    }
                }

                wline[2] = L'\0';
                if (name)
                    ret.emplace_back(wline, hecl::WideToUTF8(name));
                else
                    ret.push_back(NameFromPath(wline));
            }
        }

        /* Adding Desktop and My Documents */
        SystemString wpath;
        SHGetSpecialFolderPathW(0, wline, CSIDL_PERSONAL, 0);
        wpath.assign(wline);
        SanitizePath(wpath);
        ret.push_back(NameFromPath(wpath));
        SHGetSpecialFolderPathW(0, wline, CSIDL_DESKTOPDIRECTORY, 0);
        wpath.assign(wline);
        SanitizePath(wpath);
        ret.push_back(NameFromPath(wpath));
    }
#else
#ifdef __APPLE__
    {
        hecl::Sstat theStat;
        const char* home = getenv("HOME");
        
        if (home)
        {
            ret.push_back(NameFromPath(home));
            std::string desktop(home);
            desktop += "/Desktop";
            if (!hecl::Stat(desktop.c_str(), &theStat))
                ret.push_back(NameFromPath(desktop));
        }
        
        /* Get mounted volumes better method OSX 10.6 and higher, see: */
        /*https://developer.apple.com/library/mac/#documentation/CoreFOundation/Reference/CFURLRef/Reference/reference.html*/
        /* we get all volumes sorted including network and do not relay on user-defined finder visibility, less confusing */

        CFURLRef cfURL = NULL;
        CFURLEnumeratorResult result = kCFURLEnumeratorSuccess;
        CFURLEnumeratorRef volEnum = CFURLEnumeratorCreateForMountedVolumes(NULL, kCFURLEnumeratorSkipInvisibles, NULL);

        while (result != kCFURLEnumeratorEnd)
        {
            char defPath[MAXPATHLEN];

            result = CFURLEnumeratorGetNextURL(volEnum, &cfURL, NULL);
            if (result != kCFURLEnumeratorSuccess)
                continue;

            CFURLGetFileSystemRepresentation(cfURL, false, (UInt8 *)defPath, MAXPATHLEN);
            ret.push_back(NameFromPath(defPath));
        }

        CFRelease(volEnum);
    }
#else
    /* unix */
    {
        hecl::Sstat theStat;
        const char* home = getenv("HOME");

        if (home)
        {
            ret.push_back(NameFromPath(home));
            std::string desktop(home);
            desktop += "/Desktop";
            if (!hecl::Stat(desktop.c_str(), &theStat))
                ret.push_back(NameFromPath(desktop));
        }

        {
            bool found = false;
#ifdef __linux__
            /* Loop over mount points */
            struct mntent *mnt;

            FILE* fp = setmntent(MOUNTED, "r");
            if (fp)
            {
                while ((mnt = getmntent(fp)))
                {
                    if (strlen(mnt->mnt_fsname) < 4 || strncmp(mnt->mnt_fsname, "/dev", 4))
                        continue;

                    std::string mntStr(mnt->mnt_dir);
                    if (mntStr.size() > 1 && mntStr.back() == '/')
                        mntStr.pop_back();
                    ret.push_back(NameFromPath(mntStr));

                    found = true;
                }
                endmntent(fp);
            }
#endif
            /* Fallback */
            if (!found)
                ret.push_back(NameFromPath("/"));
        }
    }
#endif
#endif
    return ret;
}

}
