#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace hecl::shaderc {

class Compiler {
  enum class StageType { Vertex, Fragment, Geometry, Control, Evaluation };

  std::vector<std::string> m_inputFiles;
  std::unordered_map<std::string, std::string> m_fileContents;
  const std::string* getFileContents(std::string_view path);
  std::unordered_map<std::string, std::string> m_defines;
  template <typename Action, typename P>
  static bool StageAction(StageType type, const std::string& name, const std::string& basename,
                          const std::string& stage, std::stringstream& implOut);
  template <typename Action>
  static bool StageAction(const std::string& platforms, StageType type, const std::string& name,
                          const std::string& basename, const std::string& stage, std::stringstream& implOut);
  bool includeFile(std::string_view file, std::string& out, int depth = 0);
  bool compileFile(std::string_view file, std::string_view baseName,
                   std::pair<std::stringstream, std::stringstream>& out);

public:
  void addInputFile(std::string_view file);
  void addDefine(std::string_view var, std::string_view val);
  bool compile(std::string_view baseName, std::pair<std::stringstream, std::stringstream>& out);
};

} // namespace hecl::shaderc
