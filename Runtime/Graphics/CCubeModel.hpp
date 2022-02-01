#pragma once

#include <memory>
#include <vector>

#include "GCNTypes.hpp"
#include "IObjectStore.hpp"
#include "CTexture.hpp"
#include "CToken.hpp"

namespace metaforce {
class CCubeSurface;
class CCubeModel;

struct SShader {
  std::vector<TCachedToken<CTexture>> x0_textures;
  CCubeModel* x10_model;

  SShader(CCubeModel* model) : x10_model(model) {}
};

class CModel {
  std::unique_ptr<u8[]> x0_data;
  u32 x4_dataLen;
  std::vector<CCubeSurface*> x8_surfaces;
  std::vector<SShader> x18_matSets;
  CCubeModel* x28_modelInst;
  u16 x2c_;
  u16 x2e_;
  CModel* x30_next;
  CModel* x34_prev;
  u32 x38_lastFrame;

public:
  CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store);
};
} // namespace metaforce
