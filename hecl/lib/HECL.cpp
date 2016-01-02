#include "HECL/HECL.hpp"

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
                                               bool sizeSort, bool reverse, bool noHidden)
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
            if (noHidden && (d.cFileName == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
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
            if (noHidden && (d.cFileName == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
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
                if (noHidden && (d.cFileName == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
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
                if (noHidden && (d.cFileName == L'.' || (d.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) != 0))
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
            if (noHidden && d->d_name[0] == '.')
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
            if (noHidden && d->d_name[0] == '.')
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
                if (noHidden && d->d_name[0] == '.')
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
                if (noHidden && d->d_name[0] == '.')
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

std::vector<HECL::SystemString> GetSystemLocations()
{
    HECL::Sstat theStat;
    std::vector<HECL::SystemString> ret;
#ifdef WIN32
    /* Add the drive names to the listing */
    {
        wchar_t wline[FILE_MAXDIR];
        __int64 tmp;
        char tmps[4], *name;
        int i;

        tmp = GetLogicalDrives();

        for (i = 0; i < 26; i++) {
            if ((tmp >> i) & 1) {
                tmps[0] = 'A' + i;
                tmps[1] = ':';
                tmps[2] = '\\';
                tmps[3] = '\0';
                name = NULL;

                /* Flee from horrible win querying hover floppy drives! */
                if (i > 1) {
                    /* Try to get volume label as well... */
                    BLI_strncpy_wchar_from_utf8(wline, tmps, 4);
                    if (GetVolumeInformationW(wline, wline + 4, FILE_MAXDIR - 4, NULL, NULL, NULL, NULL, 0)) {
                        size_t label_len;

                        BLI_strncpy_wchar_as_utf8(line, wline + 4, FILE_MAXDIR - 4);

                        label_len = MIN2(strlen(line), FILE_MAXDIR - 6);
                        BLI_snprintf(line + label_len, 6, " (%.2s)", tmps);

                        name = line;
                    }
                }

                fsmenu_insert_entry(fsmenu, FS_CATEGORY_SYSTEM, tmps, name, FS_INSERT_SORTED);
            }
        }

        /* Adding Desktop and My Documents */
        if (read_bookmarks) {
            SHGetSpecialFolderPathW(0, wline, CSIDL_PERSONAL, 0);
            BLI_strncpy_wchar_as_utf8(line, wline, FILE_MAXDIR);
            fsmenu_insert_entry(fsmenu, FS_CATEGORY_SYSTEM_BOOKMARKS, line, NULL, FS_INSERT_SORTED);
            SHGetSpecialFolderPathW(0, wline, CSIDL_DESKTOPDIRECTORY, 0);
            BLI_strncpy_wchar_as_utf8(line, wline, FILE_MAXDIR);
            fsmenu_insert_entry(fsmenu, FS_CATEGORY_SYSTEM_BOOKMARKS, line, NULL, FS_INSERT_SORTED);
        }
    }
#else
#ifdef __APPLE__
    {
        /* Get mounted volumes better method OSX 10.6 and higher, see: */
        /*https://developer.apple.com/library/mac/#documentation/CoreFOundation/Reference/CFURLRef/Reference/reference.html*/
        /* we get all volumes sorted including network and do not relay on user-defined finder visibility, less confusing */

        CFURLRef cfURL = NULL;
        CFURLEnumeratorResult result = kCFURLEnumeratorSuccess;
        CFURLEnumeratorRef volEnum = CFURLEnumeratorCreateForMountedVolumes(NULL, kCFURLEnumeratorSkipInvisibles, NULL);

        while (result != kCFURLEnumeratorEnd) {
            unsigned char defPath[FILE_MAX];

            result = CFURLEnumeratorGetNextURL(volEnum, &cfURL, NULL);
            if (result != kCFURLEnumeratorSuccess)
                continue;

            CFURLGetFileSystemRepresentation(cfURL, false, (UInt8 *)defPath, FILE_MAX);
            fsmenu_insert_entry(fsmenu, FS_CATEGORY_SYSTEM, (char *)defPath, NULL, FS_INSERT_SORTED);
        }

        CFRelease(volEnum);

        /* Finally get user favorite places */
        if (read_bookmarks) {
            UInt32 seed;
            OSErr err = noErr;
            CFArrayRef pathesArray;
            LSSharedFileListRef list;
            LSSharedFileListItemRef itemRef;
            CFIndex i, pathesCount;
            CFURLRef cfURL = NULL;
            CFStringRef pathString = NULL;
            list = LSSharedFileListCreate(NULL, kLSSharedFileListFavoriteItems, NULL);
            pathesArray = LSSharedFileListCopySnapshot(list, &seed);
            pathesCount = CFArrayGetCount(pathesArray);

            for (i = 0; i < pathesCount; i++) {
                itemRef = (LSSharedFileListItemRef)CFArrayGetValueAtIndex(pathesArray, i);

                err = LSSharedFileListItemResolve(itemRef,
                                                  kLSSharedFileListNoUserInteraction |
                                                  kLSSharedFileListDoNotMountVolumes,
                                                  &cfURL, NULL);
                if (err != noErr)
                    continue;

                pathString = CFURLCopyFileSystemPath(cfURL, kCFURLPOSIXPathStyle);

                if (pathString == NULL || !CFStringGetCString(pathString, line, sizeof(line), kCFStringEncodingUTF8))
                    continue;

                /* Exclude "all my files" as it makes no sense in blender fileselector */
                /* Exclude "airdrop" if wlan not active as it would show "" ) */
                if (!strstr(line, "myDocuments.cannedSearch") && (*line != '\0')) {
                    fsmenu_insert_entry(fsmenu, FS_CATEGORY_SYSTEM_BOOKMARKS, line, NULL, FS_INSERT_LAST);
                }

                CFRelease(pathString);
                CFRelease(cfURL);
            }

            CFRelease(pathesArray);
            CFRelease(list);
        }
    }
#else
    /* unix */
    {
        const char* home = getenv("HOME");

        if (home)
        {
            ret.push_back(home);
            std::string desktop(home);
            desktop += "/Desktop";
            if (!HECL::Stat(desktop.c_str(), &theStat))
                ret.push_back(std::move(desktop));
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
                    ret.push_back(std::move(mntStr));

                    found = true;
                }
                endmntent(fp);
            }
#endif
            /* Fallback */
            if (!found)
                ret.push_back("/");
        }
    }
#endif
#endif
    return ret;
}

}
