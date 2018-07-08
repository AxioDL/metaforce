#include "CWallWalker.hpp"

namespace urde
{
CWallWalker::CWallWalker(ECharacter chr, TUniqueId uid, std::string_view name, EFlavorType flavType,
                         const CEntityInfo& eInfo, const zeus::CTransform& xf,
                         CModelData&& mData, const CPatternedInfo& pInfo, EMovementType mType,
                         EColliderType colType, EBodyType bType, const CActorParameters& aParms, s32 w1, u32 w2)
    : CPatterned(chr, uid, name, flavType, eInfo, xf, std::move(mData), pInfo, mType, colType, bType, aParms, w1)
{}
}
