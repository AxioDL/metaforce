#ifndef _CPARTICLEELECTRIC
#define _CPARTICLEELECTRIC

#include "Kyoto/Particles/CParticleGen.hpp"

#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/list.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CElectricDescription;
class CElementGen;
class CParticleSwoosh;

class CParticleElectric : public CParticleGen {
public:
  explicit CParticleElectric(TToken< CElectricDescription > desc);

  ~CParticleElectric() override;
  const bool Update(double) override;
  void Render() override;
  void SetOrientation(const CTransform4f& orientation) override;
  void SetTranslation(const CVector3f& translation) override;
  void SetGlobalOrientation(const CTransform4f& orientation) override;
  void SetGlobalTranslation(const CVector3f& translation) override;
  void SetGlobalScale(const CVector3f& scale) override;
  void SetLocalScale(const CVector3f& scale) override;
  void SetParticleEmission(const bool emission) override;
  void SetModulationColor(const CColor& col) override;
  const CTransform4f& GetOrientation() const override;
  const CVector3f& GetTranslation() const override;
  const CTransform4f& GetGlobalOrientation() const override;
  const CVector3f& GetGlobalTranslation() const override;
  const CVector3f& GetGlobalScale() const override { return mGlobalScale; }
  bool GetParticleEmission() const override { return mEmitting; }
  const CColor& GetModulationColor() const override { return mModuColor; }
  bool IsSystemDeletable() const override;
  rstl::optional_object< CAABox > GetBounds() const override;
  int GetParticleCount() const override;
  bool SystemHasLight() const override;
  CLight GetLight() const override;
  void DestroyParticles() override;
  uint Get4CharId() const override;

  class CLineManager {
    friend class CParticleElectric;
    rstl::vector< CVector3f, rstl::aligned_allocator > mVerts;
    float mWidth1;
    float mWidth2;
    float mWidth3;
    CColor mColor1;
    CColor mColor2;
    CColor mColor3;
    CAABox mBounds;

  public:
    CLineManager()
    : mWidth1(1.f)
    , mWidth2(2.f)
    , mWidth3(3.f)
    , mColor1(CColor::White())
    , mColor2(CColor::White())
    , mColor3(CColor::White())
    , mBounds(CAABox::MakeMaxInvertedBox()) {}
  };

  class CParticleElectricManager {
    friend class CParticleElectric;
    uint mIdx;
    uint mSlif;
    uint mStartFrame;
    uint mEndFrame;
    int mGpsmIdx;
    int mEpsmIdx;

  public:
    CParticleElectricManager(uint idx, uint slif, uint startFrame)
    : mIdx(idx)
    , mSlif(slif)
    , mStartFrame(startFrame)
    , mEndFrame(startFrame + slif)
    , mGpsmIdx(-1)
    , mEpsmIdx(-1) {}
  };

  void ForceParticleCreation(int count);
  void SetOverrideIPos(const CVector3f& vec) { mOverrideIPos = vec; }
  void SetOverrideIVel(const CVector3f& vec) { mOverrideIVel = vec; }
  void SetOverrideFPos(const CVector3f& vec) { mOverrideFPos = vec; }
  void SetOverrideFVel(const CVector3f& vec) { mOverrideFVel = vec; }
  static void SetGlobalSeed(ushort seed) { sSeed = seed; }

private:
  void SetupLineGXMaterial();
  void DrawLineStrip(const rstl::vector< CVector3f, rstl::aligned_allocator >& verts, float width,
                     const CColor& color);
  void RenderLines();
  void RenderSwooshes();
  void UpdateCachedTransform();
  void UpdateLine(int idx, int frame);
  void UpdateElectricalEffects();
  void CalculateFractal(int start, int end, float ampl, float ampd);
  void CalculatePoints();
  void CreateNewParticles(int count);
  void AddElectricalEffects();
  void BuildBounds();

  TLockedToken< CElectricDescription > mElecDesc;
  int mCurrentFrame;
  int mLIFE;
  double mCurTime;
  CVector3f mTranslation;
  CTransform4f mOrientation;
  CTransform4f mInvOrientation;
  CVector3f mGlobalTranslation;
  CTransform4f mGlobalOrientation;
  CVector3f mGlobalScale;
  CVector3f mLocalScale;
  CTransform4f mCachedXf;
  float x128_;
  float mRenderTime;
  CVector3f mMinBounds;
  CVector3f mMaxBounds;
  float x148_;
  CRandom16 mRandState;
  int mSSEG;
  int mSCNT;
  int x158_;
  float mGenRem;
  CAABox mSystemBounds;
  rstl::optional_object< CVector3f > mOverrideIPos;
  rstl::optional_object< CVector3f > mOverrideIVel;
  rstl::optional_object< CVector3f > mOverrideFPos;
  rstl::optional_object< CVector3f > mOverrideFVel;
  CColor mModuColor;
  rstl::reserved_vector< bool, 32 > mAllocated;
  rstl::reserved_vector< rstl::auto_ptr< CParticleSwoosh >, 32 > mSwooshGenerators;
  rstl::reserved_vector< rstl::auto_ptr< CLineManager >, 32 > mLineManagers;
  rstl::list< CParticleElectricManager > mElectricManagers;
  rstl::vector< rstl::auto_ptr< CElementGen > > mGpsmGenerators;
  rstl::vector< rstl::auto_ptr< CElementGen > > mEpsmGenerators;
  rstl::vector< CVector3f, rstl::aligned_allocator > mCalculatedVerts;
  rstl::vector< float > mFractalMags;
  rstl::vector< CVector3f > mFractalOffsets;
  bool mEmitting : 1;
  bool mHaveGPSM : 1;
  bool mHaveEPSM : 1;
  bool mHaveSSWH : 1;
  bool mHaveLWD : 1;
  bool mTransformDirty : 1;

  static ushort sSeed;
};
CHECK_SIZEOF(CParticleElectric, 0x458)
NESTED_CHECK_SIZEOF(CParticleElectric, CLineManager, 0x40)
NESTED_CHECK_SIZEOF(CParticleElectric, CParticleElectricManager, 0x18)

#endif // _CPARTICLEELECTRIC
