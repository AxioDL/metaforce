#include "CMappableObject.hpp"
#include "ITweakAutoMapper.hpp"

namespace urde
{
const zeus::CVector3f CMappableObject::skDoorVerts[8] = {};

void CMappableObject::ReadAutoMapperTweaks(const ITweakAutoMapper& tweaks)
{
    const zeus::CVector3f& center = tweaks.GetDoorCenter();
    /* Ugly hack, but necessary */
    zeus::CVector3f* doorVerts = (zeus::CVector3f*)CMappableObject::skDoorVerts;
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
