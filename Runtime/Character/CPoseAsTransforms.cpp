#include "CPoseAsTransforms.hpp"

namespace urde
{

CPoseAsTransforms::CPoseAsTransforms(u8 boneCount)
: x1_count(boneCount), xd0_transformArr(new zeus::CTransform[boneCount])
{}

bool CPoseAsTransforms::ContainsDataFor(const CSegId& id) const
{
    const std::pair<CSegId, CSegId>& link = x8_links[id];
    if (link.first != 0xff || link.second != 0xff)
        return true;
    return false;
}

void CPoseAsTransforms::Clear()
{
    for (u32 i=0 ; i<100 ; ++i)
        x8_links[i] = std::make_pair(CSegId(), CSegId());
    xd4_lastInserted = 0;
    x0_nextId = 0;
}

void CPoseAsTransforms::AccumulateScaledTransform(const CSegId& id,
                                                  zeus::CMatrix3f& rotation,
                                                  float scale) const
{
    rotation.addScaledMatrix(GetRotation(id), scale);
}

const zeus::CTransform& CPoseAsTransforms::GetTransform(const CSegId& id) const
{
    const std::pair<CSegId, CSegId>& link = x8_links[id];
    return xd0_transformArr[link.second];
}

const zeus::CVector3f& CPoseAsTransforms::GetOffset(const CSegId& id) const
{
    const std::pair<CSegId, CSegId>& link = x8_links[id];
    return xd0_transformArr[link.second].origin;
}

const zeus::CMatrix3f& CPoseAsTransforms::GetRotation(const CSegId& id) const
{
    const std::pair<CSegId, CSegId>& link = x8_links[id];
    return xd0_transformArr[link.second].basis;
}

void CPoseAsTransforms::Insert(const CSegId& id,
                               const zeus::CMatrix3f& rotation,
                               const zeus::CVector3f& offset)
{
    xd0_transformArr[x0_nextId] = zeus::CTransform(rotation, offset);
    std::pair<CSegId, CSegId>& link = x8_links[id];
    link.first = xd4_lastInserted;
    link.second = x0_nextId;
    xd4_lastInserted = id;
    ++x0_nextId;
}

}
