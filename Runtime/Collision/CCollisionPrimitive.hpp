#ifndef __URDE_CCOLLISIONPRIMITIVE_HPP__
#define __URDE_CCOLLISIONPRIMITIVE_HPP__

namespace urde
{

class CCollisionPrimitive
{
public:
    enum class Type
    {
    };

    static void InitBeginTypes();
    static void InitAddType(Type tp);
    static void InitEndTypes();

    static void InitBeginColliders();
    static void InitAddCollider(Type tp);
    static void InitEndTypes();
};

}

#endif // __URDE_CCOLLISIONPRIMITIVE_HPP__
