#pragma once

#include "IAnimReader.hpp"

namespace urde {

class CAnimTreeNode : public IAnimReader {
protected:
  std::string x4_name;

public:
  CAnimTreeNode(std::string_view name) : x4_name(name) {}
  bool IsCAnimTreeNode() const override { return true; }
  static std::shared_ptr<CAnimTreeNode> Cast(std::unique_ptr<IAnimReader>&& ptr) {
    if (ptr->IsCAnimTreeNode())
      return std::static_pointer_cast<CAnimTreeNode>(std::shared_ptr<IAnimReader>(std::move(ptr)));
    return {};
  }

  virtual u32 Depth() const = 0;
  virtual CAnimTreeEffectiveContribution VGetContributionOfHighestInfluence() const = 0;
  virtual u32 VGetNumChildren() const = 0;
  virtual std::shared_ptr<IAnimReader> VGetBestUnblendedChild() const = 0;
  virtual void VGetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out,
                                   float w) const = 0;
  void GetWeightedReaders(rstl::reserved_vector<std::pair<float, std::weak_ptr<IAnimReader>>, 16>& out, float w) const {
    VGetWeightedReaders(out, w);
  }

  CAnimTreeEffectiveContribution GetContributionOfHighestInfluence() const;
  u32 GetNumChildren() const;
  std::shared_ptr<IAnimReader> GetBestUnblendedChild() const;

  std::string_view GetName() const { return x4_name; }
};

} // namespace urde
