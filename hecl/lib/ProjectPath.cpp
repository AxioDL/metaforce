#include "hecl/hecl.hpp"

#include <regex>

#include "hecl/Database.hpp"
#include "hecl/FourCC.hpp"

namespace hecl {
static const std::regex regPATHCOMP("[/\\\\]*([^/\\\\]+)", std::regex::ECMAScript | std::regex::optimize);

static std::string CanonRelPath(std::string_view path) {
  /* Tokenize Path */
  std::vector<std::string> comps;
  std::smatch matches;
  std::string in(path);
  SanitizePath(in);
  for (; std::regex_search(in, matches, regPATHCOMP); in = matches.suffix().str()) {
    std::smatch::const_reference match = matches[1];
    if (match == ".")
      continue;
    else if (match == "..") {
      if (comps.empty()) {
        /* Unable to resolve outside project */
        LogModule.report(logvisor::Fatal, FMT_STRING("Unable to resolve outside project root in {}"), path);
        return ".";
      }
      comps.pop_back();
      continue;
    }
    comps.push_back(match.str());
  }

  /* Emit relative path */
  if (comps.size()) {
    auto it = comps.begin();
    std::string retval = *it;
    for (++it; it != comps.end(); ++it) {
      if ((*it).size()) {
        retval += '/';
        retval += *it;
      }
    }
    return retval;
  }
  return ".";
}

static std::string CanonRelPath(std::string_view path, const ProjectRootPath& projectRoot) {
  /* Absolute paths not allowed; attempt to make project-relative */
  if (IsAbsolute(path))
    return CanonRelPath(projectRoot.getProjectRelativeFromAbsolute(path));
  return CanonRelPath(path);
}

void ProjectPath::assign(Database::Project& project, std::string_view path) {
  m_proj = &project;

  std::string usePath;
  size_t pipeFind = path.rfind('|');
  if (pipeFind != std::string::npos) {
    m_auxInfo.assign(path.cbegin() + pipeFind + 1, path.cend());
    usePath.assign(path.cbegin(), path.cbegin() + pipeFind);
  } else
    usePath = path;

  m_relPath = CanonRelPath(usePath, project.getProjectRootPath());
  m_absPath = std::string(project.getProjectRootPath().getAbsolutePath()) + '/' + m_relPath;
  SanitizePath(m_relPath);
  SanitizePath(m_absPath);

  ComputeHash();
}

void ProjectPath::assign(const ProjectPath& parentPath, std::string_view path) {
  m_proj = parentPath.m_proj;

  std::string usePath;
  size_t pipeFind = path.rfind('|');
  if (pipeFind != std::string::npos) {
    m_auxInfo.assign(path.cbegin() + pipeFind + 1, path.cend());
    usePath.assign(path.cbegin(), path.cbegin() + pipeFind);
  } else
    usePath = path;

  m_relPath = CanonRelPath(parentPath.m_relPath + '/' + usePath);
  m_absPath = std::string(m_proj->getProjectRootPath().getAbsolutePath()) + '/' + m_relPath;
  SanitizePath(m_relPath);
  SanitizePath(m_absPath);

  ComputeHash();
}

ProjectPath ProjectPath::getWithExtension(const char* ext, bool replace) const {
  ProjectPath pp(*this);
  if (replace) {
    auto relIt = pp.m_relPath.end();
    if (relIt != pp.m_relPath.begin())
      --relIt;
    auto absIt = pp.m_absPath.end();
    if (absIt != pp.m_absPath.begin())
      --absIt;
    while (relIt != pp.m_relPath.begin() && *relIt != '.' && *relIt != '/') {
      --relIt;
      --absIt;
    }
    if (*relIt == '.' && relIt != pp.m_relPath.begin()) {
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
    return ret.getWithExtension((std::string(".") + getAuxInfo().data()).c_str());
  else
    return ret;
}

ProjectPath::Type ProjectPath::getPathType() const {
  if (m_absPath.empty())
    return Type::None;
  if (m_absPath.find('*') != std::string::npos)
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
  if (m_absPath.find('*') != std::string::npos) {
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
  LogModule.report(logvisor::Fatal, FMT_STRING("invalid path type for computing modtime in '{}'"), m_absPath);
  return Time();
}

static void _recursiveGlob(Database::Project& proj, std::vector<ProjectPath>& outPaths, const std::string& remPath,
                           const std::string& itStr, bool needSlash) {
  std::smatch matches;
  if (!std::regex_search(remPath, matches, regPATHCOMP))
    return;

  const std::string& comp = matches[1];
  if (comp.find('*') == std::string::npos) {
    std::string nextItStr = itStr;
    if (needSlash)
      nextItStr += '/';
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
  std::regex regComp(comp, std::regex::ECMAScript);

  hecl::DirectoryEnumerator de(itStr, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
  for (const hecl::DirectoryEnumerator::Entry& ent : de) {
    if (std::regex_match(ent.m_name, regComp)) {
      std::string nextItStr = itStr;
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

void ProjectPath::getDirChildren(std::map<std::string, ProjectPath>& outPaths) const {
  hecl::DirectoryEnumerator de(m_absPath, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
  for (const hecl::DirectoryEnumerator::Entry& ent : de)
    outPaths[ent.m_name] = ProjectPath(*this, ent.m_name);
}

hecl::DirectoryEnumerator ProjectPath::enumerateDir() const {
  return hecl::DirectoryEnumerator(m_absPath, hecl::DirectoryEnumerator::Mode::DirsThenFilesSorted, false, false, true);
}

void ProjectPath::getGlobResults(std::vector<ProjectPath>& outPaths) const {
  auto rootPath = m_proj->getProjectRootPath().getAbsolutePath();
  _recursiveGlob(*m_proj, outPaths, m_relPath, rootPath.data(), rootPath.back() != '/');
}

template <typename T>
static bool RegexSearchLast(const T& str, std::match_results<typename T::const_iterator>& m,
                            const std::basic_regex<typename T::value_type>& reg) {
  using Iterator = std::regex_iterator<typename T::const_iterator>;
  Iterator begin = Iterator(str.begin(), str.end(), reg);
  Iterator end = Iterator();
  if (begin == end)
    return false;
  Iterator last_it;
  for (auto it = begin; it != end; ++it)
    last_it = it;
  m = *last_it;
  return true;
}

static const std::regex regParsedHash32(R"(_([0-9a-fA-F]{8}))",
                                               std::regex::ECMAScript | std::regex::optimize);
uint32_t ProjectPath::parsedHash32() const {
  if (!m_auxInfo.empty()) {
    std::smatch match;
    if (RegexSearchLast(m_auxInfo, match, regParsedHash32)) {
      auto hexStr = match[1].str();
      if (auto val = hecl::StrToUl(hexStr.c_str(), nullptr, 16))
        return val;
    }
  } else {
    std::match_results<std::string_view::const_iterator> match;
    if (RegexSearchLast(getLastComponent(), match, regParsedHash32)) {
      auto hexStr = match[1].str();
      if (auto val = hecl::StrToUl(hexStr.c_str(), nullptr, 16))
        return val;
    }
  }
  return hash().val32();
}

ProjectRootPath SearchForProject(std::string_view path) {
  ProjectRootPath testRoot(path);
  auto begin = testRoot.getAbsolutePath().begin();
  auto end = testRoot.getAbsolutePath().end();
  while (begin != end) {
    std::string testPath(begin, end);
    std::string testIndexPath = testPath + "/.hecl/beacon";
    Sstat theStat;
    if (!hecl::Stat(testIndexPath.c_str(), &theStat)) {
      if (S_ISREG(theStat.st_mode)) {
        const auto fp = hecl::FopenUnique(testIndexPath.c_str(), "rb");
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

    while (begin != end && *(end - 1) != '/' && *(end - 1) != '\\')
      --end;
    if (begin != end)
      --end;
  }
  return ProjectRootPath();
}

ProjectRootPath SearchForProject(std::string_view path, std::string& subpathOut) {
  const ProjectRootPath testRoot(path);
  auto begin = testRoot.getAbsolutePath().begin();
  auto end = testRoot.getAbsolutePath().end();

  while (begin != end) {
    std::string testPath(begin, end);
    std::string testIndexPath = testPath + "/.hecl/beacon";
    Sstat theStat;

    if (!hecl::Stat(testIndexPath.c_str(), &theStat)) {
      if (S_ISREG(theStat.st_mode)) {
        const auto fp = hecl::FopenUnique(testIndexPath.c_str(), "rb");
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
        while (end != origEnd && *end != '/' && *end != '\\') {
          ++end;
        }
        if (end != origEnd && (*end == '/' || *end == '\\')) {
          ++end;
        }

        subpathOut.assign(end, origEnd);
        return newRootPath;
      }
    }

    while (begin != end && *(end - 1) != '/' && *(end - 1) != '\\') {
      --end;
    }
    if (begin != end) {
      --end;
    }
  }
  return ProjectRootPath();
}

} // namespace hecl
