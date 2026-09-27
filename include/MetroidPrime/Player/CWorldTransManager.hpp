#ifndef _CWORLDTRANSMANAGER
#define _CWORLDTRANSMANAGER

#include "Kyoto/SObjectTag.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/single_ptr.hpp"

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
  float mSfxInterval;
  int mStrIdx;
  bool mTransitionFinished : 1;
  bool mStopSoon : 1;
  bool mGoingUp : 1;
  bool mFadeWhite : 1;
  bool mTextDirty : 1;
};
CHECK_SIZEOF(CWorldTransManager, 0x48)

#endif // _CWORLDTRANSMANAGER
