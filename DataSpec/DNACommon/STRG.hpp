#ifndef __COMMON_STRG_HPP__
#define __COMMON_STRG_HPP__

#include <string>
#include <HECL/HECL.hpp>
#include "DNACommon.hpp"
#include "../Logging.hpp"

namespace Retro
{
struct ISTRG
{
    virtual size_t count() const=0;
    virtual std::string getUTF8(const FourCC& lang, size_t idx) const=0;
    virtual std::wstring getUTF16(const FourCC& lang, size_t idx) const=0;
    virtual HECL::SystemString getSystemString(const FourCC& lang, size_t idx) const=0;
    virtual int32_t lookupIdx(const std::string& name) const=0;
};
std::unique_ptr<ISTRG> LoadSTRG(Athena::io::IStreamReader& reader);
}

#endif // __COMMON_STRG_HPP__
