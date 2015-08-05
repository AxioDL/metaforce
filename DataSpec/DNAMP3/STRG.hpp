#ifndef __DNAMP3_STRG_HPP__
#define __DNAMP3_STRG_HPP__

#include <unordered_map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/STRG.hpp"

namespace Retro
{
namespace DNAMP3
{

struct STRG : ISTRG, BigDNA
{
    DECL_EXPLICIT_DNA
    void _read(Athena::io::IStreamReader& reader);
    std::vector<std::pair<FourCC, std::vector<std::string>>> langs;
    std::unordered_map<FourCC, std::vector<std::string>*> langMap;
    std::map<std::string, int32_t> names;

    inline int32_t lookupIdx(const std::string& name) const
    {
        auto search = names.find(name);
        if (search == names.end())
            return -1;
        return search->second;
    }

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
            return search->second->at(idx);
        return std::string();
    }
    inline std::wstring getUTF16(const FourCC& lang, size_t idx) const
    {
        auto search = langMap.find(lang);
        if (search != langMap.end())
            return HECL::UTF8ToWide(search->second->at(idx));
        return std::wstring();
    }
    inline HECL::SystemString getSystemString(const FourCC& lang, size_t idx) const
    {
        auto search = langMap.find(lang);
        if (search != langMap.end())
#if HECL_UCS2
            return HECL::UTF8ToWide(search->second->at(idx));
#else
            return search->second->at(idx);
#endif
        return HECL::SystemString();
    }

    bool readAngelScript(const AngelScript::asIScriptModule& in);
    void writeAngelScript(std::ofstream& out) const;

    static bool Extract(const SpecBase& dataspec, PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        std::unique_ptr<ISTRG> strg = LoadSTRG(rs);
        std::ofstream strgOut(outPath.getAbsolutePath());
        strg->writeAngelScript(strgOut);
        return true;
    }

    static bool Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath)
    {
        STRG strg;
        HECL::Database::ASUniqueModule mod = HECL::Database::ASUniqueModule::CreateFromPath(inPath);
        if (!mod)
            return false;
        strg.readAngelScript(mod);
        Athena::io::FileWriter ws(outPath.getAbsolutePath());
        strg.write(ws);
        return true;
    }
};

}
}

#endif // __DNAMP2_STRG_HPP__
