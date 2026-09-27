#ifndef _CMETROIDPRIMERELAY
#define _CMETROIDPRIMERELAY

#include "MetroidPrime/CEntity.hpp"
#include "MetroidPrime/CHealthInfo.hpp"
#include "MetroidPrime/Enemies/CMetroidPrime.hpp"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "MetroidPrime/CAnimRes.hpp"

class CMetroidPrimeRelay : public CEntity {
public:
  CMetroidPrimeRelay(
      TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool active,
      const CTransform4f& xf, const CVector3f& scale, const CMetroidPrimeData& parms, float f1,
      float f2, float f3, int w1, bool b1, int w2, const CHealthInfo& hInfo1,
      const CHealthInfo& hInfo2, int w3, int w4, int w5,
      const rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 >& roomParms);

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  TUniqueId GetMetroidPrimeExoId() const { return mMpUid; }
  void SetMetroidPrimeExoId(TUniqueId uid) { mMpUid = uid; }

  const CTransform4f& GetTransform() const { return mXf; }
  const CVector3f& GetScale() const { return mScale; }
  const CMetroidPrimeData& GetParms() const { return mParms; }
  float GetFloat1() const { return mF1; }
  float GetFloat2() const { return mF2; }
  float GetFloat3() const { return mF3; }
  int GetW1() const { return mW1; }
  bool GetB1() const { return mB1; }
  int GetW2() const { return mW2; }
  const CHealthInfo& GetHealthInfo1() const { return mHInfo1; }
  const CHealthInfo& GetHealthInfo2() const { return mHInfo2; }
  int GetW3() const { return mW3; }
  int GetW4() const { return mW4; }
  int GetW5() const { return mW5; }
  const rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 >&
  GetRoomParms() const {
    return mRoomParms;
  }

  void CreateMetroidPrime(CStateManager& mgr);
  void ForwardScriptMessageToMP(EScriptObjectMessage msg, CStateManager& mgr);

private:
  TUniqueId mMpUid;
  CTransform4f mXf;
  CVector3f mScale;
  CMetroidPrimeData mParms;
  float mF1;
  float mF2;
  float mF3;
  int mW1;
  bool mB1;
  int mW2;
  CHealthInfo mHInfo1;
  CHealthInfo mHInfo2;
  int mW3;
  int mW4;
  int mW5;
  rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 > mRoomParms;
};
CHECK_SIZEOF(CMetroidPrimeRelay, 0xDAC)

#endif // _CMETROIDPRIMERELAY
