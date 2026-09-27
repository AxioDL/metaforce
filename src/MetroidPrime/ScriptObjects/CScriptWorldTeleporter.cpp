#include "MetroidPrime/ScriptObjects/CScriptWorldTeleporter.hpp"

#include "MetroidPrime/CAnimRes.hpp"
#include "MetroidPrime/CMain.hpp"
#include "MetroidPrime/CMainFlow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CWorldTransManager.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CResFactory.hpp"

CScriptWorldTeleporter::CScriptWorldTeleporter(const TUniqueId uid, const rstl::string& name,
                                               const CEntityInfo& info, const bool active,
                                               const CAssetId worldId, const CAssetId areaId)
: CEntity(uid, info, active, name)
, mWorldId(worldId)
, mAreaId(areaId)
, mType(kTT_NoTransition)
, mUpElevator(false)
, mInTransition(false)
, mFadeWhite(false)
, mCharFadeIn(0.1f)
, mCharsPerSecond(8.0f)
, mShowDelay(0.0f)
, mPlayerAnim(kInvalidAssetId, -1, 0)
, mPlayerScale(CVector3f::Zero())
, mPlatformModel(kInvalidAssetId)
, mPlatformScale(CVector3f::Zero())
, mBackgroundModel(kInvalidAssetId)
, mBackgroundScale(CVector3f::Zero())
, mSoundId(CSfxManager::kInternalInvalidSfxId)
, mVolume(0)
, mPanning(0) {}

CScriptWorldTeleporter::CScriptWorldTeleporter(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const bool active,
    CAssetId worldId, const CAssetId areaId, CAssetId playerAncs, uint charIdx, uint defaultAnim,
    const CVector3f& playerScale, const CAssetId platformModel, const CVector3f& platformScale,
    const CAssetId backgroundModel, const CVector3f& backgroundScale, const bool upElevator,
    const ushort soundId, const uchar volume, const uchar panning)
: CEntity(uid, info, active, name)
, mWorldId(worldId)
, mAreaId(areaId)
, mType(kTT_Elevator)
, mUpElevator(upElevator)
, mInTransition(false)
, mFadeWhite(false)
, mCharFadeIn(0.1f)
, mCharsPerSecond(8.0f)
, mShowDelay(0.0f)
, mPlayerAnim(playerAncs, charIdx, defaultAnim)
, mPlayerScale(playerScale)
, mPlatformModel(platformModel)
, mPlatformScale(platformScale)
, mBackgroundModel(backgroundModel)
, mBackgroundScale(backgroundScale)
, mSoundId(CSfxManager::TranslateSFXID(soundId))
, mVolume(volume)
, mPanning(panning) {}

CScriptWorldTeleporter::CScriptWorldTeleporter(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const bool active,
    const CAssetId worldId, const CAssetId areaId, const int soundId, const uchar volume,
    const uchar panning, CAssetId fontId, const CAssetId stringId, const bool fadeWhite,
    const float charFadeIn, const float charsPerSecond, const float showDelay)
: CEntity(uid, info, active, name)
, mWorldId(worldId)
, mAreaId(areaId)
, mType(kTT_Text)
, mUpElevator(false)
, mInTransition(false)
, mFadeWhite(fadeWhite)
, mCharFadeIn(charFadeIn)
, mCharsPerSecond(charsPerSecond)
, mShowDelay(showDelay)
, mPlayerAnim(kInvalidAssetId, -1, 0)
, mPlayerScale(CVector3f::Zero())
, mPlatformModel(kInvalidAssetId)
, mPlatformScale(CVector3f::Zero())
, mBackgroundModel(kInvalidAssetId)
, mBackgroundScale(CVector3f::Zero())
, mSoundId(CSfxManager::TranslateSFXID(soundId))
, mVolume(volume)
, mPanning(panning)
, mFontId(fontId)
, mStringId(stringId) {}

CScriptWorldTeleporter::~CScriptWorldTeleporter() {}

ENTITY_ACCEPT_IMPL(CScriptWorldTeleporter)

void CScriptWorldTeleporter::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                             CStateManager& mgr) {
  if (GetActive()) {
    CWorldTransManager* transMgr = mgr.WorldTransManager();
    switch (msg) {
    case kSM_SetToZero: {
      CWorld* world = mgr.World();
      world->SetLoadPauseState(true);
      CAssetId currentWorld = gpGameState->CurrentWorldAssetId();
      gpGameState->SetCurrentWorldId(mWorldId);

      if (gpResourceFactory->GetResLoader().GetResourceTypeById(mWorldId) == 'MLVL') {
        StartTransition(mgr);
        gpGameState->SetCurrentWorldId(mWorldId);
        gpGameState->CurrentWorldState().SetDesiredAreaAssetId(mAreaId);
        gpMain->SetRestartMode(CMain::kRM_None);
        mgr.QuitGame();
      } else {
        mInTransition = false;
        transMgr->DisableTransition();
        gpGameState->SetCurrentWorldId(currentWorld);
      }
      break;
    }
    case kSM_Play:
      StartTransition(mgr);
      transMgr->SetSfx(mSoundId, mVolume, mPanning);
      transMgr->SfxStart();
      break;

    case kSM_Stop:
      mInTransition = false;
      transMgr->DisableTransition();
      transMgr->SfxStop();
      break;

    default:
      break;
    }
  }
  CEntity::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptWorldTeleporter::StartTransition(CStateManager& mgr) {
  if (mInTransition) {
    return;
  }

  CWorldTransManager* transMgr = mgr.WorldTransManager();

  if (mType == kTT_Elevator && mPlayerAnim.GetACSFile() != kInvalidAssetId &&
      mPlayerAnim.GetCharacter() != u32(-1)) {
    CAnimRes animRes(mPlayerAnim.GetACSFile(), mPlayerAnim.GetInitialAnimation(),
                     mPlayerScale, mPlayerAnim.GetCharacter(), true);
    transMgr->EnableTransition(animRes, mPlatformModel, mPlatformScale, mBackgroundModel,
                               mBackgroundScale, mUpElevator);
    mInTransition = true;

  } else if (mType == kTT_Text) {
    transMgr->EnableTransition(mFontId, mStringId, 0, GetFadeWhite(), mCharFadeIn,
                               mCharsPerSecond, mShowDelay);
    mInTransition = true;

  } else {
    transMgr->DisableTransition();
  }
}
