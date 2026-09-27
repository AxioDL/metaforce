#ifndef _CELEMENTGEN
#define _CELEMENTGEN

#include "types.h"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Particles/CParticleGen.hpp"
#include "Kyoto/TToken.hpp"

class CGenDescription;
class CModVectorElement;

#pragma cpp_extensions on
class CElementGen : public CParticleGen {
public:
  struct CParticleListItem {
    ushort mPartIdx;
    CVector3f mViewPoint;

    explicit CParticleListItem(short partIdx, const CVector3f& viewPoint)
    : mPartIdx(partIdx), mViewPoint(viewPoint) {}
  };

  struct CTexturedParticleListItem {
    ushort mTexMapIdx;
    ushort mPartIdx;
    CVector3f mViewPoint;

    explicit CTexturedParticleListItem(short texMapIdx, short partIdx, const CVector3f& viewPoint)
    : mTexMapIdx(texMapIdx), mPartIdx(partIdx), mViewPoint(viewPoint) {}
  };

  enum EModelOrientationType {
    kMOT_Normal,
    kMOT_One,
  };
  enum EOptionalSystemFlags {
    kOSF_None,
    kOSF_One,
    kOSF_Two,
  };
  enum LightType {
    kLT_None = 0,
    kLT_Custom = 1,
    kLT_Directional = 2,
    kLT_Spot = 3,
  };
  struct CParticle {
    int mEndFrame;
    CVector3f mPos;
    CVector3f mPrevPos;
    CVector3f mVel;
    int mStartFrame;
    float mLineLengthOrSize;
    float mLineWidthOrRota;
    CColor mColor;

    CParticle()
    : mPos(CVector3f::Zero())
    , mPrevPos(mPos)
    , mVel(mPrevPos)
    , mColor(static_cast< u8 >(0xFF), 0x00, 0xFF, 0xFF) {}
  };
  struct CAdvancedValues {
    float values[8];
  };

  CElementGen();
  CElementGen(TToken< CGenDescription >, EModelOrientationType = kMOT_Normal,
              EOptionalSystemFlags = kOSF_One);
  ~CElementGen() override;

  virtual const bool Update(double) override;
  virtual void Render() override;
  virtual void SetOrientation(const CTransform4f& orientation) override;
  virtual void SetTranslation(const CVector3f& translation) override;
  virtual void SetGlobalOrientation(const CTransform4f& orientation) override;
  virtual void SetGlobalTranslation(const CVector3f& translation) override;
  virtual void SetGlobalScale(const CVector3f& scale) override;
  virtual void SetLocalScale(const CVector3f& scale) override;
  virtual void SetParticleEmission(const bool emission) override;
  virtual void SetModulationColor(const CColor& col) override;
  virtual void SetGeneratorRate(float rate) override;
  virtual const CTransform4f& GetOrientation() const override { return mOrientation; }
  virtual const CVector3f& GetTranslation() const override { return mTranslation; }
  virtual const CTransform4f& GetGlobalOrientation() const override;
  virtual const CVector3f& GetGlobalTranslation() const override;
  virtual const CVector3f& GetGlobalScale() const override { return mGlobalScale; }
  virtual float GetGeneratorRate() const override;
  virtual bool GetParticleEmission() const override;
  virtual const CColor& GetModulationColor() const override;
  virtual bool IsSystemDeletable() const override;
  virtual rstl::optional_object< CAABox > GetBounds() const override;
  virtual int GetParticleCount() const override { return mActiveParticleCount; }
  virtual bool SystemHasLight() const override;
  virtual CLight GetLight() const override;
  virtual void DestroyParticles() override;
  virtual uint Get4CharId() const override;
  int GetMaxParticles() const { return mMAXP; }
  void SetZTest(bool enabled) { mZTest = enabled; }
  rstl::vector< CParticle >& Particles() { return mParticles; }
  const rstl::vector< CParticle >& GetParticles() const { return mParticles; }
  int GetEmitterTime() const;
  int GetSystemCount();
  void EndLifetime();
  void ForceParticleCreation(int amount);

