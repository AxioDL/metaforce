#ifndef _CLIGHT
#define _CLIGHT

#include "Kyoto/Graphics/CColor.hpp"
#include "Kyoto/Math/CVector3f.hpp"

enum ELightType {
  kLT_Spot = 0,
  kLT_Point = 1,
  kLT_Directional = 2,
  kLT_LocalAmbient = 3,
  kLT_Custom = 4,
};

enum EFalloffType { kFT_Constant, kFT_Linear, kFT_Quadratic };

class CLight {
  static const CVector3f kDefaultPosition;
  static const CVector3f kDefaultDirection;

  float CalculateLightRadius() const;

public:
  CLight(ELightType type, const CVector3f& position, const CVector3f& direction,
         const CColor& color, float cutoff);
  CLight(const CVector3f& pos, const CVector3f& direction, const CColor& color, float distC,
         float distL, float distQ, float angleC, float angleL, float angleQ);

  void SetPosition(const CVector3f& pos);
  const CVector3f& GetPosition() const { return mPos; }
  void SetDirection(const CVector3f& dir);
  const CVector3f& GetDirection() const { return mDir; }
  void SetColor(const CColor& col);
  void SetAttenuation(float constant, float linear, float quadratic);
  float GetSpotCutoff() const { return mSpotCutoff; }
  float GetAttenuationConstant() const { return mDistC; }
  float GetAttenuationLinear() const { return mDistL; }
  float GetAttenuationQuadratic() const { return mDistQ; }

  void SetAngleAttenuation(float constant, float linear, float quadratic);
  float GetAngleAttenuationConstant() const { return mAngleC; }
  float GetAngleAttenuationLinear() const { return mAngleL; }
  float GetAngleAttenuationQuadratic() const { return mAngleQ; }

  ELightType GetType() const { return mType; }
  int GetId() const { return mLightId; }
  float GetIntensity() const;
  float GetRadius() const;
  const CColor& GetColor() const { return mColor; }

  int GetPriority() const { return mPriority; }
  void SetPriority(uint priority) { mPriority = priority; }
  void SetId(int lightId) { mLightId = lightId; }

  CVector3f GetNormalIndependentLightingAtPoint(const CVector3f& point) const;

  static CLight BuildDirectional(const CVector3f& dir, const CColor& color);
  static CLight BuildSpot(const CVector3f& pos, const CVector3f& dir, const CColor& color,
                          float cutoff);
  static CLight BuildPoint(const CVector3f& pos, const CColor& color);
  static CLight BuildCustom(const CVector3f& pos, const CVector3f& dir, const CColor& color,
                            float distC, float distL, float distQ, float angleC, float angleL,
                            float angleQ);
  static CLight BuildLocalAmbient(const CVector3f& pos, const CColor& color);

private:
  CVector3f mPos;
  CVector3f mDir;
  CColor mColor;
  ELightType mType;
  float mSpotCutoff;
  float mDistC;
  float mDistL;
  float mDistQ;
  float mAngleC;
  float mAngleL;
  float mAngleQ;
  int mPriority;
  int mLightId;
  mutable float mCachedRadius;
  mutable float mCachedIntensity;
  mutable bool mIntensityDirty : 1;
  mutable bool mRadiusDirty : 1;
};
CHECK_SIZEOF(CLight, 0x50)

#endif // _CLIGHT
