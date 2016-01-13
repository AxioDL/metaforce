#include <iostream>
#include <string.h>
#include <tinyxml2.h>

#include <unordered_map>
#include <map>
#include <functional>
#include <memory>
#include <Athena/IStreamReader.hpp>
#include <Athena/IStreamWriter.hpp>
#include <Athena/DNA.hpp>
#include <Athena/DNAYaml.hpp>

typedef char* FourCC;

#define EXPECTED_PROPERTY_VERSION 3

enum class ParseError : atInt32
{
    Success=0,
    InvalidArgs = -1,
    InvalidGame = -2,
    UnsupportedVersion = -3,
    InputFileNotFound = -4,
    OutputFileNotFound= -5
};

namespace Retro
{
namespace DNAM2
{

struct IPropertyBase
{
    atUint32 hash;

    IPropertyBase(atUint32 hash)
        : hash(hash)
    {
    }

    atUint32 id()
    {
        return hash;
    }

    virtual void read(Athena::io::IStreamReader& in) = 0;
    virtual void read(Athena::io::YAMLDocReader& in) = 0;
    virtual void write(Athena::io::IStreamWriter& out) const=0;
    virtual void write(Athena::io::YAMLDocWriter& out) const=0;
};

struct IPropertyBase;
template <class ValueType>
class IProperty : public IPropertyBase
{
    ValueType m_val;
public:
    IProperty(atUint32 hash)  : IPropertyBase(hash) { }

    ValueType value() const
    { return m_val; }

    virtual void read(Athena::io::IStreamReader& in)
    { m_val = in.readVal<ValueType>(); }
    virtual void read(Athena::io::YAMLDocReader& in)
    { m_val = in.readVal<ValueType>(""); }

    virtual void write(Athena::io::IStreamWriter& out) const
    { out.writeVal(m_val); }

    virtual void write(Athena::io::YAMLDocWriter& out) const
    { out.writeVal("", m_val); }
};

using Int32Property   = IProperty<atInt32>;
using FloatProperty   = IProperty<float>;
using Vector3Property = IProperty<atVec3f>;
using ColorProperty   = IProperty<atVec4f>;
using BoolProperty    = IProperty<bool>;
using StringProperty  = IProperty<std::string>;

class ResourceProperty : public IPropertyBase
{
    atUint32 m_resource; /* TODO: Replace with UniqueID32/UniqueID64 */
    FourCC m_type;
public:
    ResourceProperty(FourCC type, atUint32 hash) : IPropertyBase(hash), m_type(type) {}

    virtual void read(Athena::io::IStreamReader& in)
    { }

    virtual void read(Athena::io::YAMLDocReader& in)
    { }

    virtual void write(Athena::io::IStreamWriter& out) const
    { }

    virtual void write(Athena::io::YAMLDocWriter& out) const
    { }
};

struct IPropertyFactory
{
    atUint32 hash;
    const char* name;
    IPropertyFactory(const char* name, atUint32 hash)
        : name(name),
          hash(hash)
    {
    }

    virtual ~IPropertyFactory() {}

    virtual std::unique_ptr<IPropertyBase> read(Athena::io::IStreamReader& in)=0;
    virtual std::unique_ptr<IPropertyBase> read(Athena::io::YAMLDocReader& in)=0;
};

template <class T=IPropertyBase>
struct PropertyFactory : public IPropertyFactory
{
    PropertyFactory(const char* name, atUint32 hash)
        : IPropertyFactory(name, hash)
    {
    }

    virtual ~PropertyFactory() {}

    std::unique_ptr<IPropertyBase> read(Athena::io::IStreamReader& in)
    {
        std::unique_ptr<T> ret(new T(hash));
        ret->read(in);
        return std::move(ret);
    }

    std::unique_ptr<IPropertyBase> read(Athena::io::YAMLDocReader& in)
    {
        in.enterSubRecord(name);
        std::unique_ptr<T> ret(new T(hash));
        ret->read(in);
        in.leaveSubRecord();
        return std::move(ret);
    }
};

struct ResourcePropertyFactory : public IPropertyFactory
{
    FourCC type;
    ResourcePropertyFactory(const char* name, FourCC type, atUint32 hash)
        : IPropertyFactory(name, hash),
          type(type)
    {
    }

    virtual ~ResourcePropertyFactory() {}

    std::unique_ptr<IPropertyBase> read(Athena::io::IStreamReader& in)
    {
        std::unique_ptr<IPropertyBase> ret(new ResourceProperty(type, hash));
        ret->read(in);
        return std::move(ret);
    }

    std::unique_ptr<IPropertyBase> read(Athena::io::YAMLDocReader& in)
    {
        in.enterSubRecord(name);
        std::unique_ptr<IPropertyBase> ret(new ResourceProperty(type, hash));
        ret->read(in);
        in.leaveSubRecord();
        return std::move(ret);
    }
};
}
}

std::unordered_map<std::string, std::string> typeTranslation = {
    { "unknown",    "Retro::DNAMP2::UnknownProperty" },
    { "long",       "Retro::DNAMP2::IntProperty"     },
    { "float",      "Retro::DNAMP2::FloatProperty"   },
    { "vector3f",   "Retro::DNAMP2::Vector3Property" },
    { "color",      "Retro::DNAMP2::ColorProperty"   },
    { "bool",       "Retro::DNAMP2::BoolProperty"    },
    { "array",      "Retro::DNAMP2::ArrayProperty"   },
    { "string",     "Retro::DNAMP2::StringProperty"  },
    { "animparams", "Retro::DNAMP1::AnimationParameters"},
};

