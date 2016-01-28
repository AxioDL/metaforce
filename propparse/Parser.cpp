#include "Parser.hpp"

bool Parser::parseProperties()
{
    return true;
}

bool Parser::parseScriptObject()
{
    return true;
}

bool Parser::parseStruct()
{
    return true;
}

bool Parser::parse(const char* file)
{
    if (!dest)
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
