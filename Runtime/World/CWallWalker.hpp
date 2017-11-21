#ifndef __URDE_CWALLWALKER_HPP__
#define __URDE_CWALLWALKER_HPP__

#include "CPatterned.hpp"

namespace urde
{
class CWallWalker : public CPatterned
{
public:
    CWallWalker(ECharacter, TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&,
                CModelData&&, const CPatternedInfo&, EMovementType, EColliderType, EBodyType,
                const CActorParameters&, s32, bool);
};
}
#endif // __URDE_CWALLWALKER_HPP__
