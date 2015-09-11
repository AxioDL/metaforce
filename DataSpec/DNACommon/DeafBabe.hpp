#ifndef _DNACOMMON_DEAFBABE_HPP_
#define _DNACOMMON_DEAFBABE_HPP_

#include "DNACommon.hpp"

namespace Retro
{
namespace DNACommon
{

struct DeafBabe : BigDNA
{
    Delete expl;
    std::vector<atUint64> materials;
    std::vector<atUint8> vertMats;
    std::vector<atUint8> edgeMats;
    std::vector<atUint8> polyMats;
    std::vector<std::pair<atUint16, atUint16>> edgeVertConnections;
    std::vector<std::pair<atUint16, atUint16>> triangleEdgeConnections;
    void read(Athena::io::IStreamReader& reader)
    {
    }
    void write(Athena::io::IStreamWriter& writer) const
    {
    }
};

}
}

#endif // _DNACOMMON_DEAFBABE_HPP_
