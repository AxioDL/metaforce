#include "CMetroidModelInstance.hpp"
#include "CModel.hpp"

namespace urde
{

CMetroidModelInstance::CMetroidModelInstance
(const void* modelHeader, std::unique_ptr<CBooModel>&& inst)
: x0_visorFlags(hecl::SBig(*static_cast<const u32*>(modelHeader))), m_instance(std::move(inst))
{
    athena::io::MemoryReader r(static_cast<const u8*>(modelHeader) + 4, INT32_MAX);
    x4_xf.read34RowMajor(r);
    x34_aabb.readBoundingBoxBig(r);
}

CMetroidModelInstance::~CMetroidModelInstance() {}

}
