#ifndef __DNAMP1_STRG_HPP__
#define __DNAMP1_STRG_HPP__

#include <unordered_map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/STRG.hpp"

namespace Retro
{
namespace DNAMP1
{

struct STRG : ISTRG, BigYAML
{
    DECL_EXPLICIT_DNA
    DECL_EXPLICIT_YAML
    void _read(Athena::io::IStreamReader& reader);
    std::vector<std::pair<FourCC, std::vector<std::wstring>>> langs;
    std::unordered_map<FourCC, std::vector<std::wstring>*> langMap;

    inline int32_t lookupIdx(const std::string& name) const {return -1;}

    inline size_t count() const
    {
        size_t retval = 0;
        for (const auto& item : langs)
        {
            size_t sz = item.second.size();
            if (sz > retval)
                retval = sz;
        }
        return retval;
    }
    inline std::string getUTF8(const FourCC& lang, size_t idx) const
    {
        auto search = langMap.find(lang);
        if (search != langMap.end())
            return HECL::WideToUTF8(search->second->at(idx));
        return std::string();
    }
    inline std::wstring getUTF16(const FourCC& lang, size_t idx) const
    {
        auto search = langMap.find(lang);
        if (search != langMap.end())
            return search->second->at(idx);
        return std::wstring();
    }
    inline HECL::SystemString getSystemString(const FourCC& lang, size_t idx) const
    {
        auto search = langMap.find(lang);
        if (search != langMap.end())
#if HECL_UCS2
            return search->second->at(idx);
#else
            return HECL::WideToUTF8(search->second->at(idx));
#endif
        return HECL::SystemString();
    }

    bool readAngelScript(const AngelScript::asIScriptModule& in);
    void writeAngelScript(std::ofstream& out) const;

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        STRG strg;
        strg.read(rs);
        FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("w"));
        strg.toYAMLFile(fp);
        fclose(fp);
        return true;
    }

    static bool Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath)
    {
        STRG strg;
        FILE* fp = HECL::Fopen(inPath.getAbsolutePath().c_str(), _S("r"));
        strg.fromYAMLFile(fp);
        fclose(fp);
        Athena::io::FileWriter ws(outPath.getAbsolutePath());
        strg.write(ws);
        return true;
    }

};

}
}

#endif // __DNAMP1_STRG_HPP__