  bool InternalUpdate(double dt);
  void UpdateLightParameters();
  void UpdateAdvanceAccessParameters(int, int);
  bool UpdateVelocitySource(int, int, CParticle&);
  void UpdateExistingParticles();
  void CreateNewParticles(int);
  void UpdatePSTranslationAndOrientation();
  CElementGen* ConstructChildParticleSystem(TToken< CGenDescription >) const;
  void UpdateChildParticleSystems(double);
  void RenderModels();
  void RenderLines();
  void RenderParticlesIndirectTexture();
  static void RenderParticlesFlameThrower(CElementGen* const* gens, int count);
  void RenderParticles();
  void RenderBasicParticlesRotNoTS(const CTransform4f&);
  void RenderBasicParticlesNoRotNoTS(const CTransform4f&);
  void RenderBasicParticlesRotTS(const CTransform4f&);
  void RenderBasicParticlesNoRotTS(const CTransform4f&);
  int GetParticleCountAll() const;
  int GetParticleCountAllInternal() const;
  void AccumulateBounds(const CVector3f&, float);
  void BuildParticleSystemBounds();

  int GetNumActiveChildParticles() const;
  CParticleGen* GetActiveChildParticle(int index) const;
  int GetCumulativeParticleCount() const { return mCumulativeParticles; }
  bool IsIndirectTextured()
      const; // { return x28_loadedGenDesc->x54_x40_TEXR && x28_loadedGenDesc->x58_x44_TIND; }
  void SetExternalVar(int index, float val);
  float GetExternalVar(int index) const;

  static void Initialize();
  static void ShutDown();

  void SetGlobalOrientAndTrans(const CTransform4f& xf);
  void SetLeaveLightsEnabledForModelRender(bool b) { mModelsUseLights = b; }

  static void SetSubtractBlend(bool subtract) { sSubtractBlend = subtract; }
  static void SetMoveRedToAlphaBuffer(const bool move) { sMoveRedToAlphaBuffer = move; }

  static void SetGlobalSeed(const ushort seed) { sSeed = seed; }

private:
  TLockedToken< CGenDescription > mGenDesc;
  CGenDescription* mLoadedGenDesc;
  EModelOrientationType mOrientType;
  rstl::vector< CParticle > mParticles;
  rstl::vector< CVector3f > x40;
  rstl::vector< CMatrix3f > mParentMatrices;
  rstl::vector< CAdvancedValues > mAdvValues;
  int mInternalStartFrame;
  int mCurFrame;
  double mCurSeconds;
  float mTimeDeltaScale;
  int mPrevFrame;
  bool mParticleEmission;
  float mGeneratorRemainder;
  int mMAXP;
  short mRandomSeed;
  float mGeneratorRate;
  float mExternalVars[16];
  CVector3f mTranslation;
  CVector3f mGlobalTranslation;
  CVector3f mPOFS;
  CVector3f mGlobalScale;
  CTransform4f mGlobalScaleTransform;
  CTransform4f mGlobalScaleTransformInverse;
  CVector3f mLocalScale;
  CTransform4f mLocalScaleTransform;
  CTransform4f mLocalScaleTransformInverse;
  CTransform4f mOrientation;
  CMatrix3f mOrientationInverse;
  CTransform4f mGlobalOrientation;
  uint mActiveParticleCount;
  uint mCumulativeParticles;
  uint mRecursiveParticleCount;
  int mPSLT;
  bool mTranslationDirty : 1;
  bool mLIT_ : 1;
  bool mAAPH : 1;
  bool mZBUF : 1;
  bool mZTest : 1;
  bool mORNT : 1;
  bool mMBLR : 1;
  bool mLINE : 1;
  bool mFXLL : 1;
  bool mWarmedUp : 1;
  bool mModelsUseLights : 1;
  bool mEnableOPTS : 1;
  bool mEnableADV : 1;
  int mMBSP;
  uchar mBackupLightActive;
  // uchar x275_pad[3];
  union {
    struct {
      bool mHasVMD[4];
    };
    uint mVmdStates;
  };
  CRandom16 mRandState;
  CModVectorElement* mVELSources[4];
  rstl::vector< CParticleGen* > mActivePartChildren;
  int mCSSD;
  int mSISY;
  int mPISY;
  int mSSSD;
  CVector3f mSSPO;
  int mSESD;
  CVector3f mSEPO;
  float x2cc;
  float x2d0;
  CVector3f mAabbMin;
  CVector3f mAabbMax;
  float mMaxSize;
  CAABox mSystemBounds;
  LightType mLightType;
  CColor mLCLR;
  float mLINT;
  CVector3f mLOFF;
  CVector3f mLDIR;
  EFalloffType mFalloffType;
  float mLFOR;
  float mLSLA;
  CColor mModuColor;

  static double kTickTime;
  static ushort sSeed;
  static int mParticleAliveCount;
  static int mParticleSystemAliveCount;
  static bool sSubtractBlend;
  static bool sMoveRedToAlphaBuffer;
};
CHECK_SIZEOF(CElementGen, 0x340)

#pragma cpp_extensions reset

#endif // _CELEMENTGEN
