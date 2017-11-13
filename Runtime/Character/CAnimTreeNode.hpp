#ifndef __URDE_CANIMTREENODE_HPP__
#define __URDE_CANIMTREENODE_HPP__

#include "IAnimReader.hpp"

namespace urde
{

class CAnimTreeNode : public IAnimReader
{
protected:
    std::string x4_name;
public:
    CAnimTreeNode(std::string_view name) : x4_name(name) {}
    bool IsCAnimTreeNode() const {return true;}

    virtual u32 Depth() const=0;
    virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const=0;
    virtual u32 VGetNumChildren() const=0;
    virtual std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const=0;
    virtual void VGetWeightedReaders(std::vector<std::pair<float, std::weak_ptr<IAnimReader>>>& out, float w) const=0;

    CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const;
    u32 GetNumChildren() const;
    std::shared_ptr<IAnimReader> GetBestUnblendedChild() const;
};

}

#endif // __URDE_CANIMTREENODE_HPP__
