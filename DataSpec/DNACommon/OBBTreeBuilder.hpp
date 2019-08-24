#pragma once

#include <memory>

#include <hecl/Blender/Connection.hpp>

namespace DataSpec {

struct OBBTreeBuilder {
  using ColMesh = hecl::blender::ColMesh;
  template <typename Node>
  static std::unique_ptr<Node> buildCol(const ColMesh& mesh);
};

} // namespace DataSpec
