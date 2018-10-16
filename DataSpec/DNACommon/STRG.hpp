#pragma once

#include <string>
#include <fstream>
#include <hecl/hecl.hpp>
#include <hecl/Database.hpp>
#include <athena/FileWriter.hpp>
#include "DNACommon.hpp"

namespace DataSpec
{
struct ISTRG : BigDNAVYaml
{
    virtual ~ISTRG() = default;

    virtual size_t count() const=0;
    virtual std::string getUTF8(const FourCC& lang, size_t idx) const=0;
    virtual std::u16string getUTF16(const FourCC& lang, size_t idx) const=0;
    virtual hecl::SystemString getSystemString(const FourCC& lang, size_t idx) const=0;
    virtual int32_t lookupIdx(std::string_view name) const=0;

    virtual void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const;
};
std::unique_ptr<ISTRG> LoadSTRG(athena::io::IStreamReader& reader);

}

