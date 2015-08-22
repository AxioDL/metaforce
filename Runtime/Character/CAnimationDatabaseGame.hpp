#ifndef __RETRO_CANIMATIONDATABASEGAME_HPP__
#define __RETRO_CANIMATIONDATABASEGAME_HPP__

#include "CAnimationDatabase.hpp"

namespace Retro
{

class CAnimationDatabaseGame : public CAnimationDatabase
{
public:
    const IMetaAnim* GetMetaAnim(u32) const;
    u32 GetNumMetaAnims() const;
    const char* GetMetaAnimName(u32) const;
    void GetAllUniquePrimitives(std::vector<CPrimitive>&) const;
    void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>&, const std::string&) const;
};

}

#endif // __RETRO_CANIMATIONDATABASEGAME_HPP__
