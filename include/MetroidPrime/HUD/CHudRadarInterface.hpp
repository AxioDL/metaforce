#ifndef _CHUDRADARINTERFACE
#define _CHUDRADARINTERFACE

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/TToken.hpp"
#include "types.h"

class CGuiFrame;
class CGuiWidget;
class CGuiModel;
class CGuiTextPane;
class CGuiCamera;
class CAuiEnergyBarT01;
class CAuiMeter;
class CStateManager;

class CTexture;
class CHudRadarInterface {
public:
  CHudRadarInterface(CGuiFrame& frame, const CStateManager& mgr);
  void SetIsVisibleGame(bool visible);
  void Update(float dt, const CStateManager& mgr);
  void Draw(const CStateManager& mgr, float alpha) const;

private:
  enum ERadarMode { kRM_Three = 3, kRM_Four = 4 };
  static bool IsWidgetVisibleMode(ERadarMode mode) {
    return mode == kRM_Three || mode == kRM_Four;
  }

  struct SRadarPaintDrawParms {
    SRadarPaintDrawParms(const CVector3f& playerPos, const CTransform4f& preTranslate,
                        const CTransform4f& postTranslate, float scopeRadius, float scopeScalar,
                        float alpha, float xyRadius, float zRadius, float zCloseRadius)
    : mPlayerPos(playerPos)
    , mPreTranslate(preTranslate)
    , mPostTranslate(postTranslate)
    , mScopeRadius(scopeRadius)
    , mScopeScalar(scopeScalar)
    , mAlpha(alpha)
    , mXyRadius(xyRadius)
    , mZRadius(zRadius)
    , mZCloseRadius(zCloseRadius) {}

    CVector3f mPlayerPos;
    CTransform4f mPreTranslate;
    CTransform4f mPostTranslate;
    float mScopeRadius;
    float mScopeScalar;
    float mAlpha;
    float mXyRadius;
    float mZRadius;
    float mZCloseRadius;
    uchar mPad[4];
  };

  void DoDrawRadarPaint(float radius) const;
  void DrawRadarPaint(const CVector3f& enemyPos, float radius, float alpha,
                     const SRadarPaintDrawParms& parms) const;

  TCachedToken< CTexture > mTxtrRadarPaint;
  CTransform4f mRadarStuffXf;
  bool mVisibleGame : 1;
  bool mVisibleDebug : 1;
  CGuiWidget* mBaseWidget_RadarStuff;
  CGuiCamera* mCamera;
};
CHECK_SIZEOF(CHudRadarInterface, 0x48)

#endif // _CHUDRADARINTERFACE
