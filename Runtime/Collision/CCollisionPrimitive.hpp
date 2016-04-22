#ifndef __URDE_CCOLLISIONPRIMITIVE_HPP__
#define __URDE_CCOLLISIONPRIMITIVE_HPP__

#include "Collision/CMaterialList.hpp"
namespace urde
{

class CCollisionPrimitive
{
    CMaterialList x8_material;
public:
    enum class Type
    {
    };

    static void InitBeginTypes();
    static void InitAddType(Type tp);
    static void InitEndTypes();
};

}

#endif // __URDE_CCOLLISIONPRIMITIVE_HPP__
