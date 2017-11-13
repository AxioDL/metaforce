#include "CAnimTreeTweenBase.hpp"

namespace urde
{

s32 CAnimTreeTweenBase::sAdvancementDepth = 0;

CAnimTreeTweenBase::CAnimTreeTweenBase(bool b1, const std::weak_ptr<CAnimTreeNode>& a,
                                       const std::weak_ptr<CAnimTreeNode>& b, int flags, std::string_view name)
: CAnimTreeDoubleChild(a, b, name), x1c_flags(flags), x20_31_b1(b1)
{
}

/*void CAnimTreeTweenBase::VGetTotalChildWeight(float t) const
{
}*/

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const {}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                             const CCharAnimTime& time) const
{
}

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const
{
    return (x14_a->VHasOffset(seg) || x18_b->VHasOffset(seg));
}

zeus::CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const
{
    const float weight = GetBlendingWeight();
    if (weight >= 1.0f)
        return x18_b->VGetOffset(seg);

    const zeus::CVector3f oA = x14_a->VGetOffset(seg);
    const zeus::CVector3f oB = x18_b->VGetOffset(seg);
    return zeus::CVector3f::lerp(oA, oB, weight);
}

zeus::CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const
{
    const float weight = GetBlendingWeight();
    if (weight >= 1.0f)
        return x18_b->VGetRotation(seg);

    const zeus::CQuaternion qA = x14_a->VGetRotation(seg);
    const zeus::CQuaternion qB = x18_b->VGetRotation(seg);
    return zeus::CQuaternion::slerp(qA, qB, weight);
}

std::pair<std::unique_ptr<IAnimReader>, bool> CAnimTreeTweenBase::VSimplified() { return {}; }
}
