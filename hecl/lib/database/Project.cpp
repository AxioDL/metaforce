#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <system_error>

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
    if (*it == '\n' || *it == '\0')
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
    }
    return false;
}

Project::ConfigFile::ConfigFile(const Project& project, const SystemString& name)
: m_project(project), m_name(name)
{
    m_filepath = project.m_rootPath.getAbsolutePath() + _S("/.hecl/config/") + name;
}

std::vector<std::string> Project::ConfigFile::readLines()
{
    FILE* fp = HECL::Fopen(m_filepath.c_str(), _S("r"));

    std::string mainString;
    char readBuf[1024];
    size_t readSz;
    while ((readSz = fread(readBuf, 1, 1024, fp)))
        mainString += std::string(readBuf, readSz);
    fclose(fp);

    std::string::const_iterator begin = mainString.begin();
    std::string::const_iterator end = mainString.begin();

    std::vector<std::string> retval;
    while (end != mainString.end())
    {
        std::string::const_iterator origEnd = end;
        if (*end == '\0')
            break;
        else if (CheckNewLineAdvance(end))
        {
            if (begin != origEnd)
                retval.push_back(std::string(begin, origEnd));
            begin = end;
            continue;
        }
        ++end;
    }
    if (begin != end)
        retval.push_back(std::string(begin, end));

    return retval;
}

void Project::ConfigFile::addLine(const std::string& line)
{
    std::vector<std::string> curLines = readLines();

    FILE* fp = HECL::Fopen(m_filepath.c_str(), _S("w"));
    for (std::string& line : curLines)
    {
        fwrite(line.data(), 1, line.length(), fp);
        fwrite("\n", 1, 1, fp);
    }
    fwrite(line.data(), 1, line.length(), fp);
    fwrite("\n", 1, 1, fp);
    fclose(fp);
}

void Project::ConfigFile::removeLine(const std::string& refLine)
{
    std::vector<std::string> curLines = readLines();

    FILE* fp = HECL::Fopen(m_filepath.c_str(), _S("w"));
    for (std::string& line : curLines)
    {
        if (line.compare(refLine))
        {
            fwrite(line.data(), 1, line.length(), fp);
            fwrite("\n", 1, 1, fp);
        }
    }
    fclose(fp);
}

bool Project::ConfigFile::checkForLine(const std::string& refLine)
{
    std::vector<std::string> curLines = readLines();
    for (std::string& line : curLines)
    {
        if (!line.compare(refLine))
            return true;
    }
    return false;
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

const ProjectRootPath& Project::getProjectRootPath(bool absolute) const
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
