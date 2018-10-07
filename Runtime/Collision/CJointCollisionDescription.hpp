#pragma once

#include "Character/CSegId.hpp"
#include "zeus/CAABox.hpp"

namespace urde
{
class CJointCollisionDescription
{
public:
    enum class ECollisionType
    {
        Sphere,
        SphereSubdivide,
        AABox,
        OBBAutoSize,
        OBB,
    };

    enum class EOrientationType
    {
        Zero,
    };

private:
    ECollisionType x0_colType;
    EOrientationType x4_orientType;
    CSegId x8_;
    CSegId x9_;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    float x24_;
    float x28_;
    std::string x2c_name;
    TUniqueId x3c_actorId = kInvalidUniqueId;
    float x40_;

public:
    CJointCollisionDescription(ECollisionType, CSegId, CSegId, const zeus::CVector3f&, const zeus::CVector3f&, float,
                               float, EOrientationType, std::string_view, float);
    std::string_view GetName() const { return x2c_name; }
    TUniqueId GetCollisionActorId() const { return x3c_actorId; }
    static CJointCollisionDescription SphereSubdivideCollision(CSegId, CSegId, float, float,
                                                               EOrientationType, std::string_view, float);
    static CJointCollisionDescription SphereCollision(CSegId, float, std::string_view, float);
    static CJointCollisionDescription AABoxCollision(CSegId, zeus::CVector3f const&, std::string_view, float);
    static CJointCollisionDescription OBBAutoSizeCollision(CSegId, CSegId, const zeus::CVector3f&, EOrientationType,
                                                           std::string_view, float);
    static CJointCollisionDescription OBBCollision(CSegId, const zeus::CVector3f&, const zeus::CVector3f&,
                                                   std::string_view, float);
    zeus::CVector3f GetPivotPoint() const;
    float GetRadius() const;
    void SetCollisionActorId(TUniqueId);
    EOrientationType GetOrientationType() const { return x4_orientType; }
    float GetMass() const;
    zeus::CAABox GetBounds() const;
    void GetType() const;
    void GetNextId() const;
    void GetPivotId() const;
    void ScaleAllBounds(const zeus::CVector3f&);
    void GetTween() const;
};
}

