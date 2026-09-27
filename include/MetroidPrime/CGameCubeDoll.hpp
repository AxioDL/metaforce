#ifndef _CGAMECUBEDOLL
#define _CGAMECUBEDOLL

#include "Kyoto/TToken.hpp"

#include "rstl/single_ptr.hpp"
#include "rstl/vector.hpp"

class CActorLights;
class CModel;
class CLight;

class CGameCubeDoll {
public:
  CGameCubeDoll();
  ~CGameCubeDoll();

  void Update(float dt);
  void Draw(float alpha);
  void Touch();
  bool CheckLoadComplete();
  bool IsLoaded() const;

private:
  CToken mModel;
  rstl::vector< CLight > mLights;
  rstl::single_ptr< CActorLights > mActorLights;
  float mFader;
  bool mLoaded : 1;
  
  void UpdateActorLights();
};

#endif // _CGAMECUBEDOLL
