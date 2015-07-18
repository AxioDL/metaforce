#ifndef __DNAMP1_STRG_HPP__
#define __DNAMP1_STRG_HPP__

#include <unordered_map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/STRG.hpp"

namespace Retro
{
namespace DNAMP1
{

struct STRG : ISTRG, BigDNA
{
    DECL_EXPLICIT_DNA
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
        return std::string();
    }

    bool readAngelScript(const AngelScript::asIScriptModule& in);
    void writeAngelScript(std::ofstream& out) const;

};

}
}

#endif // __DNAMP1_STRG_HPP__
