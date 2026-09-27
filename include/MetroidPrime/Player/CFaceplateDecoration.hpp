#ifndef _CFACEPLATEDECORATION
#define _CFACEPLATEDECORATION

#include "Kyoto/TToken.hpp"

#include "rstl/optional_object.hpp"

class CTexture;
class CStateManager;

class CFaceplateDecoration {
  CAssetId mId;
  rstl::optional_object< TToken< CTexture > > mTex;

public:
  explicit CFaceplateDecoration(const CStateManager& stateMgr);
  void Update(float dt, const CStateManager& stateMgr);
  void Draw(const CStateManager& stateMgr) const;
};

#endif // _CFACEPLATEDECORATION
