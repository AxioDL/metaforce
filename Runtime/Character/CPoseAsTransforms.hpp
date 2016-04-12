#ifndef __PSHAG_CPOSEASTRANSFORMS_HPP__
#define __PSHAG_CPOSEASTRANSFORMS_HPP__

#include "RetroTypes.hpp"
#include "CSegId.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CPoseAsTransforms
{
    CSegId x0_nextId = 0;
    CSegId x1_count;
    std::pair<CSegId, CSegId> x8_links[100];
    std::unique_ptr<zeus::CTransform[]> xd0_transformArr;
    CSegId xd4_lastInserted = 0;
public:
    CPoseAsTransforms(u8 boneCount);
    bool ContainsDataFor(const CSegId& id) const;
    void Clear();
    void AccumulateScaledTransform(const CSegId& id, zeus::CMatrix3f& rotation, float scale) const;
    const zeus::CTransform& GetTransform(const CSegId& id) const;
    const zeus::CVector3f& GetOffset(const CSegId& id) const;
    const zeus::CMatrix3f& GetRotation(const CSegId& id) const;
    void Insert(const CSegId& id, const zeus::CMatrix3f& rotation, const zeus::CVector3f& offset);
};

}

#endif // __PSHAG_CPOSEASTRANSFORMS_HPP__
