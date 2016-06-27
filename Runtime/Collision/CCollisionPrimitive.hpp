#ifndef __URDE_CCOLLISIONPRIMITIVE_HPP__
#define __URDE_CCOLLISIONPRIMITIVE_HPP__

#include "Collision/CMaterialList.hpp"
#include "CRayCastResult.hpp"
#include "zeus/CAABox.hpp"

#include <functional>

namespace urde
{

class COBBTree;
class CInternalCollisionStructure;
class CCollisionInfo;
class CCollisionInfoList;
class CInternalRayCastStructure;
class CMaterialFilter;
class CCollisionPrimitive
{
public:
    class Type;
private:

    CMaterialList x8_material;
    static std::unique_ptr<std::vector<Type>> sCollisionTypeList;
    static bool sTypesAdding;
public:
    class Type
    {
        std::function<void(u32)> x0_setter;
        const char* x4_info;
    public:
        Type() = default;
        Type(const std::function<void(u32)>& setter, const char * info);

        const char* GetInfo() const;

        std::function<void(u32)> GetSetter() const;
    };

    class Comparison
    {
        std::function<bool(const CInternalCollisionStructure&, CCollisionInfoList&)> x0_collider;
        const char* x4_type1;
        const char* x8_type2;
    public:
        Comparison(const std::function<bool(const CInternalCollisionStructure&, CCollisionInfoList&)>& collider,
                   const char* type1, const char* type2)
            : x0_collider(collider),
              x4_type1(type1),
              x8_type2(type2)
        {
        }

        std::function<bool(const CInternalCollisionStructure&, CCollisionInfoList&)> GetCollider() const { return x0_collider; }
        const char* GetType1() const { return x4_type1; }
        const char* GetType2() const { return x8_type2; }
    };

    class MovingComparison
    {
        std::function<bool(const CInternalCollisionStructure&, const zeus::CVector2f&, double&,
                           CCollisionInfo&)> x0_collider;
        const char* x4_type1;
        const char* x8_type2;
    public:
        MovingComparison(const std::function<bool(const CInternalCollisionStructure&, const zeus::CVector2f&, double&,
                                                  CCollisionInfo&)>& collider, const char* type1, const char* type2)
            : x0_collider(collider),
              x4_type1(type1),
              x8_type2(type2)
        {
        }

        std::function<bool(const CInternalCollisionStructure&, const zeus::CVector2f&, double&,
                           CCollisionInfo&)> GetCollider() const { return x0_collider; }
        const char* GetType1() const { return x4_type1; }
        const char* GetType2() const { return x8_type2; }
    };

    class BooleanComparison
    {
        std::function<bool(const CInternalCollisionStructure&)> x0_collider;
        const char* x4_type1;
        const char* x8_type2;

    public:
        BooleanComparison(const std::function<bool(const CInternalCollisionStructure&)>& collider, const char* type1,
                          const char* type2)
            : x0_collider(collider),
              x4_type1(type1),
              x8_type2(type2)
        {
        }

        std::function<bool(const CInternalCollisionStructure&)> GetCollider() const { return x0_collider; }
        const char* GetType1() const { return x4_type1; }
        const char* GetType2() const { return x8_type2; }
    };

    CCollisionPrimitive()=default;
    CCollisionPrimitive(const CMaterialList& list);
    virtual u32 GetTableIndex() const=0;
    virtual void SetMaterial(const CMaterialList&);
    virtual const CMaterialList& GetMaterial() const;
    virtual zeus::CAABox CalculateAABox(const zeus::CTransform&) const=0;
    virtual zeus::CAABox CalculateLocalAABox() const=0;
    virtual FourCC GetPrimType() const=0;
    virtual ~CCollisionPrimitive() {}
    virtual CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const=0;
    CRayCastResult CastRay(const zeus::CVector3f&, const zeus::CVector3f&, float, const CMaterialFilter&,
                           const zeus::CTransform&) const;


    static void InitBeginTypes();
    static void InitAddType(const Type& tp);
    static void InitEndTypes();

    static void InitBeginColliders();
    static void InitAddBooleanCollider(const BooleanComparison& cmp);
    static void InitAddMovingCollider(const MovingComparison& cmp);
    static void InitAddCollider(const Comparison& cmp);
    static void InitEndColliders();
};

}

#endif // __URDE_CCOLLISIONPRIMITIVE_HPP__
