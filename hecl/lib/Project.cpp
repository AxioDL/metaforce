#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>

#if _WIN32
#else
#include <unistd.h>
#endif

#include "hecl/ClientProcess.hpp"
#include "hecl/Database.hpp"
#include "hecl/Blender/Connection.hpp"
#include "hecl/MultiProgressPrinter.hpp"

#include <logvisor/logvisor.hpp>

namespace hecl::Database {

logvisor::Module LogModule("hecl::Database");
constexpr hecl::FourCC HECLfcc("HECL");

/**********************************************
 * Project::ConfigFile
 **********************************************/

static bool CheckNewLineAdvance(std::string::const_iterator& it) {
  if (*it == '\n') {
    it += 1;
    return true;
  } else if (*it == '\r') {
    if (*(it + 1) == '\n') {
      it += 2;
      return true;
    }
    it += 1;
    return true;
  }
  return false;
}

Project::ConfigFile::ConfigFile(const Project& project, SystemStringView name, SystemStringView subdir) {
  m_filepath = SystemString(project.m_rootPath.getAbsolutePath()) + subdir.data() + name.data();
}

std::vector<std::string>& Project::ConfigFile::lockAndRead() {
  if (m_lockedFile != nullptr) {
    return m_lines;
  }

  m_lockedFile = hecl::FopenUnique(m_filepath.c_str(), _SYS_STR("a+"), FileLockType::Write);
  hecl::FSeek(m_lockedFile.get(), 0, SEEK_SET);

  std::string mainString;
  char readBuf[1024];
  size_t readSz;
  while ((readSz = std::fread(readBuf, 1, sizeof(readBuf), m_lockedFile.get()))) {
    mainString += std::string(readBuf, readSz);
  }

  auto begin = mainString.cbegin();
  auto end = mainString.cbegin();

  m_lines.clear();
  while (end != mainString.end()) {
    auto origEnd = end;
    if (*end == '\0') {
      break;
    }
    if (CheckNewLineAdvance(end)) {
      if (begin != origEnd) {
        m_lines.emplace_back(begin, origEnd);
      }
      begin = end;
      continue;
    }
    ++end;
  }
  if (begin != end) {
    m_lines.emplace_back(begin, end);
  }

  return m_lines;
}

void Project::ConfigFile::addLine(std::string_view line) {
  if (!checkForLine(line))
    m_lines.emplace_back(line);
}

void Project::ConfigFile::removeLine(std::string_view refLine) {
  if (!m_lockedFile) {
    LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__, fmt("Project::ConfigFile::lockAndRead not yet called"));
    return;
  }

  for (auto it = m_lines.begin(); it != m_lines.end();) {
    if (*it == refLine) {
      it = m_lines.erase(it);
      continue;
    }
    ++it;
  }
}

bool Project::ConfigFile::checkForLine(std::string_view refLine) {
  if (!m_lockedFile) {
    LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__, fmt("Project::ConfigFile::lockAndRead not yet called"));
    return false;
  }

  for (const std::string& line : m_lines)
    if (line == refLine)
      return true;
  return false;
}

void Project::ConfigFile::unlockAndDiscard() {
  if (m_lockedFile == nullptr) {
    LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__, fmt("Project::ConfigFile::lockAndRead not yet called"));
    return;
  }

  m_lines.clear();
  m_lockedFile.reset();
}

