#ifndef __RETRO_CANIMATIONDATABASE_HPP__
#define __RETRO_CANIMATIONDATABASE_HPP__

#include "../RetroTypes.hpp"
#include <vector>
#include <set>
#include <string>

namespace Retro
{
class IMetaAnim;
class CPrimitive;

class CAnimationDatabase
{
public:
    virtual const IMetaAnim* GetMetaAnim(u32) const=0;
    virtual u32 GetNumMetaAnims() const=0;
    virtual const char* GetMetaAnimName(u32) const=0;
    virtual void GetAllUniquePrimitives(std::vector<CPrimitive>&) const=0;
    virtual void GetUniquePrimitivesFromMetaAnim(std::set<CPrimitive>&, const std::string&) const=0;
};

}

#endif // __RETRO_CANIMATIONDATABASE_HPP__
