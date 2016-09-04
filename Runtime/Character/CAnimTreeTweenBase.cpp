#include "CAnimTreeTweenBase.hpp"

namespace urde
{

CAnimTreeTweenBase::CAnimTreeTweenBase(bool b1,
                                       const std::shared_ptr<CAnimTreeNode>& a,
                                       const std::shared_ptr<CAnimTreeNode>& b,
                                       int flags, const std::string& name)
: CAnimTreeDoubleChild(a, b, name), x1c_flags(flags), x20_31_b1(b1)
{
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const
{
}

void CAnimTreeTweenBase::VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut,
                                             const CCharAnimTime& time) const
{
}

bool CAnimTreeTweenBase::VHasOffset(const CSegId& seg) const
{
}

zeus::CVector3f CAnimTreeTweenBase::VGetOffset(const CSegId& seg) const
{
}

zeus::CQuaternion CAnimTreeTweenBase::VGetRotation(const CSegId& seg) const
{
}

std::shared_ptr<IAnimReader> CAnimTreeTweenBase::VSimplified()
{
}

}
