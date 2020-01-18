#include "Runtime/MP1/World/CMetroidPrimeRelay.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {

CMetroidPrimeRelay::CMetroidPrimeRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                                       const zeus::CTransform& xf, const zeus::CVector3f& scale,
                                       SPrimeExoParameters&& parms, float f1, float f2, float f3, u32 w1, bool b1,
                                       u32 w2, const CHealthInfo& hInfo1, const CHealthInfo& hInfo2, u32 w3, u32 w4,
                                       u32 w5, rstl::reserved_vector<SPrimeExoRoomParameters, 4>&& roomParms)
: CEntity(uid, info, active, name)
, x38_xf(xf)
, x68_scale(scale)
, x74_parms(std::move(parms))
, xc84_f1(f1)
, xc88_f2(f2)
, xc8c_f3(f3)
, xc90_w1(w1)
, xc94_b1(b1)
, xc98_w2(w2)
, xc9c_hInfo1(hInfo1)
, xca4_hInfo2(hInfo2)
, xcac_w3(w3)
, xcb0_w4(w4)
, xcb4_w5(w5)
, xcb8_roomParms(std::move(roomParms)) {}

void CMetroidPrimeRelay::Accept(IVisitor& visitor) { visitor.Visit(this); }

} // namespace urde::MP1
