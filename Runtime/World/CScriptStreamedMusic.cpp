#include "CScriptStreamedMusic.hpp"
#include "CStringExtras.hpp"
#include "TCastTo.hpp"

namespace urde
{

bool CScriptStreamedMusic::ValidateFileName(const std::string& fileName)
{
    if (!CStringExtras::CompareCaseInsensitive(fileName, "sw"))
        return true;
    if (CStringExtras::IndexOfSubstring(fileName, ".dsp") != -1)
        return true;
    return false;
}

CScriptStreamedMusic::CScriptStreamedMusic(TUniqueId id, const CEntityInfo& info, const std::string& name,
                                           bool active, const std::string& fileName, bool b1, float f1, float f2,
                                           u32 w1, bool b2, bool b3)
: CEntity(id, info, active, name), x34_fileName(fileName), x44_b1(b1),
  x45_fileNameValid(ValidateFileName(fileName)), x46_b2(b2), x47_b3(b3),
  x48_f1(f1), x4c_f2(f2), x50_w1(w1) {}

void CScriptStreamedMusic::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptStreamedMusic::PreThink(float, CStateManager&)
{

}

void CScriptStreamedMusic::Think(float, CStateManager&)
{

}

void CScriptStreamedMusic::AcceptScriptMsg(EScriptObjectMessage msg,
                                           TUniqueId objId, CStateManager& stateMgr)
{

}

}
