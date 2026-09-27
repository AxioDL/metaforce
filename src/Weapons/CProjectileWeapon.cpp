#include "Weapons/CProjectileWeapon.hpp"
#include "Kyoto/Graphics/CLight.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Weapons/IWeaponRenderer.hpp"
#include "rstl/optional_object.hpp"

#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/CRandom16.hpp>
#include <Kyoto/Graphics/CGraphics.hpp>
#include <Kyoto/Graphics/CModel.hpp>
#include <Kyoto/Graphics/CModelFlags.hpp>
#include <Kyoto/Math/CRelAngle.hpp>
#include <Kyoto/Math/CTransform4f.hpp>
#include <Kyoto/Math/CloseEnough.hpp>
#include <Kyoto/Particles/CElementGen.hpp>
#include <Kyoto/Particles/CParticleGlobals.hpp>
#include <Kyoto/Particles/CParticleSwoosh.hpp>
#include <Kyoto/TToken.hpp>
#include <MetaRender/CCubeRenderer.hpp>
#include <Weapons/CCollisionResponseData.hpp>

uint CProjectileWeapon::skGlobalSeed = 99;

float CProjectileWeapon::GetTickPeriod() { return 1 / 60.f; }

CProjectileWeapon::CProjectileWeapon(const TToken< CWeaponDescription >& description,
                                     const CVector3f& worldOffset, const CTransform4f& localToWorld,
                                     const CVector3f& scale, int flags)
: mWeaponDesc(description)
, mRandom(skGlobalSeed)
, mLocalToWorldXf(localToWorld)
, mLocalXf(CTransform4f::Identity())
, mWorldOffset(worldOffset)
#if VERSION >= VERSION_GM8P_00
, mPreviousLocalOffset(CVector3f::Zero())
#endif
, mLocalOffset(CVector3f::Zero())
#if VERSION >= VERSION_GM8P_00
, mInterpolationOffset(CVector3f::Zero())
#endif
, mProjOffset(CVector3f::Zero())
, mScale(CVector3f(1.f, 1.f, 1.f))
, mLocalOffset2(CVector3f::Zero())
, mVelocity(CVector3f::Zero())
, mGravity(CVector3f::Zero())
, mAmbientLightColor(CColor::White())
, mCurTime(0.0)
, mRemainderTime(0.0)
, mMaxTurnRate(0.f)
, mFlags(flags)
, mLifetime(0)
, mChildSystemUpdateRate(0)
, xf0_(0)
, mCurFrame(0)
, mLastParticleFrame(-1)
, mAPSMGen(nullptr)
, mAPS2Gen(nullptr)
, x104_(nullptr)
, mSwoosh1(nullptr)
, mSwoosh2(nullptr)
, mSwoosh3(nullptr)
, mActive(true)
, mAPSO(false)
, mAP11(false)
, mAP21(false)
, mAS11(false)
, mAS12(false)
, mAS13(false)
, mVMD2(false) {
  CGlobalRandom __(mRandom);
  mVMD2 = mWeaponDesc->mVMD2;
  mAPSO = mWeaponDesc->mAPSO;
  uint unk = mFlags & 1;
  if (mWeaponDesc->mAPSM) {
    mAPSMGen = rs_new CElementGen(*mWeaponDesc->mAPSM, CElementGen::kMOT_Normal,
                                     unk ? CElementGen::kOSF_Two : CElementGen::kOSF_One);
    mAPSMGen->SetGlobalScale(scale);
  }

  if (mWeaponDesc->mAPS2) {
    mAPS2Gen = rs_new CElementGen(*mWeaponDesc->mAPS2, CElementGen::kMOT_Normal,
                                      unk ? CElementGen::kOSF_Two : CElementGen::kOSF_One);
    mAPS2Gen->SetGlobalScale(scale);
  }
  if (mWeaponDesc->mASW1) {
    mSwoosh1 = rs_new CParticleSwoosh(*mWeaponDesc->mASW1, 0);
    mSwoosh1->SetGlobalScale(scale);
  }
  if (mWeaponDesc->mASW2) {
    mSwoosh2 = rs_new CParticleSwoosh(*mWeaponDesc->mASW2, 0);
    mSwoosh2->SetGlobalScale(scale);
  }
  if (mWeaponDesc->mASW3) {
    mSwoosh3 = rs_new CParticleSwoosh(*mWeaponDesc->mASW3, 0);
    mSwoosh3->SetGlobalScale(scale);
  }

  if (mWeaponDesc->mPSLT) {
    mWeaponDesc->mPSLT->GetValue(0, mLifetime);
  } else {
    mLifetime = 0x7FFFFF;
  }

  if (mWeaponDesc->mIVEC) {
    mWeaponDesc->mIVEC->GetValue(0, mVelocity);
  }

  if (mWeaponDesc->mIORN) {
    CTransform4f orient(CTransform4f::Identity());
    CVector3f angle(0.f, 0.f, 0.f);
    mWeaponDesc->mIORN->GetValue(0, angle);
    CRelAngle relAngleX = CRelAngle::FromDegrees(angle.GetX());
    orient.RotateLocalX(relAngleX);
    CRelAngle relAngleY = CRelAngle::FromDegrees(angle.GetY());
    orient.RotateLocalY(relAngleY);
    CRelAngle relAngleZ = CRelAngle::FromDegrees(angle.GetZ());
    orient.RotateLocalZ(relAngleZ);
    SetRelativeOrientation(orient);
  } else {
    SetRelativeOrientation(CTransform4f::Identity());
  }
  if (mWeaponDesc->GetOHEF()) {
    mModel = *mWeaponDesc->GetOHEF();
  }

  mAP11 = mWeaponDesc->mAP11;
  mAP21 = mWeaponDesc->mAP21;
  mAS11 = mWeaponDesc->mAS11;
  mAS12 = mWeaponDesc->mAS12;
  mAS13 = mWeaponDesc->mAS13;
  UpdateChildParticleSystems(1.f / 60.f);
}

