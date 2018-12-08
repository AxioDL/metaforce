#pragma once
#include "hecl/SystemChar.hpp"
#include <string>
#include <vector>
#include <unordered_map>

namespace hecl::shaderc {

class Compiler {
  enum class StageType { Vertex, Fragment, Geometry, Control, Evaluation };

  std::vector<SystemString> m_inputFiles;
  std::unordered_map<SystemString, std::string> m_fileContents;
  const std::string* getFileContents(SystemStringView path);
  std::unordered_map<std::string, std::string> m_defines;
  template <typename Action, typename P>
  static bool StageAction(StageType type, const std::string& name, const std::string& basename,
                          const std::string& stage, std::string& implOut);
  template <typename Action>
  static bool StageAction(const std::string& platforms, StageType type, const std::string& name,
                          const std::string& basename, const std::string& stage, std::string& implOut);
  bool includeFile(SystemStringView file, std::string& out, int depth = 0);
  bool compileFile(SystemStringView file, std::string_view baseName, std::pair<std::string, std::string>& out);

public:
  void addInputFile(SystemStringView file);
  void addDefine(std::string_view var, std::string_view val);
  bool compile(std::string_view baseName, std::pair<std::string, std::string>& out);
};

} // namespace hecl::shaderc
