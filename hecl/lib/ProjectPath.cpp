#include "HECL/HECL.hpp"
#include <stdexcept>
#include <regex>

#if _WIN32
char* win_realpath(const char* name, char* restrict resolved)
{
}
#endif

namespace HECL
{

static const SystemRegex regGLOB(_S("\\*"), SystemRegex::ECMAScript|SystemRegex::optimize);
static const SystemRegex regPATHCOMP(_S("/([^/]+)"), SystemRegex::ECMAScript|SystemRegex::optimize);
static const SystemRegex regDRIVELETTER(_S("^([^/]*)/"), SystemRegex::ECMAScript|SystemRegex::optimize);

bool ProjectPath::_canonAbsPath(const SystemString& path)
{
#if _WIN32
#else
    SystemChar resolvedPath[PATH_MAX];
    if (!realpath(path.c_str(), resolvedPath))
    {
        throw std::invalid_argument("Unable to resolve '" + SystemUTF8View(path).utf8_str() +
                                    "' as a canonicalized path");
        return false;
    }
    m_absPath = resolvedPath;
#endif
    return true;
}

ProjectPath::ProjectPath(const ProjectRootPath& rootPath, const SystemString& path)
{
    _canonAbsPath(path);
    if (m_absPath.size() < ((ProjectPath&)rootPath).m_absPath.size() ||
        m_absPath.compare(0, ((ProjectPath&)rootPath).m_absPath.size(),
                          ((ProjectPath&)rootPath).m_absPath))
    {
        throw std::invalid_argument("'" + SystemUTF8View(m_absPath).utf8_str() + "' is not a subpath of '" +
                                    SystemUTF8View(((ProjectPath&)rootPath).m_absPath).utf8_str() + "'");
        return;
    }
    if (m_absPath.size() == ((ProjectPath&)rootPath).m_absPath.size())
    {
        /* Copies of the project root are permitted */
        return;
    }
    SystemString::iterator beginit = m_absPath.begin() + ((ProjectPath&)rootPath).m_absPath.size();
    if (*beginit == _S('/'))
        ++beginit;
    m_relPath = SystemString(beginit, m_absPath.end());
    m_hash = Hash(m_relPath);

#if HECL_UCS2
    m_utf8AbsPath = WideToUTF8(m_absPath);
    m_utf8RelPath = m_utf8AbsPath.c_str() + ((ProjectPath&)rootPath).m_utf8AbsPath.size();
#endif
}

ProjectPath::PathType ProjectPath::getPathType() const
{
    if (std::regex_search(m_absPath, regGLOB))
        return PT_GLOB;
#if _WIN32
#else
    struct stat theStat;
    if (stat(m_absPath.c_str(), &theStat))
        return PT_NONE;
    if (S_ISDIR(theStat.st_mode))
        return PT_DIRECTORY;
    if (S_ISREG(theStat.st_mode))
        return PT_FILE;
    return PT_NONE;
#endif
}

Time ProjectPath::getModtime() const
{
    struct stat theStat;
    time_t latestTime = 0;
    if (std::regex_search(m_absPath, regGLOB))
    {
        std::vector<SystemString> globReults;
        getGlobResults(globReults);
        for (SystemString& path : globReults)
        {
            if (!HECL::Stat(path.c_str(), &theStat))
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
    LogModule.report(LogVisor::Error, _S("invalid path type for computing modtime"));
    return Time();
}

static void _recursiveGlob(std::vector<SystemString>& outPaths,
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
        _recursiveGlob(outPaths, level+1, pathCompMatches, nextItStr, true);
        return;
    }

    /* Compile component into regex */
    SystemRegex regComp(comp, SystemRegex::ECMAScript);

#if _WIN32
#else
    DIR* dir = opendir(itStr.c_str());
    if (!dir)
        throw std::runtime_error("unable to open directory for traversal at '" + itStr + "'");

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
                _recursiveGlob(outPaths, level+1, pathCompMatches, nextItStr, true);
            else if (S_ISREG(theStat.st_mode))
                outPaths.push_back(nextItStr);
        }
    }
#endif
}

void ProjectPath::getGlobResults(std::vector<SystemString>& outPaths) const
{
#if _WIN32
    TSystemPath itStr;
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
        _recursiveGlob(outPaths, 1, pathCompMatches, itStr, false);
}

std::unique_ptr<ProjectRootPath> SearchForProject(const SystemString& path)
{
    ProjectRootPath testRoot(path);
    SystemString::const_iterator begin = testRoot.getAbsolutePath().begin();
    SystemString::const_iterator end = testRoot.getAbsolutePath().end();
    while (begin != end)
    {
        SystemString testPath(begin, end);
        SystemString testIndexPath = testPath + _S("/.hecl/beacon");
        struct stat theStat;
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
                return std::unique_ptr<ProjectRootPath>(new ProjectRootPath(testPath));
            }
        }

        while (begin != end && *(end-1) != _S('/') && *(end-1) != _S('\\'))
            --end;
        --end;
    }
    return std::unique_ptr<ProjectRootPath>();
}

}