CProjectileWeapon::~CProjectileWeapon() {
  delete mAPSMGen;
  delete mAPS2Gen;
  delete x104_;
  delete mSwoosh1;
  delete mSwoosh2;
  delete mSwoosh3;
}

bool CProjectileWeapon::Update(float dt) {
  CGlobalRandom __(mRandom);

  double actualTime = mCurFrame * (1.0 / 60.0);
  mChildSystemUpdateRate = 0;
  double useDt = close_enough(dt, 1.f / 60.f, 1.6666666851961054e-5f) ? (1.0 / 60.0) : dt;
  double timeScale = 1.0;
  useDt *= timeScale;
  if (useDt < 0.f) {
    useDt = 0.f;
  }
  mCurTime += useDt;
  while (actualTime < mCurTime &&
         !close_enough(actualTime, mCurTime, 1.6666666666666667e-5)) {
    if (mCurFrame < mLifetime) {
      CParticleGlobals::SetEmitterTime(mCurFrame);
      CParticleGlobals::SetParticleLifetime(mLifetime);
      CParticleGlobals::UpdateParticleLifetimeTweenValues(mCurFrame);
      UpdatePSTranslationAndOrientation();
    }

    actualTime += 1.f / 60.f;
    ++mCurFrame;
    ++mChildSystemUpdateRate;
  }

  if (close_enough(actualTime, mCurTime, 1.6666666666666667e-5)) {
    mCurTime = actualTime;
  }

  mRemainderTime = (float)((actualTime - mCurTime) / (1.f / 60.0));
#if VERSION >= VERSION_GM8P_00
  mInterpolationOffset =
      static_cast< float >(mRemainderTime) * (mPreviousLocalOffset - mLocalOffset);
#endif

  if (mCurFrame < mLifetime) {
    mMaxTurnRate = 0.f;
    if (CRealElement* trat = mWeaponDesc->mTRAT) {
      trat->GetValue(0, mMaxTurnRate);
    }
  }

  return false;
}

