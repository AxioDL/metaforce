#ifndef __URDE_IMETAANIM_HPP__
#define __URDE_IMETAANIM_HPP__

#include "../RetroTypes.hpp"
#include "CCharAnimTime.hpp"
#include <set>

namespace urde
{
class CAnimTreeNode;
class CAnimSysContext;
class CMetaAnimTreeBuildOrders;
class CPrimitive;
class IAnimReader;

enum class EMetaAnimType
{
    Primitive,
    Blend,
    PhaseBlend,
    Random,
    Sequence
};

class CPreAdvanceIndicator
{
    bool x0_isTime;
    CCharAnimTime x4_time;
    const char* xc_string;
public:
    CPreAdvanceIndicator(const CCharAnimTime& time)
    : x0_isTime(true), x4_time(time) {}
    CPreAdvanceIndicator(const char* string)
    : x0_isTime(false), xc_string(string) {}
    const char* GetString() const {return xc_string;}
    bool IsString() const {return !x0_isTime;}
    const CCharAnimTime& GetTime() const {return x4_time;}
    bool IsTime() const {return x0_isTime;}
};

class IMetaAnim
{
public:
    virtual ~IMetaAnim() = default;
    virtual std::shared_ptr<CAnimTreeNode> GetAnimationTree(const CAnimSysContext& animSys,
                                                            const CMetaAnimTreeBuildOrders& orders) const;
    virtual void GetUniquePrimitives(std::set<CPrimitive>& primsOut) const=0;
    virtual EMetaAnimType GetType() const=0;
    virtual std::shared_ptr<CAnimTreeNode> VGetAnimationTree(const CAnimSysContext& animSys,
                                                             const CMetaAnimTreeBuildOrders& orders) const=0;

    static void AdvanceAnim(IAnimReader& anim, const CCharAnimTime& dt);
    CCharAnimTime GetTime(const CPreAdvanceIndicator& ind, const IAnimReader& anim);
};

}

#endif // __URDE_IMETAANIM_HPP__
