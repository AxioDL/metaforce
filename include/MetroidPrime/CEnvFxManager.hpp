#ifndef _CENVFXMANAGER
#define _CENVFXMANAGER

#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Audio/CSfxHandle.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector2i.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"
#include "rstl/pair.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CGenDescription;
class CStateManager;
class CTexture;
class CTransform4f;

enum EEnvFxType {
  kEFX_None,
  kEFX_Snow,
  kEFX_Rain,
  kEFX_UnderwaterFlake,
};

enum EPhazonType { kPT_None, kPT_Blue, kPT_Orange };

class CVectorFixed8_8 {
public:
  CVectorFixed8_8() : x(0), y(0), z(0) {}
  CVectorFixed8_8(short xi, short yi, short zi) : x(xi), y(yi), z(zi) {}

  const short GetX() const { return x; }
  const short GetY() const { return y; }
  const short GetZ() const { return z; }

  CVectorFixed8_8& operator+=(const CVectorFixed8_8& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
  }

  CVectorFixed8_8& operator-=(const CVectorFixed8_8& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
  }

  CVectorFixed8_8& operator=(const CVectorFixed8_8& rhs) {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
  }

  CVectorFixed8_8 operator+(const CVectorFixed8_8& rhs) const {
    short outX = x + rhs.x;
    short outY = y + rhs.y;
    short outZ = z + rhs.z;
    return CVectorFixed8_8(outX, outY, outZ);
  }

  CVectorFixed8_8 operator-(const CVectorFixed8_8& rhs) const {
    short outX = x - rhs.x;
    short outY = y - rhs.y;
    short outZ = z - rhs.z;
    return CVectorFixed8_8(outX, outY, outZ);
  }

  static inline CVectorFixed8_8 FromCVector3f(const CVector3f& v);

  short x;
  short y;
  short z;
};
CHECK_SIZEOF(CVectorFixed8_8, 0x6);

inline short real_to_fixed8_8(float f) {
  return static_cast< short >(static_cast< int >(256.f * f));
}

inline float fixed8_8_to_real(short val) { return (1.f / 256.f) * static_cast< float >(val); }

inline CVectorFixed8_8 CVectorFixed8_8::FromCVector3f(const CVector3f& v) {
  short x = real_to_fixed8_8(v.GetX());
  short y = real_to_fixed8_8(v.GetY());
  short z = real_to_fixed8_8(v.GetZ());
  return CVectorFixed8_8(x, y, z);
}

class CEnvFxManagerGrid {
  friend class CEnvFxManager;

public:
  CEnvFxManagerGrid(const CVector2i& position, const CVector2i& extent,
                    const rstl::vector< CVectorFixed8_8 >& initialParticles, int reserve);

  void Render(const CTransform4f& xf, const CTransform4f& invXf, const CTransform4f& camXf,
              float fxDensity, EEnvFxType fxType);

  void SetDirty(bool dirty) { mBlockDirty = dirty; }
  bool IsDirty() const { return mBlockDirty; }

  const CVector2i& GetStart() const { return mPosition; }
  const CVector2i& GetSize() const { return mExtent; }
  void SetStart(const CVector2i& start) { mPosition = start; }

  rstl::pair< bool, float > GetVisibility() const { return mBlock; }
  void SetVisibility(rstl::pair< bool, float > vis) { mBlock = vis; }

  rstl::vector< CVectorFixed8_8 >& Particles() { return mParticles; }
  const rstl::vector< CVectorFixed8_8 >& Particles() const { return mParticles; }

private:
  void RenderSnowParticles(const CTransform4f& camXf);
  void RenderRainParticles(const CTransform4f& camXf);
  void RenderUnderwaterParticles(const CTransform4f& camXf);

  bool mBlockDirty : 1;
  CVector2i mPosition;               /* 8.8 fixed point */
  CVector2i mExtent;                 /* 8.8 fixed point */
  rstl::pair< bool, float > mBlock; /* Blocked-bool, Z-coordinate */
  rstl::vector< CVectorFixed8_8 > mParticles;
};
CHECK_SIZEOF(CEnvFxManagerGrid, 0x2c);

class CEnvFxManager {
public:
  CEnvFxManager();

  void Update(float dt, CStateManager& mgr);
  void Render(const CStateManager& mgr);
  static void Initialize();
  void SetFxDensity(int, float);
  void AreaLoaded();
  void AsyncLoadResources(CStateManager& mgr);
  void SetSplashRate(float f) { mBaseSplashRate = f; }
  bool IsSplashActive() const { return mEnableSplash; }
  float GetRainMagnitude() const { return mFxDensity; }
  void Cleanup();

private:
  void SetSplashEffectRate(float rate, CStateManager& mgr);
  void UpdateRainSounds(CStateManager& mgr);
  CVector3f GetParticleBoundsToWorldScale() const;
  CTransform4f GetParticleBoundsToWorldTransform() const;
  void UpdateVisorSplash(CStateManager& mgr, float dt, const CTransform4f& camXf);
  void MoveWrapCells(int moveX, int moveY);
  void CalculateSnowForces(const CVectorFixed8_8& zVec,
                           rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces,
                           EEnvFxType type, const CVector3f& oopbtws, float dt);
  static void BuildBlockObjectList(rstl::reserved_vector< TUniqueId, 1024 >& list,
                                   CStateManager& mgr);
  void UpdateBlockedGrids(CStateManager& mgr, EEnvFxType type, const CTransform4f& camXf,
                          const CTransform4f& xf, const CTransform4f& invXf);
  void CreateNewParticles(EEnvFxType type);
  void UpdateSnowParticles(rstl::reserved_vector< CVectorFixed8_8, 256 >& snowForces);
  void UpdateRainParticles(const CVectorFixed8_8& zVec, const CVector3f& oopbtws, float dt);
  void UpdateUnderwaterParticles(const CVectorFixed8_8& zVec);
  void SetupSnowTevs(CStateManager& mgr);
  void SetupRainTevs();
  void SetupUnderwaterTevs(const CTransform4f& invXf, CStateManager& mgr);
  void SetupDefaultTevSwapMode();
  void BlankFirstSnowflakeMip(CTexture& tex);

  CAABox mParticleBounds;
  CVector3f mFocusCellPosition;
  bool mEnableSplash;
  float mFirstSnowForce;
  int mLastBlockedGridIdx;
  float mFxDensity;
  float mTargetFxDensity;
  float mMaxDensityDeltaSpeed;
  bool mSnowflakeTextureMipBlanked;
  rstl::optional_object< TLockedToken< CTexture > > mTxtrEnvGradient;
  rstl::reserved_vector< CEnvFxManagerGrid, 64 > mGrids;
  float mBaseSplashRate;
  rstl::optional_object< TLockedToken< CGenDescription > > mEnvRainSplash;
  TUniqueId mEnvRainSplashId;
  bool mRainSoundActive;
  CSfxHandle mLeftRainSound;
  CSfxHandle mRightRainSound;
  rstl::optional_object< TLockedToken< CTexture > > mTxtrSnowFlake;
  rstl::reserved_vector< CVector3f, 16 > mSnowZDeltas;
  rstl::optional_object< TLockedToken< CTexture > > mUnderwaterFlake;
};
CHECK_SIZEOF(CEnvFxManager, 0xc58);

#endif // _CENVFXMANAGER