void CProjectileWeapon::UpdateParticleFX() {
#if VERSION >= VERSION_GM8P_00
  if (mAPSMGen && mWeaponDesc->mSPS1) {
    mAPSMGen->Update(1.f / 60.f);
  }
  if (mAPS2Gen && mWeaponDesc->mSPS2) {
    mAPS2Gen->Update(1.f / 60.f);
  }
#endif
  for (int i = 0; i < mChildSystemUpdateRate; ++i) {
    UpdateChildParticleSystems(1.f / 60.f);
  }
}

const CTransform4f CProjectileWeapon::GetTransform() const {
  return mLocalToWorldXf * mLocalXf;
}

CTransform4f CProjectileWeapon::GetTransform() { return mLocalToWorldXf * mLocalXf; }

const CVector3f CProjectileWeapon::GetTranslation() const {
#if VERSION >= VERSION_GM8P_00
  return mLocalToWorldXf *
             (mLocalOffset + mInterpolationOffset + mLocalXf * mProjOffset) +
         mWorldOffset;
#else
  return mLocalToWorldXf * (mLocalOffset + mLocalXf * mProjOffset) + mWorldOffset;
#endif
}

void CProjectileWeapon::SetRelativeOrientation(const CTransform4f& orient) { mLocalXf = orient; }

void CProjectileWeapon::SetWorldSpaceOrientation(const CTransform4f& orient) {
  mLocalXf = mLocalToWorldXf.GetInverse() * orient;
}

void CProjectileWeapon::UpdatePSTranslationAndOrientation() {
  if (mLifetime >= mCurFrame && mActive) {
#if VERSION >= VERSION_GM8P_00
    mPreviousLocalOffset = mLocalOffset;
#endif
    if (CModVectorElement* psvm = mWeaponDesc->mPSVM) {
      psvm->GetValue(mCurFrame, mVelocity, mLocalOffset);
    }

    if (mVMD2) {
#if VERSION >= VERSION_GM8P_00
      CVector3f velocity = mLocalXf * mVelocity;
      mLocalOffset += velocity;
#else
      mLocalOffset += mLocalXf * mVelocity;
#endif
    } else {
      mLocalOffset += mVelocity;
    }

    mVelocity += mGravity / 60.f;

    if (CVectorElement* psov = mWeaponDesc->mPSOV) {
      CVector3f orient(0.f, 0.f, 0.f);
      psov->GetValue(mCurFrame, orient);

      CTransform4f xf = mLocalXf;
      xf.RotateLocalX(CRelAngle::FromDegrees(orient.GetX()));
      xf.RotateLocalY(CRelAngle::FromDegrees(orient.GetY()));
      xf.RotateLocalZ(CRelAngle::FromDegrees(orient.GetZ()));
      SetRelativeOrientation(xf);
    }

    if (CVectorElement* pscl = mWeaponDesc->mPSCL) {
      pscl->GetValue(mCurFrame, mScale);
    }

    if (CColorElement* pcol = mWeaponDesc->mPCOL) {
      pcol->GetValue(mCurFrame, mAmbientLightColor);
    }

    if (CVectorElement* pofs = mWeaponDesc->mPOFS) {
      pofs->GetValue(mCurFrame, mLocalOffset2);
    }

    if (CVectorElement* ofst = mWeaponDesc->mOFST) {
      ofst->GetValue(mCurFrame, mProjOffset);
    }
  }
}

