#ifndef _CANIMTREENODE
#define _CANIMTREENODE

#include "Kyoto/Animation/CCharAnimMemoryMetrics.hpp"
#include "Kyoto/Animation/IAnimReader.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/string.hpp"

class CAnimTreeNode : public IAnimReader {
public:
  virtual uint Depth() const = 0;
  CAnimTreeNode(const rstl::string&);
  ~CAnimTreeNode() override {
    CCharAnimMemoryMetrics::SubtractFromTotalSize(mName.size(), CCharAnimMemoryMetrics::kASS_Two);
  }

  virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const = 0;

  virtual uint VGetNumChildren() const = 0;
  virtual rstl::rc_ptr< CAnimTreeNode > VGetBestUnblendedChild() const = 0;
  virtual void VGetWeightedReaders(
      float w, rstl::reserved_vector< rstl::pair< float, IAnimReader* >, 16 >& out) const = 0;


  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const {
    return VGetContributionOfHighestInfluence();
  }

  rstl::rc_ptr< CAnimTreeNode > GetBestUnblendedChild() const {
    return VGetBestUnblendedChild();
  }

  const rstl::string& GetPrimitiveName() const { return mName; }

  bool IsCAnimTreeNode() const override;

protected:
  rstl::string mName;
};
CHECK_SIZEOF(CAnimTreeNode, 0x14)

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr);

#endif // _CANIMTREENODE
