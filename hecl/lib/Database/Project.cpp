#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <system_error>

#if _WIN32
#else
#include <unistd.h>
#endif

#include "HECL/Database.hpp"

namespace HECL
{
namespace Database
{

LogVisor::LogModule LogModule("HECLDatabase");

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

Project::ConfigFile::ConfigFile(const Project& project, const SystemString& name,
                                const SystemString& subdir)
{
    m_filepath = project.m_rootPath.getAbsolutePath() + subdir + name;
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
    {
        LogModule.reportSource(LogVisor::FatalError, __FILE__, __LINE__,
                               "Project::ConfigFile::lockAndRead not yet called");
        return;
    }

    for (auto it = m_lines.begin();
         it != m_lines.end();)
    {
        if (!(*it).compare(refLine))
        {
            it = m_lines.erase(it);
            continue;
        }
        ++it;
    }
}

bool Project::ConfigFile::checkForLine(const std::string& refLine)
{
    if (!m_lockedFile)
    {
        LogModule.reportSource(LogVisor::FatalError, __FILE__, __LINE__,
                               "Project::ConfigFile::lockAndRead not yet called");
        return false;
    }

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
    {
        LogModule.reportSource(LogVisor::FatalError, __FILE__, __LINE__,
                               "Project::ConfigFile::lockAndRead not yet called");
        return;
    }

    m_lines.clear();
    fclose(m_lockedFile);
    m_lockedFile = NULL;
}

bool Project::ConfigFile::unlockAndCommit()
{
    if (!m_lockedFile)
    {
        LogModule.reportSource(LogVisor::FatalError, __FILE__, __LINE__,
                               "Project::ConfigFile::lockAndRead not yet called");
        return false;
    }

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
#if HECL_UCS2
        _wunlink(newPath.c_str());
#else
        unlink(newPath.c_str());
#endif
        return false;
    }
    else
    {
#if HECL_UCS2
        _wrename(newPath.c_str(), m_filepath.c_str());
#else
        rename(newPath.c_str(), m_filepath.c_str());
#endif
        return true;
    }
}

/**********************************************
 * Project
 **********************************************/

Project::Project(const ProjectRootPath& rootPath)
: m_rootPath(rootPath),
  m_dotPath(m_rootPath, _S(".hecl")),
  m_cookedRoot(m_dotPath, _S("cooked")),
  m_specs(*this, _S("specs")),
  m_paths(*this, _S("paths")),
  m_groups(*this, _S("groups"))
{
    /* Stat for existing project directory (must already exist) */
    Sstat myStat;
    if (HECL::Stat(m_rootPath.getAbsolutePath().c_str(), &myStat))
        throw std::error_code(errno, std::system_category());

    if (!S_ISDIR(myStat.st_mode))
        throw std::invalid_argument("provided path must be a directory; '" +
                                    m_rootPath.getAbsolutePathUTF8() + "' isn't");

    /* Create project directory structure */
    m_dotPath.makeDir();
    m_cookedRoot.makeDir();

    /* Ensure beacon is valid or created */
    ProjectPath beaconPath(m_dotPath, _S("beacon"));
    FILE* bf = HECL::Fopen(beaconPath.getAbsolutePath().c_str(), _S("a+b"));
    struct BeaconStruct
    {
        HECL::FourCC magic;
        uint32_t version;
    } beacon;
#define DATA_VERSION 1
    static const HECL::FourCC HECLfcc("HECL");
    if (fread(&beacon, 1, sizeof(beacon), bf) != sizeof(beacon))
    {
        fseek(bf, 0, SEEK_SET);
        beacon.magic = HECLfcc;
        beacon.version = SBig(DATA_VERSION);
        fwrite(&beacon, 1, sizeof(beacon), bf);
    }
    fclose(bf);
    if (beacon.magic != HECLfcc ||
        SBig(beacon.version) != DATA_VERSION)
    {
        LogModule.report(LogVisor::FatalError, "incompatible project version");
        return;
    }

    /* Compile current dataspec */
    rescanDataSpecs();
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
                 it != existingPaths.end();)
            {
                if (!(*it).compare(0, recursiveBase.size(), recursiveBase))
                {
                    it = existingPaths.erase(it);
                    continue;
                }
                ++it;
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

void Project::rescanDataSpecs()
{
    m_compiledSpecs.clear();
    m_specs.lockAndRead();
    for (const DataSpecEntry* spec : DATA_SPEC_REGISTRY)
    {
        SystemUTF8View specUTF8(spec->m_name);
        m_compiledSpecs.push_back({*spec, ProjectPath(m_cookedRoot, HECL::SystemString(spec->m_name) + _S(".spec")),
                                   m_specs.checkForLine(specUTF8) ? true : false});
    }
    m_specs.unlockAndDiscard();
}

bool Project::enableDataSpecs(const std::vector<SystemString>& specs)
{
    m_specs.lockAndRead();
    for (const SystemString& spec : specs)
    {
        SystemUTF8View specView(spec);
        m_specs.addLine(specView);
    }
    bool result = m_specs.unlockAndCommit();
    rescanDataSpecs();
    return result;
}

bool Project::disableDataSpecs(const std::vector<SystemString>& specs)
{
    m_specs.lockAndRead();
    for (const SystemString& spec : specs)
    {
        SystemUTF8View specView(spec);
        m_specs.removeLine(specView);
    }
    bool result = m_specs.unlockAndCommit();
    rescanDataSpecs();
    return result;
}

bool Project::cookPath(const ProjectPath& path,
                       std::function<void(SystemString&, Cost, unsigned)> feedbackCb,
                       bool recursive)
{
    return false;
}

void Project::interruptCook()
{
}

bool Project::cleanPath(const ProjectPath& path, bool recursive)
{
    return false;
}

PackageDepsgraph Project::buildPackageDepsgraph(const ProjectPath& path)
{
    return PackageDepsgraph();
}

}
}
