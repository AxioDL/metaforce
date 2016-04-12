#include "CAnimData.hpp"
#include "CCharacterInfo.hpp"
#include "CCharLayoutInfo.hpp"
#include "Graphics/CSkinnedModel.hpp"
#include "CCharacterFactory.hpp"
#include "CAnimationManager.hpp"
#include "CTransitionManager.hpp"

namespace urde
{

CAnimData::CAnimData(TResId id, const CCharacterInfo& character, int a, int b, bool c,
                     const TLockedToken<CCharLayoutInfo>& layout,
                     const TToken<CSkinnedModel>& model,
                     const std::weak_ptr<CAnimSysContext>& ctx,
                     const std::shared_ptr<CAnimationManager>& animMgr,
                     const std::shared_ptr<CTransitionManager>& transMgr,
                     const TLockedToken<CCharacterFactory>& charFactory)
: x0_charFactory(charFactory),
  xc_charInfo(character),
  xcc_layoutData(layout),
  xd8_modelData(model),
  xfc_animCtx(ctx.lock()),
  x100_animMgr(animMgr),
  x1d8_selfId(id),
  x1fc_transMgr(transMgr),
  x204_b(b),
  x208_a(a),
  x21c_25_c(c),
  x220_pose(layout->GetSegIdList().GetList().size()),
  x2f8_poseBuilder(layout)
{
}

}
