#ifndef DNACOMMON_OBBTREEBUILDER_HPP
#define DNACOMMON_OBBTREEBUILDER_HPP

#include "DNACommon.hpp"

namespace DataSpec
{

struct OBBTreeBuilder
{
    using ColMesh = hecl::blender::ColMesh;
    template <typename Node>
    static std::unique_ptr<Node> buildCol(const ColMesh& mesh);
};

}

#endif // DNACOMMON_OBBTREEBUILDER_HPP
