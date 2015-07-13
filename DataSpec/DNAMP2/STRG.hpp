#ifndef __DNAMP2_STRG_HPP__
#define __DNAMP2_STRG_HPP__

#include <unordered_map>
#include "../DNACommon/DNACommon.hpp"
#include "../DNACommon/STRG.hpp"

namespace Retro
{
namespace DNAMP2
{

struct STRG : ISTRG, BigDNA
{
    DECL_EXPLICIT_DNA
    void _read(Athena::io::IStreamReader& reader);
    std::unordered_map<FourCC, std::vector<std::wstring>> langs;
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
        for (auto item : langs)
        {
            size_t sz = item.second.size();
            if (sz > retval)
                retval = sz;
        }
        return retval;
    }
    inline std::string getUTF8(const FourCC& lang, size_t idx) const
    {
        auto search = langs.find(lang);
        if (search != langs.end())
            return HECL::WideToUTF8(search->second.at(idx));
        return std::string();
    }
    inline std::wstring getUTF16(const FourCC& lang, size_t idx) const
    {
        auto search = langs.find(lang);
        if (search != langs.end())
            return search->second.at(idx);
        return std::wstring();
    }
    inline HECL::SystemString getSystemString(const FourCC& lang, size_t idx) const
    {
        auto search = langs.find(lang);
        if (search != langs.end())
#if HECL_UCS2
            return search->second.at(idx);
#else
            return HECL::WideToUTF8(search->second.at(idx));
#endif
        return std::string();
    }
};

}
}

#endif // __DNAMP2_STRG_HPP__
