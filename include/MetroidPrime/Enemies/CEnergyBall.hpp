#ifndef _CENERGYBALL
#define _CENERGYBALL

#include "MetroidPrime/Enemies/CPatterned.hpp"

#include "MetroidPrime/CDamageInfo.hpp"

class CEnergyBall : public CPatterned {
public:
  CEnergyBall(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
              const CTransform4f& xf, const CModelData& mData, const CActorParameters& aParms,
              const CPatternedInfo& pInfo, int w1, float f1, const CDamageInfo& dInfo1, float f2,
              uint a1, ushort sfxId1, uint a2, uint a3, ushort sfxId2, float f3, float f4,
              uint a4, const CDamageInfo& dInfo2, float f5);
  ~CEnergyBall();

  DECLARE_TYPES_MATCH_OR_ACCEPT;
  int GetBallType() const { return x570_ballType; }

private:
  uchar x568_pad[8];
  int x570_ballType;
  uchar x574_pad[0x5F0 - 0x574];
};
CHECK_SIZEOF(CEnergyBall, (VERSION >= VERSION_GM8P_00 ? 0x600 : 0x5F0))

#endif // _CENERGYBALL
