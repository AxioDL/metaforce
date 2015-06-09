#include "HECL.hpp"
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
        throw std::invalid_argument("Unable to resolve '" + CSystemUTF8View(path).utf8_str() +
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
        throw std::invalid_argument("'" + CSystemUTF8View(m_absPath).utf8_str() + "' is not a subpath of '" +
                                    CSystemUTF8View(((ProjectPath&)rootPath).m_absPath).utf8_str() + "'");
        return;
    }
    if (m_absPath.size() == ((ProjectPath&)rootPath).m_absPath.size())
    {
        /* Copies of the project root are permitted */
        return;
    }
    m_relPath = m_absPath.c_str() + ((ProjectPath&)rootPath).m_absPath.size();
    if (m_relPath[0] == _S('/'))
        ++m_relPath;
    if (m_relPath[0] == _S('\0'))
        m_relPath = NULL;
}

ProjectPath::PathType ProjectPath::getPathType()
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

void ProjectPath::getGlobResults(std::vector<SystemString>& outPaths)
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

}
