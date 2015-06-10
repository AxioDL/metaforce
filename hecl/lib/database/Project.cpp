#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <system_error>

#if _WIN32
#else
#include <unistd.h>
#endif

#include "HECLDatabase.hpp"

namespace HECL
{
namespace Database
{

/**********************************************
 * Project::ConfigFile
 **********************************************/

static inline bool CheckNewLineAdvance(std::string::const_iterator& it)
{
    if (*it == '\n')
    {
        it += 1;
        return true;
    }
    else if (*it == '\r')
    {
        if (*(it+1) == '\n')
        {
            it += 2;
            return true;
        }
        it += 1;
        return true;
    }
    return false;
}

Project::ConfigFile::ConfigFile(const Project& project, const SystemString& name)
{
    m_filepath = project.m_rootPath.getAbsolutePath() + _S("/.hecl/config/") + name;
}

const std::vector<std::string>& Project::ConfigFile::lockAndRead()
{
    if (m_lockedFile)
        return m_lines;

    m_lockedFile = HECL::Fopen(m_filepath.c_str(), _S("r+"), LWRITE);

    std::string mainString;
    char readBuf[1024];
    size_t readSz;
    while ((readSz = fread(readBuf, 1, 1024, m_lockedFile)))
        mainString += std::string(readBuf, readSz);

    std::string::const_iterator begin = mainString.begin();
    std::string::const_iterator end = mainString.begin();

    m_lines.clear();
    while (end != mainString.end())
    {
        std::string::const_iterator origEnd = end;
        if (*end == '\0')
            break;
        else if (CheckNewLineAdvance(end))
        {
            if (begin != origEnd)
                m_lines.push_back(std::string(begin, origEnd));
            begin = end;
            continue;
        }
        ++end;
    }
    if (begin != end)
        m_lines.push_back(std::string(begin, end));

    return m_lines;
}

void Project::ConfigFile::addLine(const std::string& line)
{
    if (!checkForLine(line))
        m_lines.push_back(line);
}

void Project::ConfigFile::removeLine(const std::string& refLine)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::ConfigFile::lockAndRead not yet called"));

    for (std::vector<std::string>::const_iterator it=m_lines.begin();
         it != m_lines.end();
         ++it)
    {
        if (!(*it).compare(refLine))
            it = m_lines.erase(it);
    }
}

bool Project::ConfigFile::checkForLine(const std::string& refLine)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::ConfigFile::lockAndRead not yet called"));

    for (const std::string& line : m_lines)
    {
        if (!line.compare(refLine))
            return true;
    }
    return false;
}

void Project::ConfigFile::unlockAndDiscard()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::ConfigFile::lockAndRead not yet called"));

    m_lines.clear();
    fclose(m_lockedFile);
    m_lockedFile = NULL;
}

void Project::ConfigFile::unlockAndCommit()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::ConfigFile::lockAndRead not yet called"));

    fseek(m_lockedFile, 0, SEEK_SET);
#if _WIN32
    SetEndOfFile((HANDLE)fileno(m_lockedFile));
#else
    ftruncate(fileno(m_lockedFile), 0);
#endif
    for (const std::string& line : m_lines)
    {
        fwrite(line.c_str(), 1, line.size(), m_lockedFile);
        fwrite("\n", 1, 1, m_lockedFile);
    }
    m_lines.clear();
    fclose(m_lockedFile);
    m_lockedFile = NULL;
}

/**********************************************
 * Project::IndexFile
 **********************************************/

struct SIndexHeader
{
    HECL::FourCC magic;
    uint32_t version;
    uint32_t entryCount;
    uint32_t maxPathLen;
    void swapWithNative()
    {
        version = ToBig(version);
        entryCount = ToBig(entryCount);
        maxPathLen = ToBig(maxPathLen);
    }
};

Project::IndexFile::IndexFile(const Project& project)
: m_project(project)
{
    m_filepath = project.m_rootPath.getAbsolutePath() + _S("/.hecl/index");
}

const std::vector<Project::IndexFile::Entry>& Project::IndexFile::lockAndRead()
{
    if (m_lockedFile)
        return m_entryStore;

    m_lockedFile = HECL::Fopen(m_filepath.c_str(), _S("r+"), LWRITE);

    SIndexHeader header;
    if (fread(&header, 1, sizeof(header), m_lockedFile) != sizeof(header))
        return m_entryStore;
    header.swapWithNative();
    if (header.magic != "HECL")
        throw HECL::Exception(_S("unrecognized HECL index"));
    if (header.version != 1)
        throw HECL::Exception(_S("unrecognized HECL version"));

    char* pathBuf = new char[header.maxPathLen];
    for (uint32_t e=0 ; e<header.entryCount ; ++e)
    {
        uint64_t mt;
        fread(&mt, 1, 8, m_lockedFile);
        mt = ToBig(mt);
        uint32_t strLen;
        fread(&strLen, 1, 4, m_lockedFile);
        strLen = ToBig(strLen);
        if (strLen > m_maxPathLen)
            m_maxPathLen = strLen;
        fread(pathBuf, 1, strLen, m_lockedFile);
        std::string pathStr(pathBuf, strLen);
        SystemStringView pathView(pathStr);
        ProjectPath path(m_project.getProjectRootPath(), pathView.sys_str());
        if (m_entryLookup.find(path) == m_entryLookup.end())
        {
            m_entryStore.push_back(Project::IndexFile::Entry(path, mt));
            m_entryLookup[path] = &m_entryStore.back();
        }
    }
    delete[] pathBuf;
}

