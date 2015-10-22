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

std::vector<std::string>& Project::ConfigFile::lockAndRead()
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
        if (!line.compare(refLine))
            return true;
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
  m_workRoot(*this, _S("")),
  m_dotPath(m_workRoot, _S(".hecl")),
  m_cookedRoot(m_dotPath, _S("cooked")),
  m_specs(*this, _S("specs")),
  m_paths(*this, _S("paths")),
  m_groups(*this, _S("groups"))
{
    /* Stat for existing project directory (must already exist) */
    Sstat myStat;
    if (HECL::Stat(m_rootPath.getAbsolutePath().c_str(), &myStat))
    {
        LogModule.report(LogVisor::Error, _S("unable to stat %s"), m_rootPath.getAbsolutePath().c_str());
        return;
    }

    if (!S_ISDIR(myStat.st_mode))
    {
        LogModule.report(LogVisor::Error, _S("provided path must be a directory; '%s' isn't"), m_rootPath.getAbsolutePath().c_str());
        return;
    }

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
    std::vector<std::string>& existingPaths = m_paths.lockAndRead();
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
        HECL::SystemString specStr(spec->m_name);
        SystemUTF8View specUTF8(specStr);
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

class CookProgress
{
    FProgress& m_progFunc;
    const SystemChar* m_dir = nullptr;
    const SystemChar* m_file = nullptr;
    int lidx = 0;
    float m_prog = 0.0;
public:
    CookProgress(FProgress& progFunc) : m_progFunc(progFunc) {}
    void changeDir(const SystemChar* dir) {m_dir = dir; ++lidx;}
    void changeFile(const SystemChar* file, float prog) {m_file = file; m_prog = prog;}
    void reportFile(const DataSpecEntry* specEnt)
    {
        SystemString submsg(m_file);
        submsg += _S(" (");
        submsg += specEnt->m_name;
        submsg += _S(')');
        m_progFunc(m_dir, submsg.c_str(), lidx, m_prog);
    }
    void reportFile(const DataSpecEntry* specEnt, const SystemChar* extra)
    {
        SystemString submsg(m_file);
        submsg += _S(" (");
        submsg += specEnt->m_name;
        submsg += _S(", ");
        submsg += extra;
        submsg += _S(')');
        m_progFunc(m_dir, submsg.c_str(), lidx, m_prog);
    }
    void reportDirComplete() {m_progFunc(m_dir, nullptr, lidx, 1.0);}
};

using SpecInst = std::pair<const DataSpecEntry*, std::unique_ptr<IDataSpec>>;

static void VisitFile(const ProjectPath& path, bool force, bool fast,
                      std::vector<SpecInst>& specInsts,
                      CookProgress& progress)
{
    for (SpecInst& spec : specInsts)
    {
        if (spec.second->canCook(path))
        {
            ProjectPath cooked = path.getCookedPath(*spec.first);
            if (fast)
                cooked = cooked.getWithExtension(".fast");
            if (force || cooked.getPathType() == ProjectPath::PT_NONE ||
                path.getModtime() > cooked.getModtime())
            {
                progress.reportFile(spec.first);
                spec.second->doCook(path, cooked, fast,
                [&](const SystemChar* extra)
                {
                    progress.reportFile(spec.first, extra);
                });
            }
        }
    }
}

static void VisitDirectory(const ProjectPath& dir,
                           bool recursive, bool force, bool fast,
                           std::vector<SpecInst>& specInsts,
                           CookProgress& progress)
{
    std::map<SystemString, ProjectPath> children;
    dir.getDirChildren(children);

    /* Pass 1: child file count */
    int childFileCount = 0;
    for (auto& child : children)
    {
        switch (child.second.getPathType())
        {
        case ProjectPath::PT_FILE:
        {
            ++childFileCount;
            break;
        }
        case ProjectPath::PT_LINK:
        {
            ProjectPath target = child.second.resolveLink();
            if (target.getPathType() == ProjectPath::PT_FILE)
                ++childFileCount;
            break;
        }
        default: break;
        }
    }

    /* Pass 2: child files */
    int progNum = 0;
    float progDenom = childFileCount;
    progress.changeDir(dir.getLastComponent());
    for (auto& child : children)
    {
        switch (child.second.getPathType())
        {
        case ProjectPath::PT_FILE:
        {
            progress.changeFile(child.first.c_str(), progNum++/progDenom);
            VisitFile(child.second, force, fast, specInsts, progress);
            break;
        }
        case ProjectPath::PT_LINK:
        {
            ProjectPath target = child.second.resolveLink();
            if (target.getPathType() == ProjectPath::PT_FILE)
            {
                progress.changeFile(target.getLastComponent(), progNum++/progDenom);
                VisitFile(target, force, fast, specInsts, progress);
            }
            break;
        }
        default: break;
        }
    }
    progress.reportDirComplete();

    /* Pass 3: child directories */
    if (recursive)
    {
        for (auto& child : children)
        {
            switch (child.second.getPathType())
            {
            case ProjectPath::PT_DIRECTORY:
            {
                VisitDirectory(child.second, recursive, force, fast, specInsts, progress);
                break;
            }
            default: break;
            }
        }
    }
}

static void VisitGlob(const ProjectPath& path,
                      bool recursive, bool force, bool fast,
                      std::vector<SpecInst>& specInsts,
                      CookProgress& progress)
{
    std::vector<ProjectPath> children;
    path.getGlobResults(children);

    /* Pass 1: child file count */
    int childFileCount = 0;
    for (ProjectPath& child : children)
    {
        switch (child.getPathType())
        {
        case ProjectPath::PT_FILE:
        {
            ++childFileCount;
            break;
        }
        case ProjectPath::PT_LINK:
        {
            ProjectPath target = path.resolveLink();
            if (target.getPathType() == ProjectPath::PT_FILE)
                ++childFileCount;
            break;
        }
        default: break;
        }
    }

    /* Pass 2: child files */
    int progNum = 0;
    float progDenom = childFileCount;
    progress.changeDir(path.getLastComponent());
    for (ProjectPath& child : children)
    {
        switch (child.getPathType())
        {
        case ProjectPath::PT_FILE:
        {
            progress.changeFile(child.getLastComponent(), progNum++/progDenom);
            VisitFile(child, force, fast, specInsts, progress);
            break;
        }
        case ProjectPath::PT_LINK:
        {
            ProjectPath target = path.resolveLink();
            if (target.getPathType() == ProjectPath::PT_FILE)
            {
                progress.changeFile(target.getLastComponent(), progNum++/progDenom);
                VisitFile(target, force, fast, specInsts, progress);
            }
            break;
        }
        default: break;
        }
    }
    progress.reportDirComplete();

    /* Pass 3: child directories */
    if (recursive)
    {
        for (ProjectPath& child : children)
        {
            switch (child.getPathType())
            {
            case ProjectPath::PT_DIRECTORY:
            {
                VisitDirectory(child, recursive, force, fast, specInsts, progress);
                break;
            }
            default: break;
            }
        }
    }
}

bool Project::cookPath(const ProjectPath& path, FProgress progress,
                       bool recursive, bool force, bool fast)
{
    /* Construct DataSpec instances for cooking */
    std::vector<SpecInst> specInsts;
    specInsts.reserve(m_compiledSpecs.size());
    for (const ProjectDataSpec& spec : m_compiledSpecs)
        if (spec.active)
            specInsts.emplace_back(&spec.spec,
            std::unique_ptr<IDataSpec>(spec.spec.m_factory(*this, TOOL_COOK)));

    /* Iterate complete directory/file/glob list */
    CookProgress cookProg(progress);
    switch (path.getPathType())
    {
    case ProjectPath::PT_FILE:
    {
        cookProg.changeFile(path.getLastComponent(), 0.0);
        VisitFile(path, force, fast, specInsts, cookProg);
        break;
    }
    case ProjectPath::PT_LINK:
    {
        ProjectPath target = path.resolveLink();
        if (target.getPathType() == ProjectPath::PT_FILE)
        {
            cookProg.changeFile(target.getLastComponent(), 0.0);
            VisitFile(target, force, fast, specInsts, cookProg);
        }
        break;
    }
    case ProjectPath::PT_DIRECTORY:
    {
        VisitDirectory(path, recursive, force, fast, specInsts, cookProg);
        break;
    }
    case ProjectPath::PT_GLOB:
    {
        VisitGlob(path, recursive, force, fast, specInsts, cookProg);
        break;
    }
    default: break;
    }

    return true;
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
