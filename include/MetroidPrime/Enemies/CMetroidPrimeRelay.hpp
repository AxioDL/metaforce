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

  TUniqueId GetMetroidPrimeExoId() const { return x34_mpUid; }
  void SetMetroidPrimeExoId(TUniqueId uid) { x34_mpUid = uid; }

  const CTransform4f& GetTransform() const { return x38_xf; }
  const CVector3f& GetScale() const { return x68_scale; }
  const CMetroidPrimeData& GetParms() const { return x74_parms; }
  float GetFloat1() const { return xc84_f1; }
  float GetFloat2() const { return xc88_f2; }
  float GetFloat3() const { return xc8c_f3; }
  int GetW1() const { return xc90_w1; }
  bool GetB1() const { return xc94_b1; }
  int GetW2() const { return xc98_w2; }
  const CHealthInfo& GetHealthInfo1() const { return xc9c_hInfo1; }
  const CHealthInfo& GetHealthInfo2() const { return xca4_hInfo2; }
  int GetW3() const { return xcac_w3; }
  int GetW4() const { return xcb0_w4; }
  int GetW5() const { return xcb4_w5; }
  const rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 >&
  GetRoomParms() const {
    return xcb8_roomParms;
  }

  void CreateMetroidPrime(CStateManager& mgr);
  void ForwardScriptMessageToMP(EScriptObjectMessage msg, CStateManager& mgr);

private:
  TUniqueId x34_mpUid;
  CTransform4f x38_xf;
  CVector3f x68_scale;
  CMetroidPrimeData x74_parms;
  float xc84_f1;
  float xc88_f2;
  float xc8c_f3;
  int xc90_w1;
  bool xc94_b1;
  int xc98_w2;
  CHealthInfo xc9c_hInfo1;
  CHealthInfo xca4_hInfo2;
  int xcac_w3;
  int xcb0_w4;
  int xcb4_w5;
  rstl::reserved_vector< CMetroidPrime::CMetroidPrimeAttackWeights, 4 > xcb8_roomParms;
};
CHECK_SIZEOF(CMetroidPrimeRelay, 0xDAC)

#endif // _CMETROIDPRIMERELAY
