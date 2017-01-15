#include "CScriptGenerator.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptGenerator::CScriptGenerator(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                   u32 w1, bool b1, const zeus::CVector3f& vec1, bool b2, bool active, float minScale, float maxScale)
: CEntity(uid, info, active, name),
  x34_(w1),
  x38_24_(b1),
  x38_25_(b2),
  x3c_(vec1),
  x48_minScale(minScale),
  x4c_maxScale(maxScale)
{
}

void CScriptGenerator::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    if (msg == EScriptObjectMessage::SetToZero && GetActive())
    {
    }

    CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}
}
