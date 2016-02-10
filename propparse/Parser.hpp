#ifndef PARSER_H
#define PARSER_H

#include <tinyxml2.h>

class Parser
{
    tinyxml2::XMLDocument m_document;
    bool parseProperties();

    bool parseScriptObject();

    bool parseStruct();

    FILE* m_parametersDest = nullptr;
    bool m_isMP3;
public:
    Parser(const char* parametersDest, bool isMP3) : m_isMP3(isMP3) { m_parametersDest = fopen(parametersDest, "w"); }
    ~Parser() { if (m_parametersDest) fclose(m_parametersDest); }

    bool parse(const char* file)
    {
        if (!m_parametersDest)
            return false;
        if (m_document.LoadFile(file) != tinyxml2::XML_SUCCESS)
            return false;
        tinyxml2::XMLElement* root = m_document.FirstChildElement();
        if (!root)
            return false;

        if (!strcasecmp(root->Name(), "Properties"))
            return parseProperties();
        if (!strcasecmp(root->Name(), "ScriptObject"))
            return parseScriptObject();
        if (!strcasecmp(root->Name(), "Struct"))
            return parseStruct();

        return false;
    }
};

#endif // PARSER_H
