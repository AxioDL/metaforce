#include "hecl/hecl.hpp"

#include <regex>

#include "hecl/Database.hpp"
#include "hecl/FourCC.hpp"

namespace hecl {
static const SystemRegex regPATHCOMP(_SYS_STR("[/\\\\]*([^/\\\\]+)"), SystemRegex::ECMAScript | SystemRegex::optimize);
static const SystemRegex regDRIVELETTER(_SYS_STR("^([^/]*)/"), SystemRegex::ECMAScript | SystemRegex::optimize);

static SystemString CanonRelPath(SystemStringView path) {
  /* Tokenize Path */
  std::vector<SystemString> comps;
  hecl::SystemRegexMatch matches;
  SystemString in(path);
  SanitizePath(in);
  for (; std::regex_search(in, matches, regPATHCOMP); in = matches.suffix().str()) {
    hecl::SystemRegexMatch::const_reference match = matches[1];
    if (!match.compare(_SYS_STR(".")))
      continue;
    else if (!match.compare(_SYS_STR(".."))) {
      if (comps.empty()) {
        /* Unable to resolve outside project */
        LogModule.report(logvisor::Fatal, fmt(_SYS_STR("Unable to resolve outside project root in {}")), path);
        return _SYS_STR(".");
      }
      comps.pop_back();
      continue;
    }
    comps.push_back(match.str());
  }

  /* Emit relative path */
  if (comps.size()) {
    auto it = comps.begin();
    SystemString retval = *it;
    for (++it; it != comps.end(); ++it) {
      if ((*it).size()) {
        retval += _SYS_STR('/');
        retval += *it;
      }
    }
    return retval;
  }
  return _SYS_STR(".");
}

static SystemString CanonRelPath(SystemStringView path, const ProjectRootPath& projectRoot) {
  /* Absolute paths not allowed; attempt to make project-relative */
  if (IsAbsolute(path))
    return CanonRelPath(projectRoot.getProjectRelativeFromAbsolute(path));
  return CanonRelPath(path);
}

void ProjectPath::assign(Database::Project& project, SystemStringView path) {
  m_proj = &project;

  SystemString usePath;
  size_t pipeFind = path.rfind(_SYS_STR('|'));
  if (pipeFind != SystemString::npos) {
    m_auxInfo.assign(path.cbegin() + pipeFind + 1, path.cend());
    usePath.assign(path.cbegin(), path.cbegin() + pipeFind);
  } else
    usePath = path;

  m_relPath = CanonRelPath(usePath, project.getProjectRootPath());
  m_absPath = SystemString(project.getProjectRootPath().getAbsolutePath()) + _SYS_STR('/') + m_relPath;
  SanitizePath(m_relPath);
  SanitizePath(m_absPath);

  ComputeHash();
}

#if HECL_UCS2
void ProjectPath::assign(Database::Project& project, std::string_view path) {
  std::wstring wpath = UTF8ToWide(path);
  assign(project, wpath);
}
#endif

void ProjectPath::assign(const ProjectPath& parentPath, SystemStringView path) {
  m_proj = parentPath.m_proj;

  SystemString usePath;
  size_t pipeFind = path.rfind(_SYS_STR('|'));
  if (pipeFind != SystemString::npos) {
    m_auxInfo.assign(path.cbegin() + pipeFind + 1, path.cend());
    usePath.assign(path.cbegin(), path.cbegin() + pipeFind);
  } else
    usePath = path;

  m_relPath = CanonRelPath(parentPath.m_relPath + _SYS_STR('/') + usePath);
  m_absPath = SystemString(m_proj->getProjectRootPath().getAbsolutePath()) + _SYS_STR('/') + m_relPath;
  SanitizePath(m_relPath);
  SanitizePath(m_absPath);

  ComputeHash();
}

#if HECL_UCS2
void ProjectPath::assign(const ProjectPath& parentPath, std::string_view path) {
  std::wstring wpath = UTF8ToWide(path);
  assign(parentPath, wpath);
}
#endif

ProjectPath ProjectPath::getWithExtension(const SystemChar* ext, bool replace) const {
  ProjectPath pp(*this);
  if (replace) {
    auto relIt = pp.m_relPath.end();
    if (relIt != pp.m_relPath.begin())
      --relIt;
    auto absIt = pp.m_absPath.end();
    if (absIt != pp.m_absPath.begin())
      --absIt;
    while (relIt != pp.m_relPath.begin() && *relIt != _SYS_STR('.') && *relIt != _SYS_STR('/')) {
      --relIt;
      --absIt;
    }
    if (*relIt == _SYS_STR('.') && relIt != pp.m_relPath.begin()) {
      pp.m_relPath.resize(relIt - pp.m_relPath.begin());
      pp.m_absPath.resize(absIt - pp.m_absPath.begin());
    }
  }
  if (ext) {
    pp.m_relPath += ext;
    pp.m_absPath += ext;
  }

  pp.ComputeHash();
  return pp;
}

ProjectPath ProjectPath::getCookedPath(const Database::DataSpecEntry& spec) const {
  ProjectPath woExt = getWithExtension(nullptr, true);
  ProjectPath ret(m_proj->getProjectCookedPath(spec), woExt.getRelativePath());

  if (getAuxInfo().size())
    return ret.getWithExtension((SystemString(_SYS_STR(".")) + getAuxInfo().data()).c_str());
  else
    return ret;
}

ProjectPath::Type ProjectPath::getPathType() const {
  if (m_absPath.find(_SYS_STR('*')) != SystemString::npos)
    return Type::Glob;
  Sstat theStat;
  if (hecl::Stat(m_absPath.c_str(), &theStat))
    return Type::None;
  if (S_ISDIR(theStat.st_mode))
    return Type::Directory;
  if (S_ISREG(theStat.st_mode))
    return Type::File;
  return Type::None;
}

Time ProjectPath::getModtime() const {
  Sstat theStat;
  time_t latestTime = 0;
  if (m_absPath.find(_SYS_STR('*')) != SystemString::npos) {
    std::vector<ProjectPath> globResults;
    getGlobResults(globResults);
    for (ProjectPath& path : globResults) {
      if (!hecl::Stat(path.getAbsolutePath().data(), &theStat)) {
        if (S_ISREG(theStat.st_mode) && theStat.st_mtime > latestTime)
          latestTime = theStat.st_mtime;
      }
    }
    return Time(latestTime);
  }
  if (!hecl::Stat(m_absPath.c_str(), &theStat)) {
    if (S_ISREG(theStat.st_mode)) {
      return Time(theStat.st_mtime);
    } else if (S_ISDIR(theStat.st_mode)) {
      hecl::DirectoryEnumerator de(m_absPath, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
      for (const hecl::DirectoryEnumerator::Entry& ent : de) {
        if (!hecl::Stat(ent.m_path.c_str(), &theStat)) {
          if (S_ISREG(theStat.st_mode) && theStat.st_mtime > latestTime)
            latestTime = theStat.st_mtime;
        }
      }
      return Time(latestTime);
    }
  }
  LogModule.report(logvisor::Fatal, fmt(_SYS_STR("invalid path type for computing modtime in '{}'")), m_absPath);
  return Time();
}

static void _recursiveGlob(Database::Project& proj, std::vector<ProjectPath>& outPaths, const SystemString& remPath,
                           const SystemString& itStr, bool needSlash) {
  SystemRegexMatch matches;
  if (!std::regex_search(remPath, matches, regPATHCOMP))
    return;

  const SystemString& comp = matches[1];
  if (comp.find(_SYS_STR('*')) == SystemString::npos) {
    SystemString nextItStr = itStr;
    if (needSlash)
      nextItStr += _SYS_STR('/');
    nextItStr += comp;

    hecl::Sstat theStat;
    if (Stat(nextItStr.c_str(), &theStat))
      return;

    if (S_ISDIR(theStat.st_mode))
      _recursiveGlob(proj, outPaths, matches.suffix().str(), nextItStr, true);
    else
      outPaths.emplace_back(proj, nextItStr);
    return;
  }

  /* Compile component into regex */
  SystemRegex regComp(comp, SystemRegex::ECMAScript);

  hecl::DirectoryEnumerator de(itStr, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
  for (const hecl::DirectoryEnumerator::Entry& ent : de) {
    if (std::regex_match(ent.m_name, regComp)) {
      SystemString nextItStr = itStr;
      if (needSlash)
        nextItStr += '/';
      nextItStr += ent.m_name;

      hecl::Sstat theStat;
      if (Stat(nextItStr.c_str(), &theStat))
        continue;

      if (ent.m_isDir)
        _recursiveGlob(proj, outPaths, matches.suffix().str(), nextItStr, true);
      else
        outPaths.emplace_back(proj, nextItStr);
    }
  }
}

void ProjectPath::getDirChildren(std::map<SystemString, ProjectPath>& outPaths) const {
  hecl::DirectoryEnumerator de(m_absPath, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
  for (const hecl::DirectoryEnumerator::Entry& ent : de)
    outPaths[ent.m_name] = ProjectPath(*this, ent.m_name);
}

hecl::DirectoryEnumerator ProjectPath::enumerateDir() const {
  return hecl::DirectoryEnumerator(m_absPath, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
}

void ProjectPath::getGlobResults(std::vector<ProjectPath>& outPaths) const {
  auto rootPath = m_proj->getProjectRootPath().getAbsolutePath();
  _recursiveGlob(*m_proj, outPaths, m_relPath, rootPath.data(), rootPath.back() != _SYS_STR('/'));
}

ProjectRootPath SearchForProject(SystemStringView path) {
  ProjectRootPath testRoot(path);
  auto begin = testRoot.getAbsolutePath().begin();
  auto end = testRoot.getAbsolutePath().end();
  while (begin != end) {
    SystemString testPath(begin, end);
    SystemString testIndexPath = testPath + _SYS_STR("/.hecl/beacon");
    Sstat theStat;
    if (!hecl::Stat(testIndexPath.c_str(), &theStat)) {
      if (S_ISREG(theStat.st_mode)) {
        const auto fp = hecl::FopenUnique(testIndexPath.c_str(), _SYS_STR("rb"));
        if (fp == nullptr) {
          continue;
        }

        char magic[4];
        const size_t readSize = std::fread(magic, 1, sizeof(magic), fp.get());
        if (readSize != sizeof(magic)) {
          continue;
        }

        static constexpr hecl::FourCC hecl("HECL");
        if (hecl::FourCC(magic) != hecl) {
          continue;
        }

        return ProjectRootPath(testPath);
      }
    }

    while (begin != end && *(end - 1) != _SYS_STR('/') && *(end - 1) != _SYS_STR('\\'))
      --end;
    if (begin != end)
      --end;
  }
  return ProjectRootPath();
}

ProjectRootPath SearchForProject(SystemStringView path, SystemString& subpathOut) {
  const ProjectRootPath testRoot(path);
  auto begin = testRoot.getAbsolutePath().begin();
  auto end = testRoot.getAbsolutePath().end();

  while (begin != end) {
    SystemString testPath(begin, end);
    SystemString testIndexPath = testPath + _SYS_STR("/.hecl/beacon");
    Sstat theStat;

    if (!hecl::Stat(testIndexPath.c_str(), &theStat)) {
      if (S_ISREG(theStat.st_mode)) {
        const auto fp = hecl::FopenUnique(testIndexPath.c_str(), _SYS_STR("rb"));
        if (fp == nullptr) {
          continue;
        }

        char magic[4];
        const size_t readSize = std::fread(magic, 1, sizeof(magic), fp.get());
        if (readSize != sizeof(magic)) {
          continue;
        }
        if (hecl::FourCC(magic) != FOURCC('HECL')) {
          continue;
        }

        const ProjectRootPath newRootPath = ProjectRootPath(testPath);
        const auto origEnd = testRoot.getAbsolutePath().end();
        while (end != origEnd && *end != _SYS_STR('/') && *end != _SYS_STR('\\')) {
          ++end;
        }
        if (end != origEnd && (*end == _SYS_STR('/') || *end == _SYS_STR('\\'))) {
          ++end;
        }

        subpathOut.assign(end, origEnd);
        return newRootPath;
      }
    }

    while (begin != end && *(end - 1) != _SYS_STR('/') && *(end - 1) != _SYS_STR('\\')) {
      --end;
    }
    if (begin != end) {
      --end;
    }
  }
  return ProjectRootPath();
}

} // namespace hecl