const std::vector<ProjectPath*> Project::IndexFile::getChangedPaths()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    std::vector<ProjectPath*> retval;
    for (Project::IndexFile::Entry& ent : m_entryStore)
    {
        if (ent.m_removed)
            continue;
        if (ent.m_lastModtime != ent.m_path.getModtime())
            retval.push_back(&ent.m_path);
    }
    return retval;
}

void Project::IndexFile::addOrUpdatePath(const ProjectPath& path)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    std::unordered_map<ProjectPath, Entry*>::iterator it = m_entryLookup.find(path);
    if (it == m_entryLookup.end())
    {
        m_entryStore.push_back(Project::IndexFile::Entry(path, path.getModtime()));
        m_entryLookup[path] = &m_entryStore.back();
        return;
    }
    (*it).second->m_lastModtime = path.getModtime();
}

void Project::IndexFile::removePath(const ProjectPath& path)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    std::unordered_map<ProjectPath, Entry*>::iterator it = m_entryLookup.find(path);
    if (it != m_entryLookup.end())
    {
        (*it).second->m_removed = true;
        m_entryLookup.erase(it);
    }
}

void Project::IndexFile::unlockAndDiscard()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    m_entryLookup.clear();
    m_entryStore.clear();
    fclose(m_lockedFile);
    m_lockedFile = NULL;
}

void Project::IndexFile::unlockAndCommit()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    fseek(m_lockedFile, 0, SEEK_SET);
#if _WIN32
    SetEndOfFile((HANDLE)fileno(m_lockedFile));
#else
    ftruncate(fileno(m_lockedFile), 0);
#endif

    SIndexHeader header =
    {
        HECL::FourCC("HECL"),
        1, (uint32_t)m_entryStore.size(), (uint32_t)m_maxPathLen
    };
    header.swapWithNative();
    fwrite(&header, 1, sizeof(header), m_lockedFile);

    for (Project::IndexFile::Entry& ent : m_entryStore)
    {
        uint64_t mt = ToBig(ent.m_lastModtime.getTs());
        fwrite(&mt, 1, 8, m_lockedFile);
        size_t strLen = strlen(ent.m_path.getRelativePathUTF8());
        uint32_t strLenb = ToBig(strLen);
        fwrite(&strLenb, 1, 4, m_lockedFile);
        fwrite(ent.m_path.getRelativePathUTF8(), 1, strLen, m_lockedFile);
    }

    m_entryLookup.clear();
    m_entryStore.clear();
    fclose(m_lockedFile);
    m_lockedFile = NULL;
}

/**********************************************
 * Project
 **********************************************/

Project::Project(const ProjectRootPath& rootPath)
: m_rootPath(rootPath)
{
    /* Stat for existing project directory (must already exist) */
    struct stat myStat;
    if (HECL::Stat(m_rootPath.getAbsolutePath().c_str(), &myStat))
        throw std::error_code(errno, std::system_category());

    if (!S_ISDIR(myStat.st_mode))
        throw std::invalid_argument("provided path must be a directory; '" +
                                    m_rootPath.getAbsolutePathUTF8() + "' isn't");

    /* Create project directory structure */
    HECL::MakeDir(m_rootPath.getAbsolutePath() + _S("/.hecl"));
    HECL::MakeDir(m_rootPath.getAbsolutePath() + _S("/.hecl/cooked"));
    HECL::MakeDir(m_rootPath.getAbsolutePath() + _S("/.hecl/config"));

    /* Create or open databases */
}

void Project::registerLogger(FLogger logger)
{
}

bool Project::addPaths(const std::vector<ProjectPath>& paths)
{
}

bool Project::removePaths(const std::vector<ProjectPath>& paths, bool recursive)
{
}

bool Project::addGroup(const HECL::ProjectPath& path)
{
}

bool Project::removeGroup(const ProjectPath& path)
{
}

const std::map<const std::string, const bool>& Project::listDataSpecs()
{
}

bool Project::enableDataSpecs(const std::vector<std::string>& specs)
{
}

bool Project::disableDataSpecs(const std::vector<std::string>& specs)
{
}

bool Project::cookPath(const ProjectPath& path,
                       std::function<void(std::string&, Cost, unsigned)> feedbackCb,
                       bool recursive)
{
}

void Project::interruptCook()
{
}

bool Project::cleanPath(const ProjectPath& path, bool recursive)
{
}

Project::PackageDepsgraph Project::buildPackageDepsgraph(const ProjectPath& path)
{
}

}
}
