#ifndef __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__
#define __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__

#include "IOStreams.hpp"
#include "CToken.hpp"

namespace Retro
{
class CSimplePool;
class CGenDescription;

class CSpawnSystemKeyframeData
{
public:
    class CSpawnSystemKeyframeInfo
    {
        friend class CSpawnSystemKeyframeData;
        u32 x0_id;
        u32 x4;
        u32 x8;
        u32 xc;
        TToken<CGenDescription> x10_token;
        bool x18_found = false;
        void LoadToken(CSimplePool* pool);
    public:
        CSpawnSystemKeyframeInfo(CInputStream& in);
    };
private:
    u32 x0;
    u32 x4;
    u32 x8;
    u32 xc;
    std::vector<std::pair<u32, std::vector<CSpawnSystemKeyframeInfo>>> x10_spawns;
public:
    CSpawnSystemKeyframeData(CInputStream& in);
    void LoadAllSpawnedSystemTokens(CSimplePool* pool);
};

}

#endif // __RETRO_CSPAWNSYSTEMKEYFRAMEDATA_HPP__
