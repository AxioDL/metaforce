#include "specter/Translator.hpp"
#include "logvisor/logvisor.hpp"

namespace specter {
static logvisor::Module Log("specter::Translator");

Locale::Locale(std::string_view name, std::string_view fullName, const unsigned char* yamlSource, size_t yamlLength)
: m_name(name), m_fullName(fullName) {
  athena::io::YAMLDocReader reader;
  yaml_parser_set_input_string(reader.getParser(), yamlSource, yamlLength);
  reader.parse(nullptr);
  m_rootNode = reader.releaseRootNode();
  if (m_rootNode) {
    m_langNode = m_rootNode->findMapChild(name.data());
    if (!m_langNode)
      Log.report(logvisor::Fatal, "no root node '%s' found in locale", name.data());
  } else
    Log.report(logvisor::Warning, "locale empty");
}

void Translator::setLocale(const Locale* targetLocale) {
  if (!targetLocale)
    Log.report(logvisor::Fatal, "null locale");
  m_targetLocale = targetLocale;
}

static std::string_view RecursiveLookup(const athena::io::YAMLNode* node, std::string_view::const_iterator start,
                                        std::string_view::const_iterator end) {
  for (std::string_view::const_iterator it = start; it != end; ++it) {
    if (*it == '/') {
      const athena::io::YAMLNode* ch = node->findMapChild(std::string(start, it));
      if (!ch)
        return nullptr;
      return RecursiveLookup(ch, it + 1, end);
    }
  }
  const athena::io::YAMLNode* ch = node->findMapChild(std::string(start, end));
  if (!ch)
    return {};
  return ch->m_scalarString;
}

std::string_view Translator::translate(std::string_view key) const {
  if (!m_targetLocale->rootNode())
    return nullptr;

  return RecursiveLookup(m_targetLocale->rootNode(), key.cbegin(), key.cend());
}

std::string_view Translator::translateOr(std::string_view key, std::string_view vor) const {
  std::string_view find = translate(key);
  if (!find.empty())
    return find;
  return vor;
}

} // namespace specter