void CProjectileWeapon::UpdateChildParticleSystems(float dt) {
  double useDt = (close_enough(dt, 1.f / 60.f, 1.6666666851961054e-5f)) ? 1.0 / 60.0 : dt;

  if (mAPSMGen) {
    if (mLastParticleFrame != mCurFrame) {
      if (mCurFrame > mLifetime) {
        mAPSMGen->SetParticleEmission(false);
        mAPSMGen->EndLifetime();
      } else {
        if (mAP11) {
          mAPSMGen->SetGlobalTranslation(GetTranslation());
        } else {
          mAPSMGen->SetTranslation(GetTranslation());
        }

        if (mAPSO) {
          mAPSMGen->SetOrientation(GetTransform());
        }
      }
    }
#if VERSION >= VERSION_GM8P_00
    if (!mWeaponDesc->mSPS1)
#endif
      mAPSMGen->Update(useDt);
    if (mAPSMGen->IsSystemDeletable() == TRUE) {
      delete mAPSMGen;
      mAPSMGen = nullptr;
    }
  }

  if (mAPS2Gen) {
    if (mLastParticleFrame != mCurFrame) {
      if (mCurFrame > mLifetime) {
        mAPS2Gen->SetParticleEmission(false);
        mAPS2Gen->EndLifetime();
      } else {
        if (mAP21) {
          mAPS2Gen->SetGlobalTranslation(GetTranslation());
        } else {
          mAPS2Gen->SetTranslation(GetTranslation());
        }

        if (mAPSO) {
          mAPS2Gen->SetOrientation(GetTransform());
        }
      }
    }

#if VERSION >= VERSION_GM8P_00
    if (!mWeaponDesc->mSPS2)
#endif
      mAPS2Gen->Update(useDt);
    if (mAPS2Gen->IsSystemDeletable() == TRUE) {
      delete mAPS2Gen;
      mAPS2Gen = nullptr;
    }
  }

  if (mSwoosh1) {
    if (mLastParticleFrame != mCurFrame) {
      if (mCurFrame > mLifetime) {
        mSwoosh1->SetParticleEmission(false);
      } else {
        if (mAS11) {
          mSwoosh1->SetGlobalTranslation(GetTranslation());
        } else {
          mSwoosh1->SetTranslation(GetTranslation());
        }
        mSwoosh1->SetOrientation(GetTransform());
      }
    }
    mSwoosh1->SetWarmUp();
    mSwoosh1->Update(0.0);
    if (mSwoosh1->IsSystemDeletable() == TRUE) {
      delete mSwoosh1;
      mSwoosh1 = nullptr;
    }
  }

  if (mSwoosh2) {
    if (mLastParticleFrame != mCurFrame) {
      if (mCurFrame > mLifetime) {
        mSwoosh2->SetParticleEmission(false);
      } else {
        if (mAS12) {
          mSwoosh2->SetGlobalTranslation(GetTranslation());
        } else {
          mSwoosh2->SetTranslation(GetTranslation());
        }
        mSwoosh2->SetOrientation(GetTransform());
      }
    }
    mSwoosh2->SetWarmUp();
    mSwoosh2->Update(0.0);
    if (mSwoosh2->IsSystemDeletable() == TRUE) {
      delete mSwoosh2;
      mSwoosh2 = nullptr;
    }
  }

  if (mSwoosh3) {
    if (mLastParticleFrame != mCurFrame) {
      if (mCurFrame > mLifetime) {
        mSwoosh3->SetParticleEmission(false);
      } else {
        if (mAS13) {
          mSwoosh3->SetGlobalTranslation(GetTranslation());
        } else {
          mSwoosh3->SetTranslation(GetTranslation());
        }
        mSwoosh3->SetOrientation(GetTransform());
      }
    }
    mSwoosh3->SetWarmUp();
    mSwoosh3->Update(0.0);
    if (mSwoosh3->IsSystemDeletable() == TRUE) {
      delete mSwoosh3;
      mSwoosh3 = nullptr;
    }
  }

  if (x104_) {
    x104_->Update(useDt);
    if (x104_->IsSystemDeletable() == TRUE) {
      delete x104_;
      x104_ = nullptr;
    }
  }

  mLastParticleFrame = mCurFrame;
}

