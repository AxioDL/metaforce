#ifndef __PSHAG_CANIMPOIDATA_HPP__
#define __PSHAG_CANIMPOIDATA_HPP__

#include "CFactoryMgr.hpp"
#include "CBoolPOINode.hpp"
#include "CInt32POINode.hpp"
#include "CParticlePOINode.hpp"
#include "CSoundPOINode.hpp"

namespace urde
{

class CAnimPOIData
{
    u32 x0_version;
    std::vector<CBoolPOINode> x4_boolNodes;
    std::vector<CInt32POINode> x14_int32Nodes;
    std::vector<CParticlePOINode> x24_particleNodes;
    std::vector<CSoundPOINode> x34_soundNodes;
public:
    CAnimPOIData(CInputStream& in);

    const std::vector<CBoolPOINode>& GetBoolPOIStream() const {return x4_boolNodes;}
    const std::vector<CInt32POINode>& GetInt32POIStream() const {return x14_int32Nodes;}
    const std::vector<CParticlePOINode>& GetParticlePOIStream() const {return x24_particleNodes;}
    const std::vector<CSoundPOINode>& GetSoundPOIStream() const {return x34_soundNodes;}
};

CFactoryFnReturn AnimPOIDataFactory(const SObjectTag& tag, CInputStream& in,
                                    const CVParamTransfer& parms);

}

#endif // __PSHAG_CANIMPOIDATA_HPP__
