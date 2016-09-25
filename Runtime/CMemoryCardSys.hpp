#ifndef __URDE_CMEMORYCARDSYS_HPP__
#define __URDE_CMEMORYCARDSYS_HPP__

#include "CToken.hpp"
#include "World/CWorld.hpp"
#include "CGameHintInfo.hpp"
#include <vector>

namespace urde
{
class CDummyWorld;
class CSaveWorld;

class CSaveWorldMemory
{
    ResId x0_mlvlId = -1;
    ResId x4_unk = -1;
    ResId x8_savwId;
    std::vector<u32> xc_;
    std::vector<u32> x1c_;
    std::unique_ptr<CDummyWorld> x2c_dummyWorld;
    TLockedToken<CSaveWorld> x3c_saveWorld; /* Used to be auto_ptr */

public:
    ResId GetWorldAssetId() const { return x0_mlvlId; }
    ResId GetSaveWorldAssetId() const { return x8_savwId; }
    CSaveWorldMemory(ResId mlvl, ResId savw);
};

/* TODO: figure out */
class CMemorySomethingElse
{
    u32 x0_;
    u32 x4_;
    u32 x8_;
    std::vector<u32> xc_;
    std::vector<u32> x1c_;
    std::unique_ptr<u32> x2c_;
    std::unique_ptr<u32> x34_;
};

class CMemoryCardSys
{
    TLockedToken<CGameHintInfo> x0_hints;
    std::vector<CSaveWorldMemory> xc_memoryWorlds;
    std::vector<CMemorySomethingElse> x1c_; /* used to be auto_ptr of vector */
public:
    const std::vector<CGameHintInfo::CGameHint>& GetHints() const { return x0_hints->GetHints(); }
    const std::vector<CSaveWorldMemory>& GetMemoryWorlds() const { return xc_memoryWorlds; }
    CMemoryCardSys();
};

}

#endif // __URDE_CMEMORYCARDSYS_HPP__
