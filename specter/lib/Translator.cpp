#include "specter/Translator.hpp"
#include "logvisor/logvisor.hpp"

namespace specter
{
static logvisor::Module Log("specter::Translator");

Locale::Locale(const std::string& name, const std::string& fullName,
               const unsigned char* yamlSource, size_t yamlLength)
: m_name(name), m_fullName(fullName)
{
    athena::io::YAMLDocReader reader;
    yaml_parser_set_input_string(reader.getParser(), yamlSource, yamlLength);
    reader.parse(nullptr);
    m_rootNode = std::move(reader.releaseRootNode());
    if (m_rootNode)
    {
        m_langNode = m_rootNode->findMapChild(name.c_str());
        if (!m_langNode)
            Log.report(logvisor::Fatal, "no root node '%s' found in locale", name.c_str());
    }
    else
        Log.report(logvisor::Warning, "locale empty");
}

void Translator::setLocale(const Locale* targetLocale)
{
    if (!targetLocale)
        Log.report(logvisor::Fatal, "null locale");
    m_targetLocale = targetLocale;
}

static const std::string* RecursiveLookup(const athena::io::YAMLNode* node,
                                          std::string::const_iterator start,
                                          std::string::const_iterator end)
{
    for (std::string::const_iterator it = start ; it != end ; ++it)
    {
        if (*it == '/')
        {
            const athena::io::YAMLNode* ch = node->findMapChild(std::string(start, it).c_str());
            if (!ch)
                return nullptr;
            return RecursiveLookup(ch, it+1, end);
        }
    }
    const athena::io::YAMLNode* ch = node->findMapChild(std::string(start, end).c_str());
    if (!ch)
        return nullptr;
    return &ch->m_scalarString;
}

const std::string* Translator::translate(const std::string& key) const
{
    if (!m_targetLocale->rootNode())
        return nullptr;

    return RecursiveLookup(m_targetLocale->rootNode(), key.cbegin(), key.cend());
}

std::string Translator::translateOr(const std::string& key, const char* vor) const
{
    const std::string* find = translate(key);
    if (find)
        return *find;
    return vor;
}

}
