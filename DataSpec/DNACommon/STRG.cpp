#include "STRG.hpp"
#include "../DNAMP1/STRG.hpp"
#include "../DNAMP2/STRG.hpp"
#include "../DNAMP3/STRG.hpp"

namespace DataSpec
{

void ISTRG::gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
{
    /* TODO: parse out resource tokens */
}

std::unique_ptr<ISTRG> LoadSTRG(athena::io::IStreamReader& reader)
{
    uint32_t magic = reader.readUint32Big();
    if (magic != 0x87654321)
    {
        LogDNACommon.report(logvisor::Error, "invalid STRG magic");
        return std::unique_ptr<ISTRG>();
    }

    uint32_t version = reader.readUint32Big();
    switch (version)
    {
    case 0:
    {
        DNAMP1::STRG* newStrg = new DNAMP1::STRG;
        newStrg->_read(reader);
        return std::unique_ptr<ISTRG>(newStrg);
    }
    case 1:
    {
        DNAMP2::STRG* newStrg = new DNAMP2::STRG;
        newStrg->_read(reader);
        return std::unique_ptr<ISTRG>(newStrg);
    }
    case 3:
    {
        DNAMP3::STRG* newStrg = new DNAMP3::STRG;
        newStrg->_read(reader);
        return std::unique_ptr<ISTRG>(newStrg);
    }
    }
    return std::unique_ptr<ISTRG>();
}
}