const bool CProjectileWeapon::IsSystemDeletable() const {
  bool ret = true;
  if (mAPSMGen && !mAPSMGen->IsSystemDeletable()) {
    ret = false;
  } else if (mAPS2Gen && !mAPS2Gen->IsSystemDeletable()) {
    ret = false;
  } else if (mSwoosh1 && !mSwoosh1->IsSystemDeletable()) {
    ret = false;
  } else if (mSwoosh2 && !mSwoosh2->IsSystemDeletable()) {
    ret = false;
  } else if (mSwoosh3 && !mSwoosh3->IsSystemDeletable()) {
    ret = false;
  } else if (x104_ && !x104_->IsSystemDeletable()) {
    ret = false;
  } else if (mActive) {
    ret = mCurFrame >= mLifetime;
  }
  return ret;
}

void CProjectileWeapon::Render() const {
  if (mCurFrame <= mLifetime && mActive && mModel) {
    CTransform4f localXf = CTransform4f::Translate(
#if VERSION >= VERSION_GM8P_00
        mLocalOffset + mInterpolationOffset + (mLocalXf * mProjOffset) +
        mLocalOffset2);
#else
        mLocalOffset + (mLocalXf * mProjOffset) + mLocalOffset2);
#endif
    CTransform4f worldXf = CTransform4f::Translate(mWorldOffset);
    CTransform4f scaleXf =
        CTransform4f::Scale(mScale.GetX(), mScale.GetY(), mScale.GetZ());
    CGraphics::SetModelMatrix(worldXf * mLocalToWorldXf * localXf * scaleXf * mLocalXf);
    CLight light = CLight::BuildLocalAmbient(CVector3f::Zero(), mAmbientLightColor);
    (*mModel)->Draw(CModelFlags(CModelFlags::kT_Opaque, 1.f));
  }
}

void CProjectileWeapon::AddToRenderer() const {
  if (mAPSMGen)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*mAPSMGen);
  if (mAPS2Gen)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*mAPS2Gen);
  if (mSwoosh1)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*mSwoosh1);
  if (mSwoosh2)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*mSwoosh2);
  if (mSwoosh3)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*mSwoosh3);
  if (x104_)
    IWeaponRenderer::GetRenderer()->AddParticleGen(*x104_);
}

void CProjectileWeapon::RenderParticles() const {
  if (mAPSMGen) {
    mAPSMGen->Render();
  }
  if (mAPS2Gen) {
    mAPS2Gen->Render();
  }
  if (mSwoosh1) {
    mSwoosh1->Render();
  }
  if (mSwoosh2) {
    mSwoosh2->Render();
  }
  if (mSwoosh3) {
    mSwoosh3->Render();
  }
  if (x104_) {
    x104_->Render();
  }
}

rstl::optional_object< TLockedToken< CGenDescription > > CProjectileWeapon::CollisionOccured(
    const EWeaponCollisionResponseTypes colType, const bool deflected, const bool useTarget,
    const CVector3f& pos, const CVector3f& normal, const CVector3f& target) {
  mLocalOffset = mLocalToWorldXf.TransposeRotate(pos - mWorldOffset) - mProjOffset;
#if VERSION >= VERSION_GM8P_00
  mInterpolationOffset = CVector3f::Zero();
#endif

  if (deflected) {
    CVector3f posToTarget = target - GetTranslation();
    if (useTarget && posToTarget.CanBeNormalized()) {
      SetWorldSpaceOrientation(CTransform4f::LookAt(CVector3f::Zero(), posToTarget.AsNormalized()));
    } else {
      const CVector3f& col = GetTransform().GetColumn(kDY);
      CTransform4f lookXf = CTransform4f::LookAt(
          CVector3f::Zero(), col - ((CVector3f::Dot(normal, col) * 2.f) * normal), normal);
      SetWorldSpaceOrientation(lookXf);
    }
    return rstl::optional_object_null();
  }

  mActive = false;
  if (mAPSMGen) {
    mAPSMGen->SetParticleEmission(false);
  }
  if (mAPS2Gen) {
    mAPS2Gen->SetParticleEmission(false);
  }
  if (mSwoosh1) {
    mSwoosh1->SetParticleEmission(false);
  }
  if (mSwoosh2) {
    mSwoosh2->SetParticleEmission(false);
  }
  if (mSwoosh3) {
    mSwoosh3->SetParticleEmission(false);
  }
  if (!mWeaponDesc->mCOLR) {
    return rstl::optional_object_null();
  }
  TToken< CCollisionResponseData > tok = (*mWeaponDesc->GetCollisionResponse());
  return tok->GetParticleDescription(colType);
}

