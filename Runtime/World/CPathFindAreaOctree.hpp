#ifndef __URDE_CPATHFINDAREAOCTREE_HPP__
#define __URDE_CPATHFINDAREAOCTREE_HPP__

#include "rstl.hpp"
#include "CPathFindRegion.hpp"
#include "zeus/CVector3f.hpp"

namespace urde
{
class CPFArea;
class CPFAreaOctree
{
public:
    void Fixup(CPFArea&);
    void GetChildIndex(const zeus::CVector3f&) const;
    void GetRegionList(const zeus::CVector3f&) const;
    void GetRegionListList(rstl::reserved_vector<rstl::prereserved_vector<CPFRegion>, 32>, const zeus::CVector3f&, float);
    bool IsPointInPaddedAABox(const zeus::CVector3f&, float);
    void Render();
};
}

#endif // __URDE_CPATHFINDAREAOCTREE_HPP__