std::map<std::string, std::string> typeTranslationCase = {
    { "unknown",    "UnknownProperty" },
    { "long",       "IntProperty"     },
    { "float",      "FloatProperty"   },
    { "vector3f",   "Vector3Property" },
    { "color",      "ColorProperty"   },
    { "bool",       "BoolProperty"    },
    { "array",      "ArrayProperty"   },
    { "string",     "StringProperty"  },
    { "animparams", "AnimationParametersMP2"},
};

std::unordered_map<std::string, std::vector<atUint32>> typeIdMap;

int main(int argc, char *argv[])
{
    if (argc < 5)
    {
        fprintf(stderr, "Usage: propparse <in> <out> <project-path> <mp2|mp3>\n");
        return int(ParseError::InvalidArgs);
    }

    const char* in = argv[1];
    const char* out = argv[2];
    const char* projectPath = argv[3];
    // TODO: Use these
    bool isMP2 = !strcasecmp(argv[4], "mp2");
    bool isMP3 = !strcasecmp(argv[4], "mp3");
    if (!isMP2 && !isMP3)
    {
        fprintf(stderr, "Invalid game specified, only mp2 and mp3 are supported\n");
        return int(ParseError::InvalidGame);
    }

    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(in) == tinyxml2::XML_SUCCESS)
    {

        tinyxml2::XMLElement* root = doc.FirstChildElement("Properties");
        atUint32 version = strtol(root->Attribute("version"), nullptr, 0);
        if (version != EXPECTED_PROPERTY_VERSION)
        {
            fprintf(stderr, "Unsupported property version %i expected %i\n", version, EXPECTED_PROPERTY_VERSION);
            return int(ParseError::UnsupportedVersion);
        }

        FILE* src = fopen(out, "w");

        if (!src)
        {
            fprintf(stderr, "Unable to open or create target file \"%s\"\n", out);
            return int(ParseError::OutputFileNotFound);
        }

        fprintf(src, "/***************************************************************************\n"
                     " *                   Automatically generated by propparse                  *\n"
                     " *                              DO NOT MODIFY                              *\n"
                     " * propparse is part of PathShagged and is available under the MIT license *\n"
                     " * for more information go to: https://github.com/AxioDL/PathShagged       *\n"
                     " ***************************************************************************/\n"
                     "\n"
                     "#include \"%s/IProperty.hpp\"\n"
                     "namespace Retro\n"
                     "{\n"
                     "namespace DNAMP2\n"
                     "{\n"
                     "const std::map<atUint32, IPropertyFactoryBase*> PropertyDatabase =\n{\n",
                projectPath);

        if (root)
        {
            tinyxml2::XMLElement* child = root->FirstChildElement();
            while (child)
            {
                const char* elementType = child->Value();
                bool isProperty = !strcmp(elementType, "property");
                bool isStruct = (!isProperty && !strcmp(elementType, "struct"));
                atUint32 id = strtol(child->Attribute("ID"), nullptr, 0);
                std::string name(child->Attribute("name"));
                if (isProperty)
                {
                    std::string type(child->Attribute("type"));
                    if (typeTranslation.find(type) != typeTranslation.end())
                    {
                        typeIdMap[typeTranslationCase[type]].push_back(id);
                        fprintf(src, "    { 0x%.8X, new Retro::DNAMP2::IPropertyFactory<%s>(\"%s\", 0x%.8X)},\n", id, typeTranslation[type].c_str(), name.c_str(), id);
                    }
                    else if (!type.compare("file"))
                    {
                        if (isMP2)
                            typeIdMap["FileProperty"].push_back(id);
                        else if (isMP3)
                            typeIdMap["File64Property"].push_back(id);

                        std::string ext(child->Attribute("ext"));
                        fprintf(src, "    { 0x%.8X, new Retro::DNAMP2::ResourcePropertyFactory(\"%s\", FOURCC('%s'), 0x%.8X)},\n", id, name.c_str(), ext.c_str(), id);
                    }
                    else
                        fprintf(src, "    { 0x%.8X, new Retro::DNAMP2::IPropertyFactory<%s>(\"%s\", 0x%.8X)},\n", id, type.c_str(), name.c_str(), id);

                }
                else if (isStruct)
                {
                    fprintf(src, "    { 0x%.8X, new Retro::DNAMP2::IPropertyFactory<Retro::DNAMP2::StructProperty>(\"%s\", 0x%.8X)},\n", id, name.c_str(), id);
                    if (id == 'XFRM')
                        typeIdMap["Transform"].push_back(id);
                    else if (id == 0x8249F6C7)
                        typeIdMap["LayerParametersMP2"].push_back(id);
                    else
                        typeIdMap["StructProperty"].push_back(id);
                }
                child = child->NextSiblingElement();
            }
        }

        fprintf(src, "}; // PropertyDatabase \n"
                     "}\n"
                     "}\n");
        fclose(src);

        for (const auto& pair : typeIdMap)
        {
            for (const atUint32& id : pair.second)
                printf("case 0x%.8X:\n", id);
            if (!pair.first.compare("UnknownProperty"))
            {
                printf("    //UnknownProperty property;\n"
                       "    Printf(\"Unknown property type %%.8X @ %%.8X size %%i\\n\", hash, FTell() - 6, size);\n"
                       "    break;\n");
            }
            else
                printf("    %s property;\n"
                       "    break;\n", pair.first.c_str());
        }
    }
    else
    {
        fprintf(stderr, "Unable to open \"%s\\n", in);
        return int(ParseError::InputFileNotFound);
    }

    return int(ParseError::Success);
}
