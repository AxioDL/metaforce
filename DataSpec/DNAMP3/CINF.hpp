#ifndef _DNAMP3_CINF_HPP_
#define _DNAMP3_CINF_HPP_

#include "BlenderConnection.hpp"
#include "../DNACommon/DNACommon.hpp"
#include "../DNAMP2/CINF.hpp"

namespace DataSpec
{
namespace DNAMP3
{

struct CINF : DNAMP2::CINF
{
    Delete expl;
    void sendCINFToBlender(hecl::BlenderConnection::PyOutStream& os, const UniqueID64& cinfId) const;
    static std::string GetCINFArmatureName(const UniqueID64& cinfId);
};

}
}

#endif // _DNAMP3_CINF_HPP_
