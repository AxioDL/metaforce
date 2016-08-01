#include "CMappableObject.hpp"
#include "GameGlobalObjects.hpp"
#include "ITweakAutoMapper.hpp"

namespace urde
{
const zeus::CVector3f CMappableObject::skDoorVerts[8] = {};

zeus::CTransform CMappableObject::AdjustTransformForType()
{
    /* TODO - Phil: Finish this */
    float doorCenterX = g_tweakAutoMapper->GetDoorCenter().x;
    float doorCenterY = g_tweakAutoMapper->GetDoorCenter().y;
    if (x0_ == EMappableObjectType::BigDoor1)
    {
        zeus::CTransform scale;
        scale.scaleBy(1.5);
        zeus::CTransform orientation;
        orientation.origin = {-1.4f*doorCenterX, 0.0f, 0.0f};
        zeus::CTransform tmp3 = x10_ * orientation;
        orientation.rotateLocalZ(zeus::degToRad(90.0f));
        return  tmp3 * scale;
    }
    else if (x0_ == EMappableObjectType::BigDoor2)
    {

    }
    else if (x0_ == EMappableObjectType::IceDoorCeiling || x0_ == EMappableObjectType::WaveDoorCeiling
             || x0_ == EMappableObjectType::Eleven)
    {
    }
    else if (x0_ == EMappableObjectType::IceDoorCeiling || x0_ == EMappableObjectType::WaveDoorFloor
             || x0_ == EMappableObjectType::Twelve)
    {
    }
    else if (EMappableObjectType(u32(x0_) - u32(EMappableObjectType::IceDoorFloor2)) <= EMappableObjectType::ShieldDoor
             || x0_ == EMappableObjectType::Fifteen)
    {
    }
    return x10_;
}

void CMappableObject::PostConstruct(const void *)
{
#if __BYTE_ORDER__!= __ORDER_BIG_ENDIAN__
    x0_ = EMappableObjectType(SBIG(u32(x0_)));
    x4_ = SBIG(x4_);
    x8_ = SBIG(x8_);
    xc_ = SBIG(xc_);
    for (u32 i = 0 ; i<3 ; i++)
    {
        for (u32 j = 0 ; j<4 ; j++)
        {
            u32* tmp = reinterpret_cast<u32*>(&x10_.basis.m[i][j]);
            *tmp = SBIG(*tmp);
        }
    }

#endif
    x10_.origin.x = x10_.basis.m[0][3];
    x10_.origin.y = x10_.basis.m[1][3];
    x10_.origin.z = x10_.basis.m[2][3];
    x10_.basis.transpose();
    x10_ = AdjustTransformForType();
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
