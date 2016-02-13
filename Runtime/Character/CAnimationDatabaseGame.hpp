#ifndef __PSHAG_CANIMATIONDATABASEGAME_HPP__
#define __PSHAG_CANIMATIONDATABASEGAME_HPP__

#include "CAnimationDatabase.hpp"

namespace pshag
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

#endif // __PSHAG_CANIMATIONDATABASEGAME_HPP__
