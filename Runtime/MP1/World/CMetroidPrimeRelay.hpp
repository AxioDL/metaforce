#pragma once

#include "Runtime/RetroTypes.hpp"
#include "Runtime/rstl.hpp"
#include "Runtime/MP1/World/CMetroidPrimeExo.hpp"
#include "Runtime/World/CEntity.hpp"

#include <zeus/CTransform.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce::MP1 {

class CMetroidPrimeRelay : public CEntity {
  TUniqueId x34_mpUid = kInvalidUniqueId;
  zeus::CTransform x38_xf;
  zeus::CVector3f x68_scale;
  SPrimeExoParameters x74_parms;
  float xc84_f1;
  float xc88_f2;
  float xc8c_f3;
  u32 xc90_w1;
  bool xc94_b1;
  u32 xc98_w2;
  CHealthInfo xc9c_hInfo1;
  CHealthInfo xca4_hInfo2;
  u32 xcac_w3;
  u32 xcb0_w4;
  u32 xcb4_w5;
  rstl::reserved_vector<CMetroidPrimeAttackWeights, 4> xcb8_roomParms;

  void ForwardMessageToMetroidPrimeExo(EScriptObjectMessage msg, CStateManager& mgr);
  void GetOrBuildMetroidPrimeExo(CStateManager& mgr);

public:
  DEFINE_ENTITY
  CMetroidPrimeRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                     const zeus::CTransform& xf, const zeus::CVector3f& scale, SPrimeExoParameters&& parms, float f1,
                     float f2, float f3, u32 w1, bool b1, u32 w2, const CHealthInfo& hInfo1, const CHealthInfo& hInfo2,
                     u32 w3, u32 w4, u32 w5, rstl::reserved_vector<CMetroidPrimeAttackWeights, 4>&& roomParms);
  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  [[nodiscard]] TUniqueId GetMetroidPrimeExoId() const { return x34_mpUid; }
  void SetMetroidPrimeExoId(TUniqueId uid) { x34_mpUid = uid; }
  [[nodiscard]] float Get_xc84() const { return xc84_f1; }
  [[nodiscard]] float Get_xc88() const { return xc88_f2; }
  [[nodiscard]] float Get_xc8c() const { return xc8c_f3; }
  [[nodiscard]] u32 Get_xc90() const { return xc90_w1; }
  [[nodiscard]] bool Get_xc94() const { return xc94_b1; }
  [[nodiscard]] u32 Get_xc98() const { return xc98_w2; }
  [[nodiscard]] CHealthInfo GetHealthInfo1() const { return xc9c_hInfo1; }
  [[nodiscard]] CHealthInfo GetHealthInfo2() const { return xca4_hInfo2; }
  [[nodiscard]] u32 Get_xcac() const { return xcac_w3; }
  [[nodiscard]] u32 Get_xcb0() const { return xcb0_w4; }
  [[nodiscard]] u32 Get_xcb4() const { return xcb4_w5; }
  [[nodiscard]] rstl::reserved_vector<CMetroidPrimeAttackWeights, 4> GetRoomParameters() const { return xcb8_roomParms; }
};

} // namespace metaforce::MP1