bool Project::ConfigFile::unlockAndCommit() {
  if (!m_lockedFile) {
    LogModule.reportSource(logvisor::Fatal, __FILE__, __LINE__, fmt("Project::ConfigFile::lockAndRead not yet called"));
    return false;
  }

  const SystemString newPath = m_filepath + _SYS_STR(".part");
  auto newFile = hecl::FopenUnique(newPath.c_str(), _SYS_STR("w"), FileLockType::Write);
  bool fail = false;
  for (const std::string& line : m_lines) {
    if (std::fwrite(line.c_str(), 1, line.size(), newFile.get()) != line.size()) {
      fail = true;
      break;
    }
    if (std::fputc('\n', newFile.get()) == EOF) {
      fail = true;
      break;
    }
  }
  m_lines.clear();
  newFile.reset();
  m_lockedFile.reset();
  if (fail) {
#if HECL_UCS2
    _wunlink(newPath.c_str());
#else
    unlink(newPath.c_str());
#endif
    return false;
  } else {
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
: m_rootPath(rootPath)
, m_workRoot(*this, _SYS_STR(""))
, m_dotPath(m_workRoot, _SYS_STR(".hecl"))
, m_cookedRoot(m_dotPath, _SYS_STR("cooked"))
, m_specs(*this, _SYS_STR("specs"))
, m_paths(*this, _SYS_STR("paths"))
, m_groups(*this, _SYS_STR("groups")) {
  /* Stat for existing project directory (must already exist) */
  Sstat myStat;
  if (hecl::Stat(m_rootPath.getAbsolutePath().data(), &myStat)) {
    LogModule.report(logvisor::Error, fmt(_SYS_STR("unable to stat {}")), m_rootPath.getAbsolutePath());
    return;
  }

  if (!S_ISDIR(myStat.st_mode)) {
    LogModule.report(logvisor::Error, fmt(_SYS_STR("provided path must be a directory; '{}' isn't")),
                     m_rootPath.getAbsolutePath());
    return;
  }

  /* Create project directory structure */
  m_dotPath.makeDir();
  m_cookedRoot.makeDir();

  /* Ensure beacon is valid or created */
  const ProjectPath beaconPath(m_dotPath, _SYS_STR("beacon"));
  auto bf = hecl::FopenUnique(beaconPath.getAbsolutePath().data(), _SYS_STR("a+b"));
  struct BeaconStruct {
    hecl::FourCC magic;
    uint32_t version;
  } beacon;
  constexpr uint32_t DATA_VERSION = 1;
  if (std::fread(&beacon, 1, sizeof(beacon), bf.get()) != sizeof(beacon)) {
    std::fseek(bf.get(), 0, SEEK_SET);
    beacon.magic = HECLfcc;
    beacon.version = SBig(DATA_VERSION);
    std::fwrite(&beacon, 1, sizeof(beacon), bf.get());
  }
  bf.reset();

  if (beacon.magic != HECLfcc || SBig(beacon.version) != DATA_VERSION) {
    LogModule.report(logvisor::Fatal, fmt("incompatible project version"));
    return;
  }

  /* Compile current dataspec */
  rescanDataSpecs();
  m_valid = true;
}

const ProjectPath& Project::getProjectCookedPath(const DataSpecEntry& spec) const {
  for (const ProjectDataSpec& sp : m_compiledSpecs)
    if (&sp.spec == &spec)
      return sp.cookedPath;
  LogModule.report(logvisor::Fatal, fmt(_SYS_STR("Unable to find spec '{}'")), spec.m_name);
  return m_cookedRoot;
}

bool Project::addPaths(const std::vector<ProjectPath>& paths) {
  m_paths.lockAndRead();
  for (const ProjectPath& path : paths)
    m_paths.addLine(path.getRelativePathUTF8());
  return m_paths.unlockAndCommit();
}

bool Project::removePaths(const std::vector<ProjectPath>& paths, bool recursive) {
  std::vector<std::string>& existingPaths = m_paths.lockAndRead();
  if (recursive) {
    for (const ProjectPath& path : paths) {
      auto recursiveBase = path.getRelativePathUTF8();
      for (auto it = existingPaths.begin(); it != existingPaths.end();) {
        if (!(*it).compare(0, recursiveBase.size(), recursiveBase)) {
          it = existingPaths.erase(it);
          continue;
        }
        ++it;
      }
    }
  } else
    for (const ProjectPath& path : paths)
      m_paths.removeLine(path.getRelativePathUTF8());
  return m_paths.unlockAndCommit();
}

bool Project::addGroup(const hecl::ProjectPath& path) {
  m_groups.lockAndRead();
  m_groups.addLine(path.getRelativePathUTF8());
  return m_groups.unlockAndCommit();
}

bool Project::removeGroup(const ProjectPath& path) {
  m_groups.lockAndRead();
  m_groups.removeLine(path.getRelativePathUTF8());
  return m_groups.unlockAndCommit();
}

void Project::rescanDataSpecs() {
  m_compiledSpecs.clear();
  m_specs.lockAndRead();
  for (const DataSpecEntry* spec : DATA_SPEC_REGISTRY) {
    hecl::SystemString specStr(spec->m_name);
    SystemUTF8Conv specUTF8(specStr);
    m_compiledSpecs.push_back({*spec, ProjectPath(m_cookedRoot, hecl::SystemString(spec->m_name) + _SYS_STR(".spec")),
                               m_specs.checkForLine(specUTF8.str())});
  }
  m_specs.unlockAndDiscard();
}

bool Project::enableDataSpecs(const std::vector<SystemString>& specs) {
  m_specs.lockAndRead();
  for (const SystemString& spec : specs) {
    SystemUTF8Conv specView(spec);
    m_specs.addLine(specView.str());
  }
  bool result = m_specs.unlockAndCommit();
  rescanDataSpecs();
  return result;
}

bool Project::disableDataSpecs(const std::vector<SystemString>& specs) {
  m_specs.lockAndRead();
  for (const SystemString& spec : specs) {
    SystemUTF8Conv specView(spec);
    m_specs.removeLine(specView.str());
  }
  bool result = m_specs.unlockAndCommit();
  rescanDataSpecs();
  return result;
}

class CookProgress {
  const hecl::MultiProgressPrinter& m_progPrinter;
  const SystemChar* m_dir = nullptr;
  const SystemChar* m_file = nullptr;
  float m_prog = 0.f;

public:
  CookProgress(const hecl::MultiProgressPrinter& progPrinter) : m_progPrinter(progPrinter) {}
  void changeDir(const SystemChar* dir) {
    m_dir = dir;
    m_progPrinter.startNewLine();
  }
  void changeFile(const SystemChar* file, float prog) {
    m_file = file;
    m_prog = prog;
  }
  void reportFile(const DataSpecEntry* specEnt) {
    SystemString submsg(m_file);
    submsg += _SYS_STR(" (");
    submsg += specEnt->m_name.data();
    submsg += _SYS_STR(')');
    m_progPrinter.print(m_dir, submsg.c_str(), m_prog);
  }
  void reportFile(const DataSpecEntry* specEnt, const SystemChar* extra) {
    SystemString submsg(m_file);
    submsg += _SYS_STR(" (");
    submsg += specEnt->m_name.data();
    submsg += _SYS_STR(", ");
    submsg += extra;
    submsg += _SYS_STR(')');
    m_progPrinter.print(m_dir, submsg.c_str(), m_prog);
  }
  void reportDirComplete() { m_progPrinter.print(m_dir, nullptr, 1.f); }
};

static void VisitFile(const ProjectPath& path, bool force, bool fast,
                      std::vector<std::unique_ptr<IDataSpec>>& specInsts, CookProgress& progress, ClientProcess* cp) {
  for (auto& spec : specInsts) {
    if (spec->canCook(path, hecl::blender::SharedBlenderToken)) {
      if (cp) {
        cp->addCookTransaction(path, force, fast, spec.get());
      } else {
        const DataSpecEntry* override = spec->overrideDataSpec(path, spec->getDataSpecEntry());
        if (!override)
          continue;
        ProjectPath cooked = path.getCookedPath(*override);
        if (fast)
          cooked = cooked.getWithExtension(_SYS_STR(".fast"));
        if (force || cooked.getPathType() == ProjectPath::Type::None || path.getModtime() > cooked.getModtime()) {
          progress.reportFile(override);
          spec->doCook(path, cooked, fast, hecl::blender::SharedBlenderToken,
                       [&](const SystemChar* extra) { progress.reportFile(override, extra); });
        }
      }
    }
  }
}

static void VisitDirectory(const ProjectPath& dir, bool recursive, bool force, bool fast,
                           std::vector<std::unique_ptr<IDataSpec>>& specInsts, CookProgress& progress,
                           ClientProcess* cp) {
  if (dir.getLastComponent().size() > 1 && dir.getLastComponent()[0] == _SYS_STR('.'))
    return;

  if (hecl::ProjectPath(dir, _SYS_STR("!project.yaml")).isFile() &&
      hecl::ProjectPath(dir, _SYS_STR("!pool.yaml")).isFile()) {
    /* Handle AudioGroup case */
    VisitFile(dir, force, fast, specInsts, progress, cp);
    return;
  }

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
  for (auto& child : children) {
    if (child.second.getPathType() == ProjectPath::Type::File) {
      progress.changeFile(child.first.c_str(), progNum++ / progDenom);
      VisitFile(child.second, force, fast, specInsts, progress, cp);
    }
  }
  progress.reportDirComplete();

  /* Pass 3: child directories */
  if (recursive) {
    for (auto& child : children) {
      switch (child.second.getPathType()) {
      case ProjectPath::Type::Directory: {
        VisitDirectory(child.second, recursive, force, fast, specInsts, progress, cp);
        break;
      }
      default:
        break;
      }
    }
  }
}

bool Project::cookPath(const ProjectPath& path, const hecl::MultiProgressPrinter& progress, bool recursive, bool force,
                       bool fast, const DataSpecEntry* spec, ClientProcess* cp) {
  /* Construct DataSpec instances for cooking */
  if (spec) {
    if (m_cookSpecs.size() != 1 || m_cookSpecs[0]->getDataSpecEntry() != spec) {
      m_cookSpecs.clear();
      if (spec->m_factory)
        m_cookSpecs.push_back(spec->m_factory(*this, DataSpecTool::Cook));
    }
  } else if (m_cookSpecs.empty()) {
    m_cookSpecs.reserve(m_compiledSpecs.size());
    for (const ProjectDataSpec& projectSpec : m_compiledSpecs) {
      if (projectSpec.active && projectSpec.spec.m_factory) {
        m_cookSpecs.push_back(projectSpec.spec.m_factory(*this, DataSpecTool::Cook));
      }
    }
  }

  /* Iterate complete directory/file/glob list */
  CookProgress cookProg(progress);
  switch (path.getPathType()) {
  case ProjectPath::Type::File:
  case ProjectPath::Type::Glob: {
    cookProg.changeFile(path.getLastComponent().data(), 0.f);
    VisitFile(path, force, fast, m_cookSpecs, cookProg, cp);
    break;
  }
  case ProjectPath::Type::Directory: {
    VisitDirectory(path, recursive, force, fast, m_cookSpecs, cookProg, cp);
    break;
  }
  default:
    break;
  }

  return true;
}

bool Project::packagePath(const ProjectPath& path, const hecl::MultiProgressPrinter& progress, bool fast,
                          const DataSpecEntry* spec, ClientProcess* cp) {
  /* Construct DataSpec instance for packaging */
  const DataSpecEntry* specEntry = nullptr;
  if (spec) {
    if (spec->m_factory) {
      specEntry = spec;
    }
  } else {
    bool foundPC = false;
    for (const ProjectDataSpec& projectSpec : m_compiledSpecs) {
      if (projectSpec.active && projectSpec.spec.m_factory) {
        if (hecl::StringUtils::EndsWith(projectSpec.spec.m_name, _SYS_STR("-PC"))) {
          foundPC = true;
          specEntry = &projectSpec.spec;
        } else if (!foundPC) {
          specEntry = &projectSpec.spec;
        }
      }
    }
  }

  if (!specEntry)
    LogModule.report(logvisor::Fatal, fmt("No matching DataSpec"));

  if (!m_lastPackageSpec || m_lastPackageSpec->getDataSpecEntry() != specEntry)
    m_lastPackageSpec = specEntry->m_factory(*this, DataSpecTool::Package);

  if (m_lastPackageSpec->canPackage(path)) {
    m_lastPackageSpec->doPackage(path, specEntry, fast, hecl::blender::SharedBlenderToken, progress, cp);
    return true;
  }

  return false;
}

void Project::interruptCook() {
  if (m_lastPackageSpec)
    m_lastPackageSpec->interruptCook();
}

bool Project::cleanPath(const ProjectPath& path, bool recursive) { return false; }

PackageDepsgraph Project::buildPackageDepsgraph(const ProjectPath& path) { return PackageDepsgraph(); }

void Project::addBridgePathToCache(uint64_t id, const ProjectPath& path) { m_bridgePathCache[id] = path; }

void Project::clearBridgePathCache() { m_bridgePathCache.clear(); }

const ProjectPath* Project::lookupBridgePath(uint64_t id) const {
  auto search = m_bridgePathCache.find(id);
  if (search == m_bridgePathCache.cend())
    return nullptr;
  return &search->second;
}

} // namespace hecl::Database
