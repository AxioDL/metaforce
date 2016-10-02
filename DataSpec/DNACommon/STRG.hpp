#ifndef __COMMON_STRG_HPP__
#define __COMMON_STRG_HPP__

#include <string>
#include <fstream>
#include <hecl/hecl.hpp>
#include <hecl/Database.hpp>
#include <athena/FileWriter.hpp>
#include "DNACommon.hpp"

namespace DataSpec
{
struct ISTRG : BigYAML
{
    virtual ~ISTRG() {}

    virtual size_t count() const=0;
    virtual std::string getUTF8(const FourCC& lang, size_t idx) const=0;
    virtual std::wstring getUTF16(const FourCC& lang, size_t idx) const=0;
    virtual hecl::SystemString getSystemString(const FourCC& lang, size_t idx) const=0;
    virtual int32_t lookupIdx(const std::string& name) const=0;

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const;
};
std::unique_ptr<ISTRG> LoadSTRG(athena::io::IStreamReader& reader);

}

#endif // __COMMON_STRG_HPP__
