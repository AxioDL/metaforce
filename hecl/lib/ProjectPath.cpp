#include "HECL/HECL.hpp"
#include "HECL/Database.hpp"
#include <regex>

namespace HECL
{
static const SystemRegex regGLOB(_S("\\*"), SystemRegex::ECMAScript|SystemRegex::optimize);
static const SystemRegex regPATHCOMP(_S("[/\\\\]*([^/\\\\]+)"), SystemRegex::ECMAScript|SystemRegex::optimize);
static const SystemRegex regDRIVELETTER(_S("^([^/]*)/"), SystemRegex::ECMAScript|SystemRegex::optimize);

static bool IsAbsolute(const SystemString& path)
{
#if WIN32
    if (path.size() && (path[0] == _S('\\') || path[0] == _S('/')))
        return true;
    if (path.size() >= 2 && iswalpha(path[0]) && path[1] == _S(':'))
        return true;
#else
    if (path[0] == _S('/'))
        return true;
#endif
    return false;
}

static SystemString CanonRelPath(const SystemString& path)
{
    /* Tokenize Path */
    std::vector<SystemString> comps;
    HECL::SystemRegexMatch matches;
    SystemString in = path;
    SanitizePath(in);
    for (; std::regex_search(in, matches, regPATHCOMP) ; in = matches.suffix())
    {
        const SystemString& match = matches[1];
        if (!match.compare(_S(".")))
            continue;
        else if (!match.compare(_S("..")))
        {
            if (comps.empty())
            {
                /* Unable to resolve outside project */
                LogModule.report(LogVisor::FatalError, _S("Unable to resolve outside project root in %s"), path.c_str());
                return _S(".");
            }
            comps.pop_back();
            continue;
        }
        comps.push_back(match);
    }

    /* Emit relative path */
    if (comps.size())
    {
        auto it = comps.begin();
        SystemString retval = *it;
        for (++it ; it != comps.end() ; ++it)
        {
            if ((*it).size())
            {
                retval += _S('/');
                retval += *it;
            }
        }
        return retval;
    }
    return _S(".");
}

static SystemString CanonRelPath(const SystemString& path, const ProjectRootPath& projectRoot)
{
    /* Absolute paths not allowed; attempt to make project-relative */
    if (IsAbsolute(path))
        return CanonRelPath(projectRoot.getProjectRelativeFromAbsolute(path));
    return CanonRelPath(path);
}

void ProjectPath::assign(Database::Project& project, const SystemString& path)
{
    m_proj = &project;
    m_relPath = CanonRelPath(path);
    m_absPath = project.getProjectRootPath().getAbsolutePath() + _S('/') + m_relPath;
    SanitizePath(m_relPath);
    SanitizePath(m_absPath);
    m_hash = Hash(m_relPath);

#if HECL_UCS2
    m_utf8AbsPath = WideToUTF8(m_absPath);
    m_utf8RelPath = WideToUTF8(m_relPath);
#endif
}

#if HECL_UCS2
void ProjectPath::assign(Database::Project& project, const std::string& path)
{
    m_proj = &project;
    std::wstring wpath = UTF8ToWide(path);
    m_relPath = CanonRelPath(wpath);
    m_absPath = project.getProjectRootPath().getAbsolutePath() + _S('/') + m_relPath;
    SanitizePath(m_relPath);
    SanitizePath(m_absPath);
    m_hash = Hash(m_relPath);
    m_utf8AbsPath = WideToUTF8(m_absPath);
    m_utf8RelPath = WideToUTF8(m_relPath);
}
#endif

void ProjectPath::assign(const ProjectPath& parentPath, const SystemString& path)
{
    m_proj = parentPath.m_proj;
    m_relPath = CanonRelPath(parentPath.m_relPath + _S('/') + path);
    m_absPath = m_proj->getProjectRootPath().getAbsolutePath() + _S('/') + m_relPath;
    SanitizePath(m_relPath);
    SanitizePath(m_absPath);
    m_hash = Hash(m_relPath);

#if HECL_UCS2
    m_utf8AbsPath = WideToUTF8(m_absPath);
    m_utf8RelPath = WideToUTF8(m_relPath);
#endif
}

#if HECL_UCS2
void ProjectPath::assign(const ProjectPath& parentPath, const std::string& path)
{
    m_proj = parentPath.m_proj;
    std::wstring wpath = UTF8ToWide(path);
    m_relPath = CanonRelPath(parentPath.m_relPath + _S('/') + wpath);
    m_absPath = m_proj->getProjectRootPath().getAbsolutePath() + _S('/') + m_relPath;
    SanitizePath(m_relPath);
    SanitizePath(m_absPath);
    m_hash = Hash(m_relPath);
    m_utf8AbsPath = WideToUTF8(m_absPath);
    m_utf8RelPath = WideToUTF8(m_relPath);
}
#endif

ProjectPath ProjectPath::getCookedPath(const Database::DataSpecEntry& spec) const
{
    ProjectPath woExt = getWithExtension(nullptr, true);
    return ProjectPath(m_proj->getProjectCookedPath(spec), woExt.getRelativePath());
}

ProjectPath::Type ProjectPath::getPathType() const
{
#if WIN32
    if (TestShellLink(m_absPath.c_str()))
        return Type::Link;
#else
    HECL::Sstat lnStat;
    if (lstat(m_absPath.c_str(), &lnStat))
        return Type::None;
    if (S_ISLNK(lnStat.st_mode))
        return Type::Link;
#endif
    if (std::regex_search(m_absPath, regGLOB))
        return Type::Glob;
    Sstat theStat;
    if (HECL::Stat(m_absPath.c_str(), &theStat))
        return Type::None;
    if (S_ISDIR(theStat.st_mode))
        return Type::Directory;
    if (S_ISREG(theStat.st_mode))
        return Type::File;
    return Type::None;
}

Time ProjectPath::getModtime() const
{
    Sstat theStat;
    time_t latestTime = 0;
    if (std::regex_search(m_absPath, regGLOB))
    {
        std::vector<ProjectPath> globResults;
        getGlobResults(globResults);
        for (ProjectPath& path : globResults)
        {
            if (!HECL::Stat(path.getAbsolutePath().c_str(), &theStat))
            {
                if (S_ISREG(theStat.st_mode) && theStat.st_mtime > latestTime)
                    latestTime = theStat.st_mtime;
            }
        }
    }
    if (!HECL::Stat(m_absPath.c_str(), &theStat))
    {
        if (S_ISREG(theStat.st_mode))
        {
            return Time(theStat.st_mtime);
        }
        else if (S_ISDIR(theStat.st_mode))
        {
#if _WIN32
#else
            DIR* dir = opendir(m_absPath.c_str());
            dirent* de;
            while ((de = readdir(dir)))
            {
                if (de->d_name[0] == '.')
                    continue;
                if (!HECL::Stat(de->d_name, &theStat))
                {
                    if (S_ISREG(theStat.st_mode) && theStat.st_mtime > latestTime)
                        latestTime = theStat.st_mtime;
                }
            }
            closedir(dir);
#endif
            return Time(latestTime);
        }
    }
    LogModule.report(LogVisor::FatalError, _S("invalid path type for computing modtime in '%s'"), m_absPath.c_str());
    return Time();
}

ProjectPath ProjectPath::resolveLink() const
{
#if WIN32
    wchar_t target[2048];
    if (FAILED(ResolveShellLink(m_absPath.c_str(), target, 2048)))
        LogModule.report(LogVisor::FatalError, _S("unable to resolve link '%s'"), m_absPath.c_str());
#else
    char target[2048];
    ssize_t targetSz;
    if ((targetSz = readlink(m_absPath.c_str(), target, 2048)) < 0)
        LogModule.report(LogVisor::FatalError, _S("unable to resolve link '%s': %s"), m_absPath.c_str(), strerror(errno));
    target[targetSz] = '\0';
#endif
    return ProjectPath(getParentPath(), target);
}

static void _recursiveGlob(Database::Project& proj,
                           std::vector<ProjectPath>& outPaths,
                           size_t level,
                           const SystemRegexMatch& pathCompMatches,
                           const SystemString& itStr,
                           bool needSlash)
{
    if (level >= pathCompMatches.size())
        return;

    SystemString comp = pathCompMatches.str(level);
    if (!std::regex_search(comp, regGLOB))
    {
        SystemString nextItStr = itStr;
        if (needSlash)
            nextItStr += _S('/');
        nextItStr += comp;
        _recursiveGlob(proj, outPaths, level+1, pathCompMatches, nextItStr, true);
        return;
    }

    /* Compile component into regex */
    SystemRegex regComp(comp, SystemRegex::ECMAScript);

#if _WIN32
#else
    DIR* dir = opendir(itStr.c_str());
    if (!dir)
    {
        LogModule.report(LogVisor::Error, "unable to open directory for traversal at '%s'", itStr.c_str());
        return;
    }

    struct dirent* de;
    while ((de = readdir(dir)))
    {
        if (std::regex_search(de->d_name, regComp))
        {
            SystemString nextItStr = itStr;
            if (needSlash)
                nextItStr += '/';
            nextItStr += de->d_name;

            struct stat theStat;
            if (stat(nextItStr.c_str(), &theStat))
                continue;

            if (S_ISDIR(theStat.st_mode))
                _recursiveGlob(proj, outPaths, level+1, pathCompMatches, nextItStr, true);
            else if (S_ISREG(theStat.st_mode))
                outPaths.emplace_back(proj, nextItStr);
        }
    }

    closedir(dir);
#endif
}

void ProjectPath::getDirChildren(std::map<SystemString, ProjectPath>& outPaths) const
{
#if _WIN32
#else
    struct dirent* de;
    DIR* dir = opendir(m_absPath.c_str());
    if (!dir)
    {
        LogModule.report(LogVisor::Error, "unable to open directory for traversal at '%s'", m_absPath.c_str());
        return;
    }

    /* Add elements */
    rewinddir(dir);
    while ((de = readdir(dir)))
    {
        if (!strcmp(de->d_name, "."))
            continue;
        if (!strcmp(de->d_name, ".."))
            continue;
        outPaths[de->d_name] = ProjectPath(*this, de->d_name);
    }

    closedir(dir);
#endif
}

void ProjectPath::getGlobResults(std::vector<ProjectPath>& outPaths) const
{
#if _WIN32
    SystemString itStr;
    SystemRegexMatch letterMatch;
    if (m_absPath.compare(0, 2, _S("//")))
        itStr = _S("\\\\");
    else if (std::regex_search(m_absPath, letterMatch, regDRIVELETTER))
        if (letterMatch[1].str().size())
            itStr = letterMatch[1];
#else
    SystemString itStr = _S("/");
#endif

    SystemRegexMatch pathCompMatches;
    if (std::regex_search(m_absPath, pathCompMatches, regPATHCOMP))
        _recursiveGlob(*m_proj, outPaths, 1, pathCompMatches, itStr, false);
}

ProjectRootPath SearchForProject(const SystemString& path)
{
    ProjectRootPath testRoot(path);
    SystemString::const_iterator begin = testRoot.getAbsolutePath().begin();
    SystemString::const_iterator end = testRoot.getAbsolutePath().end();
    while (begin != end)
    {
        SystemString testPath(begin, end);
        SystemString testIndexPath = testPath + _S("/.hecl/beacon");
        Sstat theStat;
        if (!HECL::Stat(testIndexPath.c_str(), &theStat))
        {
            if (S_ISREG(theStat.st_mode))
            {
                FILE* fp = HECL::Fopen(testIndexPath.c_str(), _S("rb"));
                if (!fp)
                    continue;
                char magic[4];
                size_t readSize = fread(magic, 1, 4, fp);
                fclose(fp);
                if (readSize != 4)
                    continue;
                static const HECL::FourCC hecl("HECL");
                if (HECL::FourCC(magic) != hecl)
                    continue;
                return ProjectRootPath(testPath);
            }
        }

        while (begin != end && *(end-1) != _S('/') && *(end-1) != _S('\\'))
            --end;
        if (begin != end)
            --end;
    }
    return ProjectRootPath();
}

ProjectRootPath SearchForProject(const SystemString& path, SystemString& subpathOut)
{
    ProjectRootPath testRoot(path);
    SystemString::const_iterator begin = testRoot.getAbsolutePath().begin();
    SystemString::const_iterator end = testRoot.getAbsolutePath().end();
    while (begin != end)
    {
        SystemString testPath(begin, end);
        SystemString testIndexPath = testPath + _S("/.hecl/beacon");
        Sstat theStat;
        if (!HECL::Stat(testIndexPath.c_str(), &theStat))
        {
            if (S_ISREG(theStat.st_mode))
            {
                FILE* fp = HECL::Fopen(testIndexPath.c_str(), _S("rb"));
                if (!fp)
                    continue;
                char magic[4];
                size_t readSize = fread(magic, 1, 4, fp);
                fclose(fp);
                if (readSize != 4)
                    continue;
                if (HECL::FourCC(magic) != FOURCC('HECL'))
                    continue;
                ProjectRootPath newRootPath = ProjectRootPath(testPath);
                SystemString::const_iterator origEnd = testRoot.getAbsolutePath().end();
                while (end != origEnd && *end != _S('/') && *end != _S('\\'))
                    ++end;
                subpathOut.assign(end, origEnd);
                return newRootPath;
            }
        }

        while (begin != end && *(end-1) != _S('/') && *(end-1) != _S('\\'))
            --end;
        if (begin != end)
            --end;
    }
    return ProjectRootPath();
}

}
