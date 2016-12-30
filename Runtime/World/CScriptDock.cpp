#include "CScriptDock.hpp"
#include "CActorParameters.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CWorld.hpp"
#include "CStateManager.hpp"

namespace urde
{
CMaterialList MakeDockMaterialList()
{
    CMaterialList list;
    list.Add(EMaterialTypes::Trigger);
    list.Add(EMaterialTypes::Immovable);
    list.Add(EMaterialTypes::AIBlock);
    return list;
}

CScriptDock::CScriptDock(TUniqueId uid, const std::string &name, const CEntityInfo &info,
                         const zeus::CVector3f position, const zeus::CVector3f& extents, s32 dock, TAreaId area,
                         bool active, s32 w1, bool b1)
    : CPhysicsActor(uid, active, name, info, zeus::CTransform(zeus::CMatrix3f::skIdentityMatrix3f, position),
        CModelData::CModelDataNull(), MakeDockMaterialList(), zeus::CAABox(-extents * 0.5f, extents * 0.5f),
                    SMoverData(1.f), CActorParameters::None(), 0.3f, 0.1f),
      x258_(w1),
      x25c_dock(dock),
      x260_area(area),
      x264_(3)
{
    x268_24_ = false;
    x268_25_ = b1;
    x268_26_ = false;
}

void CScriptDock::AreaLoaded(CStateManager & mgr)
{
    SetLoadConnected(mgr, x268_25_);
}

void CScriptDock::SetLoadConnected(CStateManager& mgr, bool loadOther)
{
    IGameArea::Dock* dock = mgr.GetWorld()->GetArea(x260_area)->DockNC(x25c_dock);
}
}
