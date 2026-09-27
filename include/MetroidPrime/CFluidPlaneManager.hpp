#ifndef _CFLUIDPLANEMANAGER
#define _CFLUIDPLANEMANAGER

#include "types.h"

#include "MetroidPrime/CRippleManager.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

class CScriptWater;
class CStateManager;
class CVector3f;

#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "MetroidPrime/CFluidUVMotion.hpp"
#include "MetroidPrime/CRippleManager.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CScriptWater;
class CStateManager;
class CRipple;
class CTexture;
class CTransform4f;
class CRippleManager;

class CFluidPlaneCPURender {
public:
  enum ENormalMode { kNM_None, kNM_NoNormals, kNM_Normals, kNM_NBT };

  struct SHFieldSample {
    float height;
    signed char nx;
    signed char ny;
    signed char nz;
    unsigned char wavecapIntensity;
  };

  struct SPatchInfo {
    char mXSubdivs;
    char mYSubdivs;
    float mLocalMinX;
    float mLocalMinY;
    float mGlobalMinX;
    float mGlobalMinY;
    float mTileSize;
    float mRippleResolution;
    float mTileHypRadius;
    float mOoTileSize;
    float mOoRippleResolution;
    short mTileX;
    short mGridDimX;
    short mGridDimY;
    short mTileY;
    const char* mGridFlags;
    uchar mRedShift;
    uchar mGreenShift;
    uchar mBlueShift;
    uchar mNormalMode;
    float mWavecapIntensityScale;

    SPatchInfo(const CVector3f& localMin, const CVector3f& localMax, const CVector3f& pos,
               float rippleResolution, float tileSize, float wavecapIntensityScale,
               int numSubdivisionsInHField, int normalMode, int redShift, int greenShift,
               uchar blueShift, int tileX, int gridDimX, int gridDimY, int tileY,
               const char* gridFlags);
  };

  struct SRippleInfo {
    const CRipple* mRipple;
    int mFromX;
    int mToX;
    int mFromY;
    int mToY;
    int mGfromX;
    int mGtoX;
    int mGfromY;
    int mGtoY;

    SRippleInfo(const CRipple& ripple, int fromX, int toX, int fromY, int toY)
    : mRipple(&ripple), mGfromX(fromX), mGtoX(toX), mGfromY(fromY), mGtoY(toY) {}
  };

  static int numTilesInHField;
  static int numSubdivisionsInTile;
  static int numSubdivisionsInHField;
};

class CFluidPlane {
  static float kRippleIntensityRange;

public:
  enum EFluidType {
    kFT_NormalWater,
    kFT_PoisonWater,
    kFT_Lava,
    kFT_PhazonFluid,
    kFT_Four,
    kFT_ThickLava
  };

  CFluidPlane(const CAssetId texPattern1, const CAssetId texPattern2, CAssetId texColor,
              const float alpha, const EFluidType fluidType, const float rippleIntensity,
              const CFluidUVMotion& motion);
  virtual ~CFluidPlane();

  virtual void AddRipple(const float mag, const TUniqueId rippler, const CVector3f& center,
                         const CScriptWater& water, CStateManager& mgr);
  virtual void AddRipple(float intensity, TUniqueId rippler, const CVector3f& center,
                         const CVector3f& velocity, const CScriptWater& water, CStateManager& mgr,
                         const CVector3f& upVec);
  virtual void AddRipple(const CRipple& ripple, const CScriptWater& water, CStateManager& mgr);
  virtual void Render(const CStateManager& mgr, const CAABox&, const CFrustumPlanes&,
                      const CRippleManager&, const CVector3f&);
  virtual void Render(const CStateManager& mgr, float alpha, const CAABox& aabb,
                      const CTransform4f& xf, const CTransform4f& areaXf, bool noNormals,
                      const CFrustumPlanes& frustum,
                      const rstl::optional_object< CRippleManager >& rippleManager,
                      TUniqueId waterId, const char* gridFlags, int gridDimX, int gridDimY,
                      const CVector3f& areaCenter) const {}

