#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "rstl/rc_ptr.hpp"
#include "rstl/auto_ptr.hpp"

CAnimTreeNode::CAnimTreeNode(const rstl::string& name) : mName(name) {}

rstl::ncrc_ptr< CAnimTreeNode > Cast(const rstl::ownership_transfer< IAnimReader >& ptr) {
  IAnimReader* reader = ptr.take_ownership();
  if (reader->IsCAnimTreeNode()) {
    return static_cast< CAnimTreeNode* >(reader);
  }

  rstl::auto_ptr< IAnimReader > invalidReader(reader);
  return rstl::ncrc_ptr< CAnimTreeNode >();
}

bool CAnimTreeNode::IsCAnimTreeNode() const { return true; }
