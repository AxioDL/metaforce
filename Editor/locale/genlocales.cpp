#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include "athena/FileReader.hpp"
#include "athena/YAMLDocReader.hpp"

#define FMT_STRING_ALIAS 1
#define FMT_ENFORCE_COMPILE_STRING 1
#define FMT_USE_GRISU 0
#include <fmt/format.h>
#include <fmt/ostream.h>

int main(int argc, char** argv) {
  if (argc < 3) {
    fmt::print(fmt("{} <out-header> <in-yamls>...\n"), argv[0]);
    return 1;
  }

  std::ofstream out(argv[1]);
  if (!out.is_open()) {
    fmt::print(fmt("Unable to open {} for writing\n"), argv[1]);
    return 1;
  }

  std::unordered_set<std::string> seenLocales;
  std::stringstream enumLocales;
  std::stringstream declLocales;
  std::unordered_set<std::string> seenKeys;
  std::stringstream keys;
  std::stringstream lookups;
  std::stringstream dos;

  for (int i = 2; i < argc; ++i) {
    athena::io::FileReader fr(argv[i]);
    if (!fr.isOpen()) {
      fmt::print(fmt("Unable to open {} for reading\n"), argv[i]);
      return 1;
    }
    athena::io::YAMLDocReader r;
    if (!r.parse(&fr)) {
      fmt::print(fmt("Unable to parse {}\n"), argv[i]);
      return 1;
    }

    std::string name;
    std::string fullName;
    athena::io::YAMLNode* listNode = nullptr;
    for (const auto& c : r.getRootNode()->m_mapChildren) {
      if (c.first == "name") {
        fullName = c.second->m_scalarString;
      } else {
        name = c.first;
        listNode = c.second.get();
      }
    }
    if (fullName.empty()) {
      fmt::print(fmt("Unable to find 'name' node in {}\n"), argv[i]);
      return 1;
    }
    if (!listNode) {
      fmt::print(fmt("Unable to find list node in {}\n"), argv[i]);
      return 1;
    }

    if (seenLocales.find(name) == seenLocales.end()) {
      seenLocales.insert(name);
      fmt::print(enumLocales, fmt("  {},\n"), name);
      fmt::print(declLocales,
        fmt("struct {0} {{ static constexpr auto Name = \"{0}\"sv; static constexpr auto FullName = \"{1}\"sv; }};\n"),
        name, fullName);
      fmt::print(dos,
        fmt("    case ELocale::{0}:\n"
            "      return act.template Do<{0}>(std::forward<Args>(args)...);\n"), name);
      fmt::print(lookups, fmt("/* {} */\n"), name);
      for (const auto& k : listNode->m_mapChildren) {
        if (seenKeys.find(k.first) == seenKeys.end()) {
          seenKeys.insert(k.first);
          fmt::print(keys, fmt("struct {} {{}};\n"), k.first);
        }
        fmt::print(lookups,
          fmt("template<> struct Lookup<{}, {}> {{ static constexpr auto Value() {{ return fmt(\"{}\"); }} }};\n"),
          name, k.first, k.second->m_scalarString);
      }
    }
    lookups << '\n';
  }

  out << "/* Locales */\n"
         "enum class ELocale {\n"
         "  Invalid = -1,\n";
  out << enumLocales.str();
  out << "  MAXLocale\n"
         "};\n";
  out << declLocales.str();
  out << "\n"
         "using DefaultLocale = en_US;\n"
         "template<typename L, typename K> struct Lookup {\n"
         "  static_assert(!std::is_same_v<L, DefaultLocale>, \"The default locale must translate all keys\");\n"
         "  static constexpr auto Value() { return Lookup<DefaultLocale, K>::Value(); }\n"
         "};\n"
         "\n"
         "/* Keys */\n";
  out << keys.str();
  out << "\n";
  out << lookups.str();
  out << "template <typename Action, typename... Args>\n"
         "constexpr auto Do(ELocale l, Action act, Args&&... args) {\n"
         "  switch (l) {\n"
         "    default:\n";
  out << dos.str();
  out << "  }\n"
         "}\n";

  return 0;
}
