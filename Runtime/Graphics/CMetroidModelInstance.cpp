#include "CMetroidModelInstance.hpp"
#include "CModel.hpp"

namespace urde
{

#if 0
CMetroidModelInstance::CMetroidModelInstance(CBooModel* inst)
: x0_visorFlags(0), m_instance(inst)
{
    x34_aabb = inst->x20_aabb;
}

CMetroidModelInstance::CMetroidModelInstance
(const void* modelHeader, CBooModel* inst)
: x0_visorFlags(hecl::SBig(*static_cast<const u32*>(modelHeader))), m_instance(inst)
{
    athena::io::MemoryReader r(static_cast<const u8*>(modelHeader) + 4, INT32_MAX);
    x4_xf.read34RowMajor(r);
    x34_aabb.readBoundingBoxBig(r);
}

CMetroidModelInstance::~CMetroidModelInstance() {}
#endif

}
