#include "Specter/Translator.hpp"
#include <LogVisor/LogVisor.hpp>

namespace Specter
{
static LogVisor::LogModule Log("Specter::Translator");

Locale::Locale(const std::string& name, const std::string& fullName,
               const unsigned char* yamlSource, size_t yamlLength)
: m_name(name), m_fullName(fullName)
{
    Athena::io::YAMLDocReader reader;
    yaml_parser_set_input_string(reader.getParser(), yamlSource, yamlLength);
    reader.parse();
    m_rootNode = std::move(reader.releaseRootNode());
    if (m_rootNode)
    {
        m_langNode = m_rootNode->findMapChild(name.c_str());
        if (!m_langNode)
            Log.report(LogVisor::FatalError, "no root node '%s' found in locale", name.c_str());
    }
    else
        Log.report(LogVisor::Warning, "locale empty");
}

void Translator::setLocale(const Locale* targetLocale)
{
    if (!targetLocale)
        Log.report(LogVisor::FatalError, "null locale");
    m_targetLocale = targetLocale;
}

static const std::string* RecursiveLookup(const Athena::io::YAMLNode& node,
                                          std::string::const_iterator start,
                                          std::string::const_iterator end)
{
    for (std::string::const_iterator it = start ; it != end ; ++it)
    {
        if (*it == '/')
        {
            const Athena::io::YAMLNode* ch = node.findMapChild(std::string(start, it).c_str());
            if (!ch)
                return nullptr;
            return RecursiveLookup(*ch, it+1, end);
        }
    }
    const Athena::io::YAMLNode* ch = node.findMapChild(std::string(start, end).c_str());
    if (!ch)
        return nullptr;
    return &ch->m_scalarString;
}

const std::string* Translator::translate(const std::string& key) const
{
    if ((&m_targetLocale->rootNode()) == nullptr)
        return nullptr;

    return RecursiveLookup(m_targetLocale->rootNode(), key.cbegin(), key.cend());
}

std::string Translator::translateOr(const std::string& key, const char* vor) const
{
    if ((&m_targetLocale->rootNode()) == nullptr)
        return vor;

    const std::string* find = RecursiveLookup(m_targetLocale->rootNode(), key.cbegin(), key.cend());
    if (find)
        return *find;
    return vor;
}

}
