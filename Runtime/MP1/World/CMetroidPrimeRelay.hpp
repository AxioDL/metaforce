#pragma once

#include "World/CEntity.hpp"
#include "CMetroidPrimeExo.hpp"

namespace urde::MP1
{

class CMetroidPrimeRelay : public CEntity
{
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
    rstl::reserved_vector<SPrimeExoRoomParameters, 4> xcb8_roomParms;

public:
    CMetroidPrimeRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                       const zeus::CTransform& xf, const zeus::CVector3f& scale, SPrimeExoParameters&& parms,
                       float f1, float f2, float f3, u32 w1, bool b1, u32 w2, const CHealthInfo& hInfo1,
                       const CHealthInfo& hInfo2, u32 w3, u32 w4, u32 w5,
                       rstl::reserved_vector<SPrimeExoRoomParameters, 4>&& roomParms);
    void Accept(IVisitor& visitor);
    TUniqueId GetMetroidPrimeExoId() const { return x34_mpUid; }
};

}

