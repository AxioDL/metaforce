#include "CMappableObject.hpp"
#include "GameGlobalObjects.hpp"

namespace urde
{
const zeus::CVector3f CMappableObject::skDoorVerts[8] = {};

zeus::CTransform CMappableObject::AdjustTransformForType()
{
    const float doorCenterX = g_tweakAutoMapper->GetDoorCenter().x;
    const float doorCenterZ = g_tweakAutoMapper->GetDoorCenter().z;
    if (x0_type == EMappableObjectType::BigDoor1)
    {
        zeus::CTransform orientation;
        orientation.origin = {-1.4f*doorCenterX, 0.0f, 0.0f};
        orientation.rotateLocalZ(zeus::degToRad(90.0f));
        return  (x10_transform * orientation) * zeus::CTransform::Scale(zeus::CVector3f{1.5f});
    }
    else if (x0_type == EMappableObjectType::BigDoor2)
    {
        zeus::CTransform orientation;
        orientation.origin = {0.f, -2.0f * doorCenterZ, -1.4f * doorCenterX};
        orientation.rotateLocalZ(zeus::degToRad(-90.f));
        return (x10_transform * orientation) * zeus::CTransform::Scale(zeus::CVector3f{1.5f});
    }
    else if (x0_type == EMappableObjectType::IceDoorCeiling || x0_type == EMappableObjectType::WaveDoorCeiling
             || x0_type == EMappableObjectType::PlasmaDoorCeiling)
    {
        zeus::CTransform orientation;
        orientation.origin = {-1.65f * doorCenterX, 0.f, -1.5f * doorCenterZ};
        orientation.rotateLocalY(zeus::degToRad(90.f));
        return x10_transform * orientation;
    }
    else if (x0_type == EMappableObjectType::IceDoorFloor || x0_type == EMappableObjectType::WaveDoorFloor
             || x0_type == EMappableObjectType::PlasmaDoorFloor)
    {
        zeus::CTransform orientation;
        orientation.origin = {-1.65f * doorCenterX, 0.f, -1.f * doorCenterZ};
        orientation.rotateLocalY(zeus::degToRad(90.f));
        return x10_transform * orientation;
    }
    else if ((u32(x0_type) - u32(EMappableObjectType::IceDoorFloor2)) <= u32(EMappableObjectType::ShieldDoor)
             || x0_type == EMappableObjectType::Fifteen)
    {
        zeus::CTransform orientation;
        orientation.origin = {-0.49f * doorCenterX, 0.f, -1.f * doorCenterZ};
        orientation.rotateLocalY(zeus::degToRad(90.f));
        return x10_transform * orientation;
    }
    else if (x0_type >= EMappableObjectType::BlueDoor || x0_type <= EMappableObjectType::Fifteen)
    {
        zeus::CMatrix4f tmp = x10_transform.toMatrix4f().transposed();
        return zeus::CTransform::Translate(tmp.m[1][0], tmp.m[2][1], tmp[3][2]);
    }
    return x10_transform;
}

void CMappableObject::PostConstruct(const void *)
{
#if __BYTE_ORDER__!= __ORDER_BIG_ENDIAN__
    x0_type = EMappableObjectType(hecl::SBig(u32(x0_type)));
    x4_ = hecl::SBig(x4_);
    x8_.id = hecl::SBig(x8_.id);
    xc_ = hecl::SBig(xc_);
    for (u32 i = 0 ; i<3 ; i++)
    {
        for (u32 j = 0 ; j<4 ; j++)
        {
            u32* tmp = reinterpret_cast<u32*>(&x10_transform.basis.m[i][j]);
            *tmp = hecl::SBig(*tmp);
        }
    }

#endif
    x10_transform.origin.x = x10_transform.basis.m[0][3];
    x10_transform.origin.y = x10_transform.basis.m[1][3];
    x10_transform.origin.z = x10_transform.basis.m[2][3];
    x10_transform.basis.transpose();
    x10_transform = AdjustTransformForType();
}

zeus::CVector3f CMappableObject::BuildSurfaceCenterPoint(s32) const
{
    return {};
}

void CMappableObject::ReadAutoMapperTweaks(const ITweakAutoMapper& tweaks)
{
    const zeus::CVector3f& center = tweaks.GetDoorCenter();
    zeus::CVector3f* doorVerts = const_cast<zeus::CVector3f*>(&CMappableObject::skDoorVerts[0]);
    /* Wrap door verts around -Z to build surface */
    doorVerts[0].assign(      -center.z, -center.y, 0.f);
    doorVerts[1].assign(      -center.z, -center.y, 2.f * center.x);
    doorVerts[2].assign(      -center.z,  center.y, 0.f);
    doorVerts[3].assign(      -center.z,  center.y, 2.f * center.x);
    doorVerts[4].assign(.2f * -center.z, -center.y, 0.f);
    doorVerts[5].assign(.2f * -center.z, -center.y, 2.f * center.x);
    doorVerts[6].assign(.2f * -center.z,  center.y, 0.f);
    doorVerts[7].assign(.2f * -center.z,  center.y, 2.f * center.x);
}
}
