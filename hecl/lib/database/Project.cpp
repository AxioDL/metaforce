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

std::list<std::string>& Project::ConfigFile::lockAndRead()
{
    if (m_lockedFile)
        return m_lines;

    m_lockedFile = HECL::Fopen(m_filepath.c_str(), _S("a+"), LWRITE);

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

    for (auto it = m_lines.begin();
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

bool Project::ConfigFile::unlockAndCommit()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::ConfigFile::lockAndRead not yet called"));

    SystemString newPath = m_filepath + _S(".part");
    FILE* newFile = HECL::Fopen(newPath.c_str(), _S("w"), LWRITE);
    bool fail = false;
    for (const std::string& line : m_lines)
    {
        if (fwrite(line.c_str(), 1, line.size(), newFile) != line.size())
        {
            fail = true;
            break;
        }
        if (fwrite("\n", 1, 1, newFile) != 1)
        {
            fail = true;
            break;
        }
    }
    m_lines.clear();
    fclose(newFile);
    fclose(m_lockedFile);
    m_lockedFile = NULL;
    if (fail)
    {
        unlink(newPath.c_str());
        return false;
    }
    else
    {
        rename(newPath.c_str(), m_filepath.c_str());
        return true;
    }
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

    /* Open file and begin lock cycle */
    m_lockedFile = HECL::Fopen(m_filepath.c_str(), _S("a+b"), LWRITE);
    m_maxPathLen = 0;
    m_onlyUpdatedMaxPathLen = 0;

    /* Read index header */
    SIndexHeader header;
    if (fread(&header, 1, sizeof(header), m_lockedFile) != sizeof(header))
        return m_entryStore; /* Not yet written, this commit will take care of it */
    header.swapWithNative();
    if (header.magic != "HECL")
        throw HECL::Exception(_S("unrecognized HECL index"));
    if (header.version != 1)
        throw HECL::Exception(_S("unrecognized HECL version"));

    /* Iterate existing index entries */
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
            m_entryStore.push_back(Entry(path, mt));
            m_entryLookup[path] = &m_entryStore.back();
        }
    }
    delete[] pathBuf;
    return m_entryStore;
}

const std::vector<ProjectPath*> Project::IndexFile::getChangedPaths()
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    std::vector<ProjectPath*> retval;
    for (Entry& ent : m_entryStore)
        if (ent.m_lastModtime != ent.m_path.getModtime())
            retval.push_back(&ent.m_path);
    return retval;
}

void Project::IndexFile::addOrUpdatePath(const ProjectPath& path)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    size_t pathLen = path.getRelativePath().size();
    if (pathLen > m_onlyUpdatedMaxPathLen)
        m_onlyUpdatedMaxPathLen = pathLen;

    std::unordered_map<ProjectPath, Entry*>::iterator it = m_entryLookup.find(path);
    if (it == m_entryLookup.end())
    {
        m_entryStore.push_back(Entry(path, path.getModtime()));
        m_entryLookup[path] = &m_entryStore.back();
        m_entryStore.back().m_updated = true;
        return;
    }
    (*it).second->m_lastModtime = path.getModtime();
    (*it).second->m_updated = true;
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

bool Project::IndexFile::unlockAndCommit(bool onlyUpdated)
{
    if (!m_lockedFile)
        throw HECL::Exception(_S("Project::IndexFile::lockAndRead not yet called"));

    SystemString newPath = m_filepath + _S(".part");
    FILE* newFile = HECL::Fopen(newPath.c_str(), _S("wb"), LWRITE);
    SIndexHeader header =
    {
        HECL::FourCC("HECL"), 1,
        (uint32_t)(onlyUpdated ? m_updatedCount : m_entryStore.size()),
        (uint32_t)(onlyUpdated ? m_onlyUpdatedMaxPathLen : m_maxPathLen)
    };
    header.swapWithNative();
    bool fail = false;
    if (fwrite(&header, 1, sizeof(header), newFile) != sizeof(header))
        fail = true;

    if (!fail)
    {
        for (Entry& ent : m_entryStore)
        {
            if (!onlyUpdated || ent.m_updated)
            {
                uint64_t mt = ToBig(ent.m_lastModtime.getTs());
                if (fwrite(&mt, 1, 8, newFile) != 8)
                {
                    fail = true;
                    break;
                }
                size_t strLen = ent.m_path.getRelativePathUTF8().size();
                uint32_t strLenb = ToBig(strLen);
                if (fwrite(&strLenb, 1, 4, newFile) != 4)
                {
                    fail = true;
                    break;
                }
                if (fwrite(ent.m_path.getRelativePathUTF8().c_str(), 1, strLen, newFile) != strLen)
                {
                    fail = true;
                    break;
                }
            }
        }
    }

    m_entryLookup.clear();
    m_entryStore.clear();
    fclose(newFile);
    fclose(m_lockedFile);
    m_lockedFile = NULL;
    if (fail)
    {
        unlink(newPath.c_str());
        return false;
    }
    else
    {
        rename(newPath.c_str(), m_filepath.c_str());
        return true;
    }
}

/**********************************************
 * Project
 **********************************************/

Project::Project(const ProjectRootPath& rootPath)
: m_rootPath(rootPath),
  m_specs(*this, _S("specs")),
  m_paths(*this, _S("paths")),
  m_groups(*this, _S("groups")),
  m_index(*this)
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

    /* Ensure index is initialized */
    if (m_index.lockAndRead().empty())
        m_index.unlockAndCommit();
    else
        m_index.unlockAndDiscard();
}

void Project::registerLogger(FLogger logger)
{
    m_logger = logger;
}

bool Project::addPaths(const std::vector<ProjectPath>& paths)
{
    m_paths.lockAndRead();
    for (const ProjectPath& path : paths)
        m_paths.addLine(path.getRelativePathUTF8());
    return m_paths.unlockAndCommit();
}

bool Project::removePaths(const std::vector<ProjectPath>& paths, bool recursive)
{
    std::list<std::string>& existingPaths = m_paths.lockAndRead();
    if (recursive)
    {
        for (const ProjectPath& path : paths)
        {
            std::string recursiveBase = path.getRelativePathUTF8();
            for (auto it = existingPaths.begin();
                 it != existingPaths.end();
                 ++it)
            {
                if (!(*it).compare(0, recursiveBase.size(), recursiveBase))
                    it = existingPaths.erase(it);
            }
        }
    }
    else
        for (const ProjectPath& path : paths)
            m_paths.removeLine(path.getRelativePathUTF8());
    return m_paths.unlockAndCommit();
}

bool Project::addGroup(const HECL::ProjectPath& path)
{
    m_groups.lockAndRead();
    m_groups.addLine(path.getRelativePathUTF8());
    return m_groups.unlockAndCommit();
}

bool Project::removeGroup(const ProjectPath& path)
{
    m_groups.lockAndRead();
    m_groups.removeLine(path.getRelativePathUTF8());
    return m_groups.unlockAndCommit();
}

bool Project::enableDataSpecs(const std::vector<SystemString>& specs)
{
    m_specs.lockAndRead();
    for (const SystemString& spec : specs)
        m_specs.addLine(spec);
    return m_specs.unlockAndCommit();
}

bool Project::disableDataSpecs(const std::vector<SystemString>& specs)
{
    m_specs.lockAndRead();
    for (const SystemString& spec : specs)
        m_specs.removeLine(spec);
    return m_specs.unlockAndCommit();
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

PackageDepsgraph Project::buildPackageDepsgraph(const ProjectPath& path)
{
}

}
}
