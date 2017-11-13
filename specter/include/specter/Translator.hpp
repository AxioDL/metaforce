#ifndef SPECTER_TRANSLATOR_HPP
#define SPECTER_TRANSLATOR_HPP

#include <string>
#include <athena/DNAYaml.hpp>

namespace specter
{

class Locale
{
    std::string_view m_name;
    std::string_view m_fullName;
    std::unique_ptr<athena::io::YAMLNode> m_rootNode;
    const athena::io::YAMLNode* m_langNode;
public:
    Locale(std::string_view name, std::string_view fullName,
           const unsigned char* yamlSource, size_t yamlLength);
    std::string_view name() const {return m_name;}
    std::string_view fullName() const {return m_fullName;}
    const athena::io::YAMLNode* rootNode() const {return m_langNode;}
};

class Translator
{
    const Locale* m_targetLocale;
public:
    Translator(const Locale* targetLocale) {setLocale(targetLocale);}
    void setLocale(const Locale* targetLocale);
    std::string_view translate(std::string_view key) const;
    std::string_view translateOr(std::string_view key, std::string_view vor) const;
};

}

#endif // SPECTER_TRANSLATOR_HPP
