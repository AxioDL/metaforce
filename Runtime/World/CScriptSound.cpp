#include "CScriptSound.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptSound::CScriptSound(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                           const zeus::CTransform& xf, s16 soundId, bool active, float, float, float,
                           u32, u32, u32, u32, u32, bool, bool, bool, bool, bool, bool, bool, bool, u32)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(),
         CMaterialList(0), CActorParameters::None(), kInvalidUniqueId)
{
}

}