  void Update();

  float CalculateRippleIntensity(const float base) const;
  float GetRippleScaleFromKineticEnergy(float baseI, float velDot);

  float GetAlpha() const { return mAlpha; }
  EFluidType GetFluidType() const { return mFluidType; }
  const CFluidUVMotion& GetUVMotion() const { return mUvMotion; }
  const TLockedToken< CTexture >& GetTexturePattern1() const { return *mTexPattern1; }
  bool HasTexturePattern1() const { return mTexPattern1; }
  const TLockedToken< CTexture >& GetTexturePattern2() const { return *mTexPattern2; }
  bool HasTexturePattern2() const { return mTexPattern2; }
  const TLockedToken< CTexture >& GetColorTexture() const { return *mTexColor; }
  bool HasColorTexture() const { return mTexColor; }
  static const float GetRippleIntensityRange() { return kRippleIntensityRange; }

protected:
  void RenderStripWithRipples(const CFluidPlaneCPURender::SHFieldSample& heights,
                              const unsigned char& flags, float curY, int startYDiv,
                              const CFluidPlaneCPURender::SPatchInfo& info);

  CAssetId mTexPattern1Id;
  CAssetId mTexPattern2Id;
  CAssetId mTexColorId;
  rstl::optional_object< TLockedToken< CTexture > > mTexPattern1;
  rstl::optional_object< TLockedToken< CTexture > > mTexPattern2;
  rstl::optional_object< TLockedToken< CTexture > > mTexColor;
  float mAlpha;
  EFluidType mFluidType;
  float mRippleIntensity;
  CFluidUVMotion mUvMotion;
};
CHECK_SIZEOF(CFluidPlane, 0xA0);

extern const bool gkWaterEnable;
extern const float gkFluidMaxCrest;
extern const bool gkWaterTurbulence;
extern const bool gkWaterBumpMapping;
extern const int gkWaterEnvMap;
extern const bool gkWaterFog;

class CFluidPlaneManager {
public:
  class CFluidProfile {
  public:
    void Clear();

  private:
    float x0_;
    float x4_;
    float x8_;
    float xc_;
    float x10_;
  };

  class CSplashRecord {
  public:
    CSplashRecord(TUniqueId id) : mTime(0.f), mId(id) {}
    void SetTime(float t) { mTime = t; }
    float GetTime() const { return mTime; }
    TUniqueId GetUniqueId() const { return mId; }

  private:
    float mTime;
    TUniqueId mId;
  };

  CFluidPlaneManager();

  void Update(float dt);
  void StartFrame(bool b) const;
  void EndFrame() const;

  void CreateSplash(TUniqueId splasher, CStateManager& mgr, const CScriptWater& water,
                    const CVector3f& pos, float factor, bool sfx);

  CRippleManager& RippleManager() { return mRippleManager; }
  const CRippleManager& GetRippleManager() const { return mRippleManager; }
  float GetLastSplashDeltaTime(TUniqueId uid) const;
  float GetLastRippleDeltaTime(TUniqueId uid) const;

  rstl::reserved_vector< CSplashRecord, 32 >& SplashRecords() { return mSplashes; }
  const rstl::reserved_vector< CSplashRecord, 32 >& GetSplashRecords() const {
    return mSplashes;
  }
  float GetTime() const { return mUvT; }
  void SetTime(float t) { mUvT = t; }

  float GetUVTime() const { return mUvT; }

  static uint GetFreqTableIndex(float);
  static void SetupRippleMap();
  static CFluidProfile sProfile;
  static const float kOldestTime;

private:
  CRippleManager mRippleManager;
  rstl::reserved_vector< CSplashRecord, 32 > mSplashes;
  float mUvT;
  mutable bool x120_;
  mutable bool x121_;
};
CHECK_SIZEOF(CFluidPlaneManager, 0x124);

#endif // _CFLUIDPLANEMANAGER
