#ifndef _CWORLDTRANSMANAGER
#define _CWORLDTRANSMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"
#include "rstl/string.hpp"

class CAnimRes;
class CGuiTextSupport;
class CStringTable;
class CVector3f;

class CWorldTransManager {
public:
  enum ETransType { kTT_Disabled, kTT_Enabled, kTT_Text };

  CWorldTransManager();
  ~CWorldTransManager();

  void SetSfx(ushort, uchar, uchar);
  void SfxStart();
  void SfxStop();

  void EnableTransition(const CAnimRes&, const CAssetId, const CVector3f&, const CAssetId, const CVector3f&, bool);
  void EnableTransition(int fontId, int stringId, int stringIdx, bool fadeWhite, float chFadeTime, float chFadeRate, float textStartTime);
  void EnableTransition(int fontId, int stringId, int stringIdx, bool fadeWhite,
                        const rstl::string& audioFile, int volume, bool showSecondaryText,
                        float chFadeTime, float chFadeRate, float textStartTime, float textEndDelay,
                        float secondaryTextStartTime, float secondaryTextFadeDuration);
  void DisableTransition();
  void StartTransition();
  void EndTransition();
  void StartTextFadeOut();
  void Update(float dt);
  void Draw() const;
  void TouchModels();
  bool WaitForModelsAndTextures();
  bool IsTransitionFinished() const { return mTransitionFinished; }

  ETransType GetTransType() const { return mTransType; }

private:
  struct SModelDatas;

  static int GetSuitCharIdx();
  void UpdateDisabled(float dt);
  void UpdateEnabled(float dt);
  void UpdateText(float dt);
  void UpdateLights(float dt);
  void DrawAllModels() const;
  void DrawFirstPass() const;
  void DrawSecondPass() const;
  void DrawEnabled() const;
  void DrawDisabled() const;
  void DrawText() const;

  float mCurTime;
  rstl::single_ptr< SModelDatas > mModelData;
  rstl::single_ptr< CGuiTextSupport > mTextData;
#if VERSION >= VERSION_GM8P_00
  rstl::single_ptr< CGuiTextSupport > mSecondaryTextData;
#endif
  rstl::optional_object< TToken< CStringTable > > mStrTable;
  float mBgOffset;
  float mBgHeight;
  CRandom16 mRandom;
  ushort mSfx;
  CSfxHandle mSfxHandle;
  uchar mVolume;
  uchar mPanning;
  ETransType mTransType;
  float mStopTime;
  float mTextStartTime;
#if VERSION >= VERSION_GM8P_00
  float mTextEndDelay;
  float mSecondaryTextStartTime;
  float mSecondaryTextFadeDuration;
#endif
  float mSfxInterval;
#if VERSION >= VERSION_GM8P_00
  rstl::string mAudioFile;
#endif
  int mStrIdx;
  bool mTransitionFinished : 1;
  bool mStopSoon : 1;
  bool mGoingUp : 1;
  bool mFadeWhite : 1;
  bool mTextDirty : 1;
#if VERSION >= VERSION_GM8P_00
  bool mShowSecondaryText : 1;
#endif
};
CHECK_SIZEOF(CWorldTransManager,
             VERSION >= VERSION_R3IJ_00 ? 0x64 : (VERSION >= VERSION_GM8P_00 ? 0x68 : 0x48))

#endif // _CWORLDTRANSMANAGER
