#ifndef __URDE_CANIMATIONDATABASEGAME_HPP__
#define __URDE_CANIMATIONDATABASEGAME_HPP__

#include "CAnimationDatabase.hpp"

namespace urde
{
class CAnimation;

class CAnimationDatabaseGame final : public CAnimationDatabase
{
    std::vector<std::shared_ptr<IMetaAnim>> x10_anims;
public:
    CAnimationDatabaseGame(const std::vector<CAnimation>& anims);
    const std::shared_ptr<IMetaAnim>& GetMetaAnim(u32 idx) const;
    u32 GetNumMetaAnims() const;
    const char* GetMetaAnimName(u32 idx) const;
    void GetAllUniquePrimitives(std::vector<CPrimitive>& primsOut) const;
    void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>& primsOut, std::string_view name) const;
};

}

#endif // __URDE_CANIMATIONDATABASEGAME_HPP__
