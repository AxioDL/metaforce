#ifndef __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__
#define __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__

#include "IOStreams.hpp"

namespace Retro
{
class CSimplePool;

class CSpawnSystemKeyframeData
{
public:
    CSpawnSystemKeyframeData(CInputStream& in);
    void LoadAllSpawnedSystemTokens(CSimplePool* pool);
};

}

#endif // __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__
