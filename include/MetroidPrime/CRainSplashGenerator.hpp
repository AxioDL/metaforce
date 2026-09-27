#ifndef _CRAINSPLASHGENERATOR
#define _CRAINSPLASHGENERATOR

#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"
#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CVector3f;
class CStateManager;
class CTransform4f;
class SSkinningWorkspace;

class CRainSplashGenerator {
  struct SSplashLine {
    static const float skInitialSpeed;
    static const float skInitialHeight;
    static const uchar skInitialWidth;

    float mT;
    float mXEnd;
    float mYEnd;
    float mSpeed;
    float mZParabolaHeight;
    uchar mLineWidth;
    uchar mLength;
    bool mActive : 1;

    SSplashLine()
    : mT(0.0f)
    , mXEnd(0.0f)
    , mYEnd(0.0f)
    , mSpeed(skInitialSpeed)
    , mZParabolaHeight(skInitialHeight)
    , mLineWidth(skInitialWidth)
    , mLength(1)
    , mActive(true) {}

    void Update(float dt, CStateManager& mgr);
    void Draw(float alpha, float dt, const CVector3f& pos) const;
    void SetActive();
  };

  struct SRainSplash {
    rstl::reserved_vector< SSplashLine, 4 > mLines;
    CVector3f mPos;
    float x70_;

    SRainSplash();

    void Update(float dt, CStateManager& mgr);
    const bool IsActive() const;
    void Draw(float alpha, float dt, const CVector3f& pos) const;
    void SetPoint(const CVector3f& pos);
  };

public:
  CRainSplashGenerator(const CVector3f& scale, int maxSplashes, int genRate, float minZ,
                       float alpha);
  ~CRainSplashGenerator() {}

  bool IsRaining() const { return mRaining; }

  void Update(float dt, CStateManager& mgr);
  void Draw(const CTransform4f& xf) const;
  void GeneratePoints(const CVector3f* vertices, const CVector3f* normals, int count);

private:
  rstl::vector< SRainSplash > mRainSplashes;
  CRandom16 mRandom;
  CVector3f mScale;
  float mGenerateTimer;
  float mGenerateInterval;
  float mDt;
  float mMinZ;
  float mAlpha;
  int mCurPoint;
  int mQueueTail;
  int mQueueHead;
  int mQueueSize;
  int mGenRate;
  bool x48_24 : 1;
  bool mRaining : 1;

  void UpdateRainSplashRange(CStateManager& mgr, int start, int end, float dt);
  void UpdateRainSplashes(CStateManager& mgr, float magnitude, float dt);
  void AddPoint(const CVector3f& pos);
  void DoDraw(const CTransform4f& xf) const;

  static int GetNextBestPt(int, const CVector3f*, const CVector3f*, int, CRandom16&, float);
};
CHECK_SIZEOF(CRainSplashGenerator, 0x4c)

#endif // _CRAINSPLASHGENERATOR
