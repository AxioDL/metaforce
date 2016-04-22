#include "CMapWorldInfo.hpp"

namespace urde
{

void CMapWorldInfo::SetDoorVisited(TEditorId eid, bool visited)
{
    x14_[eid] = visited;
}

}
