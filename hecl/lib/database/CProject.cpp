#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <system_error>

#include "HECLDatabase.hpp"

namespace HECLDatabase
{

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

class CProject : public IProject
{
    HECL::SystemString m_rootPath;

    class ConfigFile
    {
        const CProject& m_project;
        const HECL::SystemString& m_name;
        HECL::SystemString m_filepath;
    public:
        ConfigFile(const CProject& project, const HECL::SystemString& name)
        : m_project(project), m_name(name)
        {
            m_filepath = project.m_rootPath + _S("/.hecl/config/") + name;
        }

        std::vector<std::string> readLines()
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
                if (CheckNewLineAdvance(end))
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

        void addLine(const std::string& line)
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

        void removeLine(const std::string& refLine)
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

        bool checkForLine(const std::string& refLine)
        {
            std::vector<std::string> curLines = readLines();
            for (std::string& line : curLines)
            {
                if (!line.compare(refLine))
                    return true;
            }
            return false;
        }

    };

public:
    CProject(const std::string& rootPath)
    : m_rootPath(rootPath)
    {
        /* Stat for existing project directory (must already exist) */
        struct stat myStat;
        if (stat(m_rootPath.c_str(), &myStat))
            throw std::error_code(errno, std::system_category());

        if (!S_ISDIR(myStat.st_mode))
            throw std::invalid_argument("provided path must be a directory; '" + m_rootPath + "' isn't");

        /* Create project directory structure */
        HECL::MakeDir(m_rootPath + "/.hecl");
        HECL::MakeDir(m_rootPath + "/.hecl/cooked");
        HECL::MakeDir(m_rootPath + "/.hecl/config");

        /* Create or open databases */
    }

    ~CProject()
    {
    }

    void registerLogger(HECL::TLogger logger)
    {
    }

    const HECL::ProjectRootPath& getProjectRootPath(bool absolute) const
    {
    }

    bool addPaths(const std::vector<HECL::ProjectPath>& paths)
    {
    }

    bool removePaths(const std::vector<HECL::ProjectPath>& paths, bool recursive)
    {
    }

    bool addGroup(const std::string& path)
    {
    }

    bool removeGroup(const std::string& path)
    {
    }

    const std::map<const std::string, const bool>& listPlatforms()
    {
    }

    bool enablePlatforms(const std::vector<std::string>& platforms)
    {
    }

    bool disablePlatforms(const std::vector<std::string>& platforms)
    {
    }

    bool cookPath(const std::string& path,
                  std::function<void(std::string&, Cost, unsigned)> feedbackCb,
                  bool recursive)
    {
    }

    void interruptCook()
    {
    }

    bool cleanPath(const std::string& path, bool recursive)
    {
    }

    bool packagePath(const std::string& path, bool recursive)
    {
    }

};

IProject* OpenProject(const std::string& rootPath)
{
    return new CProject(rootPath);
}

}
