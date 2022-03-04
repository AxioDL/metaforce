#pragma once

#include "Graphics/GX.hpp"
#include "RetroTypes.hpp"

#include <aurora/gfx.hpp>

namespace metaforce::CTevCombiners {
class CTevPass {
  u32 x0_id;
  ColorPass x4_colorPass;
  AlphaPass x14_alphaPass;
  CTevOp x24_colorOp;
  CTevOp x38_alphaOp;

  static u32 sNextUniquePass;

public:
  CTevPass(const ColorPass& colPass, const AlphaPass& alphaPass, const CTevOp& colorOp = {}, const CTevOp& alphaOp = {})
  : x0_id(++sNextUniquePass)
  , x4_colorPass(colPass)
  , x14_alphaPass(alphaPass)
  , x24_colorOp(colorOp)
  , x38_alphaOp(alphaOp) {}

  void Execute(ERglTevStage stage) const;

  bool operator<=>(const CTevPass&) const = default;
};

extern const CTevPass skPassThru;
extern const CTevPass sTevPass805a5698;
extern const CTevPass sTevPass805a5e70;
extern const CTevPass sTevPass805a5ebc;
extern const CTevPass sTevPass805a5f08;
extern const CTevPass sTevPass805a5f54;
extern const CTevPass sTevPass805a5fa0;
extern const CTevPass sTevPass804bfcc0;
extern const CTevPass sTevPass805a5fec;
extern const CTevPass sTevPass805a6038;
extern const CTevPass sTevPass805a6084;

void Init();
void SetupPass(ERglTevStage stage, const CTevPass& pass);
void DeletePass(ERglTevStage stage);
bool SetPassCombiners(ERglTevStage stage, const CTevPass& pass);
void RecomputePasses();
void ResetStates();
} // namespace metaforce::CTevCombiners
