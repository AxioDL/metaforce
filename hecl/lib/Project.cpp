#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <system_error>

#if _WIN32
#else
#include <unistd.h>
#endif

#include "hecl/Database.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/ClientProcess.hpp"

namespace hecl::Database
{

logvisor::Module LogModule("hecl::Database");
static const hecl::FourCC HECLfcc("HECL");

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

Project::ConfigFile::ConfigFile(const Project& project, SystemStringView name,
                                SystemStringView subdir)
{
    m_filepath = SystemString(project.m_rootPath.getAbsolutePath()) + subdir.data() + name.data();
}

std::vector<std::string>& Project::ConfigFile::lockAndRead()
{
    if (m_lockedFile)
        return m_lines;

    m_lockedFile = hecl::Fopen(m_filepath.c_str(), _S("a+"), FileLockType::Write);
    hecl::FSeek(m_lockedFile, 0, SEEK_SET);

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

void Project::ConfigFile::addLine(std::string_view line)
{
    if (!checkForLine(line))
        m_lines.emplace_back(line);
}

void Project::ConfigFile::removeLine(std::string_view refLine)
{
    if (!m_lockedFile)
    {
        LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__,
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

bool Project::ConfigFile::checkForLine(std::string_view refLine)
{
    if (!m_lockedFile)
    {
        LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__,
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
        LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__,
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
        LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__,
                               "Project::ConfigFile::lockAndRead not yet called");
        return false;
    }

    SystemString newPath = m_filepath + _S(".part");
    FILE* newFile = hecl::Fopen(newPath.c_str(), _S("w"), FileLockType::Write);
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
        //_wrename(newPath.c_str(), m_filepath.c_str());
        MoveFileExW(newPath.c_str(), m_filepath.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
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
    if (hecl::Stat(m_rootPath.getAbsolutePath().data(), &myStat))
    {
        LogModule.report(logvisor::Error, _S("unable to stat %s"), m_rootPath.getAbsolutePath().data());
        return;
    }

    if (!S_ISDIR(myStat.st_mode))
    {
        LogModule.report(logvisor::Error, _S("provided path must be a directory; '%s' isn't"),
                         m_rootPath.getAbsolutePath().data());
        return;
    }

    /* Create project directory structure */
    m_dotPath.makeDir();
    m_cookedRoot.makeDir();

    /* Ensure beacon is valid or created */
    ProjectPath beaconPath(m_dotPath, _S("beacon"));
    FILE* bf = hecl::Fopen(beaconPath.getAbsolutePath().data(), _S("a+b"));
    struct BeaconStruct
    {
        hecl::FourCC magic;
        uint32_t version;
    } beacon;
#define DATA_VERSION 1
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
        LogModule.report(logvisor::Fatal, "incompatible project version");
        return;
    }

    /* Compile current dataspec */
    rescanDataSpecs();
    m_valid = true;
}

const ProjectPath& Project::getProjectCookedPath(const DataSpecEntry& spec) const
{
    for (const ProjectDataSpec& sp : m_compiledSpecs)
        if (&sp.spec == &spec)
            return sp.cookedPath;
    LogModule.report(logvisor::Fatal, "Unable to find spec '%s'", spec.m_name.data());
    return m_cookedRoot;
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
            auto recursiveBase = path.getRelativePathUTF8();
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

bool Project::addGroup(const hecl::ProjectPath& path)
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
        hecl::SystemString specStr(spec->m_name);
        SystemUTF8Conv specUTF8(specStr);
        m_compiledSpecs.push_back({*spec, ProjectPath(m_cookedRoot, hecl::SystemString(spec->m_name) + _S(".spec")),
                                   m_specs.checkForLine(specUTF8.str())});
    }
    m_specs.unlockAndDiscard();
}

bool Project::enableDataSpecs(const std::vector<SystemString>& specs)
{
    m_specs.lockAndRead();
    for (const SystemString& spec : specs)
    {
        SystemUTF8Conv specView(spec);
        m_specs.addLine(specView.str());
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
        SystemUTF8Conv specView(spec);
        m_specs.removeLine(specView.str());
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
        submsg += specEnt->m_name.data();
        submsg += _S(')');
        if (m_progFunc)
            m_progFunc(m_dir, submsg.c_str(), lidx, m_prog);
    }
    void reportFile(const DataSpecEntry* specEnt, const SystemChar* extra)
    {
        SystemString submsg(m_file);
        submsg += _S(" (");
        submsg += specEnt->m_name.data();
        submsg += _S(", ");
        submsg += extra;
        submsg += _S(')');
        if (m_progFunc)
            m_progFunc(m_dir, submsg.c_str(), lidx, m_prog);
    }
    void reportDirComplete()
    {
        if (m_progFunc)
            m_progFunc(m_dir, nullptr, lidx, 1.0);
    }
};

static void VisitFile(const ProjectPath& path, bool force, bool fast,
                      std::vector<std::unique_ptr<IDataSpec>>& specInsts,
                      CookProgress& progress, ClientProcess* cp)
{
    for (auto& spec : specInsts)
    {
        if (spec->canCook(path, hecl::blender::SharedBlenderToken))
        {
            if (cp)
            {
                cp->addCookTransaction(path, spec.get());
            }
            else
            {
                const DataSpecEntry* override = spec->overrideDataSpec(path, spec->getDataSpecEntry(),
                                                                       hecl::blender::SharedBlenderToken);
                if (!override)
                    continue;
                ProjectPath cooked = path.getCookedPath(*override);
                if (fast)
                    cooked = cooked.getWithExtension(_S(".fast"));
                if (force || cooked.getPathType() == ProjectPath::Type::None ||
                    path.getModtime() > cooked.getModtime())
                {
                    progress.reportFile(override);
                    spec->doCook(path, cooked, fast, hecl::blender::SharedBlenderToken,
                                 [&](const SystemChar* extra)
                                 {
                                     progress.reportFile(override, extra);
                                 });
                }
            }
        }
    }
}

static void VisitDirectory(const ProjectPath& dir,
                           bool recursive, bool force, bool fast,
                           std::vector<std::unique_ptr<IDataSpec>>& specInsts,
                           CookProgress& progress, ClientProcess* cp)
{
    if (dir.getLastComponent().size() > 1 && dir.getLastComponent()[0] == _S('.'))
        return;

    std::map<SystemString, ProjectPath> children;
    dir.getDirChildren(children);

    /* Pass 1: child file count */
    int childFileCount = 0;
    for (auto& child : children)
        if (child.second.getPathType() == ProjectPath::Type::File)
            ++childFileCount;

    /* Pass 2: child files */
    int progNum = 0;
    float progDenom = childFileCount;
    progress.changeDir(dir.getLastComponent().data());
    for (auto& child : children)
    {
        if (child.second.getPathType() == ProjectPath::Type::File)
        {
            progress.changeFile(child.first.c_str(), progNum++/progDenom);
            VisitFile(child.second, force, fast, specInsts, progress, cp);
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
            case ProjectPath::Type::Directory:
            {
                VisitDirectory(child.second, recursive, force, fast, specInsts, progress, cp);
                break;
            }
            default: break;
            }
        }
    }
}

bool Project::cookPath(const ProjectPath& path, FProgress progress,
                       bool recursive, bool force, bool fast, ClientProcess* cp)
{
    /* Construct DataSpec instances for cooking */
    if (m_cookSpecs.empty())
    {
        m_cookSpecs.reserve(m_compiledSpecs.size());
        for (const ProjectDataSpec& spec : m_compiledSpecs)
            if (spec.active && spec.spec.m_factory)
                m_cookSpecs.push_back(std::unique_ptr<IDataSpec>(spec.spec.m_factory(*this, DataSpecTool::Cook)));
    }

    /* Iterate complete directory/file/glob list */
    CookProgress cookProg(progress);
    switch (path.getPathType())
    {
    case ProjectPath::Type::File:
    case ProjectPath::Type::Glob:
    {
        cookProg.changeFile(path.getLastComponent().data(), 0.0);
        VisitFile(path, force, fast, m_cookSpecs, cookProg, cp);
        break;
    }
    case ProjectPath::Type::Directory:
    {
        VisitDirectory(path, recursive, force, fast, m_cookSpecs, cookProg, cp);
        break;
    }
    default: break;
    }

    return true;
}

bool Project::packagePath(const ProjectPath& path, FProgress progress, bool fast, ClientProcess* cp)
{
    /* Construct DataSpec instance for packaging */
    const DataSpecEntry* specEntry = nullptr;
    bool foundPC = false;
    for (const ProjectDataSpec& spec : m_compiledSpecs)
    {
        if (spec.active && spec.spec.m_factory)
        {
            if (hecl::StringUtils::EndsWith(spec.spec.m_name, _S("-PC")))
            {
                foundPC = true;
                specEntry = &spec.spec;
            }
            else if (!foundPC)
            {
                specEntry = &spec.spec;
            }
        }
    }

    if (!specEntry)
        LogModule.report(logvisor::Fatal, "No matching DataSpec");

    if (!m_lastPackageSpec || m_lastPackageSpec->getDataSpecEntry() != specEntry)
        m_lastPackageSpec = std::unique_ptr<IDataSpec>(specEntry->m_factory(*this, DataSpecTool::Package));

    if (m_lastPackageSpec->canPackage(path))
    {
        m_lastPackageSpec->doPackage(path, specEntry, fast, hecl::blender::SharedBlenderToken, progress, cp);
        return true;
    }

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

void Project::addBridgePathToCache(uint64_t id, const ProjectPath& path)
{
    m_bridgePathCache[id] = path;
}

void Project::clearBridgePathCache()
{
    m_bridgePathCache.clear();
}

const ProjectPath* Project::lookupBridgePath(uint64_t id) const
{
    auto search = m_bridgePathCache.find(id);
    if (search == m_bridgePathCache.cend())
        return nullptr;
    return &search->second;
}

}
