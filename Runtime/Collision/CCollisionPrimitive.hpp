#ifndef __URDE_CCOLLISIONPRIMITIVE_HPP__
#define __URDE_CCOLLISIONPRIMITIVE_HPP__

#include "Collision/CMaterialList.hpp"
#include <functional>

namespace urde
{

class CCollisionPrimitive
{
    CMaterialList x8_material;
public:
    class Type
    {
        std::function<void(u32)> x0_setter;
        const char* x4_info;
    public:
        Type() = default;
        Type(std::function<void(unsigned int)> setter, const char * info)
            : x0_setter(setter),
              x4_info(info)
        {
        }

        const char* GetInfo() const
        {
            return x4_info;
        }

        std::function<void(u32)> GetSetter() const
        {
            return x0_setter;
        }
    };

    static void InitBeginTypes();
    static void InitAddType(const Type& tp);
    static void InitEndTypes();

    static void InitBeginColliders();
    static void InitAddCollider(const Type& tp);
};

}

#endif // __URDE_CCOLLISIONPRIMITIVE_HPP__