uint CProjectileWeapon::GetSoundIdForCollision(EWeaponCollisionResponseTypes type) const {
  if (!mWeaponDesc->GetCollisionResponse()) {
    return -1;
  }
  TToken< CCollisionResponseData > tok = (*mWeaponDesc->GetCollisionResponse());
  return tok->GetSoundEffectId(type);
}

rstl::optional_object< TLockedToken< CDecalDescription > >
CProjectileWeapon::GetDecalForCollision(EWeaponCollisionResponseTypes type) const {
  if (!mWeaponDesc->GetCollisionResponse()) {
    return rstl::optional_object_null();
  }
  TToken< CCollisionResponseData > tok = (*mWeaponDesc->GetCollisionResponse());
  return tok->GetDecalDescription(type);
}

float CProjectileWeapon::GetAudibleRange() const {
  if (!mWeaponDesc->GetCollisionResponse()) {
    return 0.f;
  }
  TToken< CCollisionResponseData > tok = (*mWeaponDesc->GetCollisionResponse());
  return tok->GetAudibleRange();
}

float CProjectileWeapon::GetAudibleFallOff() const {
  if (!mWeaponDesc->GetCollisionResponse()) {
    return 0.f;
  }
  TToken< CCollisionResponseData > tok = (*mWeaponDesc->GetCollisionResponse());
  return tok->GetAudibleFallOff();
}

float CProjectileWeapon::GetMaxTurnRate() const { return mMaxTurnRate; }

void CProjectileWeapon::SetVelocity(const CVector3f& velocity) { mVelocity = velocity; }

const CVector3f& CProjectileWeapon::GetVelocity() const { return mVelocity; }

void CProjectileWeapon::SetGravity(const CVector3f& gravity) { mGravity = gravity; }

const CVector3f& CProjectileWeapon::GetGravity() const { return mGravity; }

void CProjectileWeapon::SetGlobalSeed(const uint seed) { skGlobalSeed = seed; }

rstl::optional_object< CAABox > CProjectileWeapon::GetBounds() const {
  CAABox ret = CAABox::MakeMaxInvertedBox();
  bool hasBox = false;

  if (mAPSMGen) {
    rstl::optional_object< CAABox > bounds = mAPSMGen->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (mAPS2Gen) {
    rstl::optional_object< CAABox > bounds = mAPS2Gen->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (mSwoosh1) {
    rstl::optional_object< CAABox > bounds = mSwoosh1->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (mSwoosh2) {
    rstl::optional_object< CAABox > bounds = mSwoosh2->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (mSwoosh3) {
    rstl::optional_object< CAABox > bounds = mSwoosh3->GetBounds();
    if (bounds) {
      CAABox& b = *bounds;
      ret.AccumulateBounds(b.GetMinPoint());
      ret.AccumulateBounds(b.GetMaxPoint());
      hasBox = true;
    }
  }

  if (hasBox) {
    return ret;
  }

  return rstl::optional_object_null();
}
