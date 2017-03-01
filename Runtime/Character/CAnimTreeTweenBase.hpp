#ifndef __URDE_CANIMTREETWEENBASE_HPP__
#define __URDE_CANIMTREETWEENBASE_HPP__

#include "CAnimTreeDoubleChild.hpp"

namespace urde
{

class CAnimTreeTweenBase : public CAnimTreeDoubleChild
{
    static s32 sAdvancementDepth;
protected:
    int x1c_flags;
    bool x20_31_b1;
    bool x20_30_b2 = false;
public:
    CAnimTreeTweenBase(bool,
                       const std::weak_ptr<CAnimTreeNode>& a,
                       const std::weak_ptr<CAnimTreeNode>& b,
                       int, const std::string& name);

    virtual void SetBlendingWeight(float w)=0;
    virtual float VGetBlendingWeight() const=0;

    float GetBlendingWeight() const { return VGetBlendingWeight(); }

    //void VGetTotalChildWeight(float) const;
    float VGetRightChildWeight() const { return GetBlendingWeight(); }

    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut) const;
    void VGetSegStatementSet(const CSegIdList& list, CSegStatementSet& setOut, const CCharAnimTime& time) const;
    bool VHasOffset(const CSegId& seg) const;
    zeus::CVector3f VGetOffset(const CSegId& seg) const;
    zeus::CQuaternion VGetRotation(const CSegId& seg) const;

    std::pair<std::unique_ptr<IAnimReader>, bool> VSimplified();
    bool ShouldCullTree() const { return false; }

    static void IncAdvancementDepth() { sAdvancementDepth++; }
    static void DecAdvancementDepth() { sAdvancementDepth--; }
};

}

#endif // __URDE_CANIMTREETWEENBASE_HPP__
