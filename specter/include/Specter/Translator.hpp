#ifndef SPECTER_TRANSLATOR_HPP
#define SPECTER_TRANSLATOR_HPP

#include <string>
#include <Athena/DNAYaml.hpp>

namespace Specter
{

class Locale
{
    std::string m_name;
    std::string m_fullName;
    std::unique_ptr<Athena::io::YAMLNode> m_rootNode;
    const Athena::io::YAMLNode* m_langNode;
public:
    Locale(const std::string& name, const std::string& fullName,
           const unsigned char* yamlSource, size_t yamlLength);
    const std::string& name() const {return m_name;}
    const std::string& fullName() const {return m_fullName;}
    const Athena::io::YAMLNode& rootNode() const {return *m_langNode;}
};

class Translator
{
    const Locale* m_targetLocale;
public:
    Translator(const Locale* targetLocale) {setLocale(targetLocale);}
    void setLocale(const Locale* targetLocale);
    const std::string* translate(const std::string& key) const;
    std::string translateOr(const std::string& key, const char* vor) const;
};

}

#endif // SPECTER_TRANSLATOR_HPP
