#ifndef __DNAMP2_STRG_HPP__
#define __DNAMP2_STRG_HPP__

#include <unordered_map>
#include "../DNACommon/PAK.hpp"
#include "../DNACommon/STRG.hpp"

namespace Retro
{
namespace DNAMP2
{

struct STRG : ISTRG
{
    DECL_YAML
    Delete expl;
    void _read(Athena::io::IStreamReader& reader);
    std::vector<std::pair<FourCC, std::vector<std::wstring>>> langs;
    std::unordered_map<FourCC, std::vector<std::wstring>*> langMap;
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

    static bool Extract(PAKEntryReadStream& rs, const HECL::ProjectPath& outPath)
    {
        STRG strg;
        strg.read(rs);
        FILE* fp = HECL::Fopen(outPath.getAbsolutePath().c_str(), _S("wb"));
        strg.toYAMLFile(fp);
        fclose(fp);
        return true;
    }

    static bool Cook(const HECL::ProjectPath& inPath, const HECL::ProjectPath& outPath)
    {
        STRG strg;
        FILE* fp = HECL::Fopen(inPath.getAbsolutePath().c_str(), _S("rb"));
        strg.fromYAMLFile(fp);
        fclose(fp);
        Athena::io::FileWriter ws(outPath.getAbsolutePath());
        strg.write(ws);
        return true;
    }
};

}
}

#endif // __DNAMP2_STRG_HPP__
