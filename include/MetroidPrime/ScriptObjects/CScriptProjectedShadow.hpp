#ifndef _CSCRIPTPROJECTEDSHADOW
#define _CSCRIPTPROJECTEDSHADOW

#include "MetroidPrime/CActor.hpp"

#include "rstl/single_ptr.hpp"

class CProjectedShadow;

class CScriptShadowProjector : public CActor {
public:
  CScriptShadowProjector(TUniqueId, const rstl::string&, const CEntityInfo&, const CTransform4f&,
                         bool, const CVector3f&, bool, float, float, float, float, int);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float, CStateManager&) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const CFrustumPlanes&) override;
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void CreateProjectedShadow();

  uchar GetPersistent() const { return mPersistent; }

private:
  float mScale;
  CVector3f mOffset;
  float mZOffsetAdjust;
  float mOpacity;
  float mOpacityRecip;
  TUniqueId mTarget;
  rstl::single_ptr< CProjectedShadow > mProjectedShadow;
  uint mTextureSize;
  uchar mPersistent : 1;
  bool mShadowInvalidated : 1;
};
CHECK_SIZEOF(CScriptShadowProjector, (VERSION >= VERSION_GM8E_02 ? 0x128 : 0x118))

#endif // _CSCRIPTPROJECTEDSHADOW
