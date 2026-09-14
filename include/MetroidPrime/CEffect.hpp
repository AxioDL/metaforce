#ifndef _CEFFECT
#define _CEFFECT

#include "types.h"

#include "MetroidPrime/CActor.hpp"

class CEffect : public CActor {
public:
  CEffect(TUniqueId uid, const CEntityInfo& info, bool, const rstl::string& name,
          const CTransform4f& xf);

  // CEntity
  ~CEffect() override {}
  DECLARE_TYPES_MATCH;

  // CActor
  void AddToRenderer(const CFrustumPlanes&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
};
CHECK_SIZEOF(CEffect, (VERSION >= VERSION_GM8P_00 ? 0xf8 : 0xe8))

#endif // _CEFFECT
