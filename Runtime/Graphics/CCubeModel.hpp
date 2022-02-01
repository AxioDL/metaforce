#pragma once

#include <memory>
#include <vector>

#include "GCNTypes.hpp"
#include "IObjectStore.hpp"
#include "CTexture.hpp"
#include "CToken.hpp"

namespace metaforce {
class CCubeSurface;
class CCubeModel {
public:
  void UnlockTextures() {}

  static void MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, bool b1) {}
};

class CModel {
public:
  struct SShader {
    std::vector<TCachedToken<CTexture>> x0_textures;
    const u8* x10_data;

    SShader(const u8* data) : x10_data(data) {}

    void UnlockTextures() {};
  };

private:
  static u32 sFrameCounter;
  static bool sIsTextureTimeoutEnabled;
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;

  std::unique_ptr<u8[]> x0_data;
  u32 x4_dataLen;
  std::vector<CCubeSurface*> x8_surfaces;
  std::vector<SShader> x18_matSets;
  CCubeModel* x28_modelInst = nullptr;
  u16 x2c_ = 0;
  u16 x2e_ = 0;
  CModel* x30_prev = nullptr;
  CModel* x34_next;
  u32 x38_lastFrame;

public:
  CModel(std::unique_ptr<u8[]> in, u32 dataLen, IObjectStore* store);

  void UpdateLastFrame();
  void MoveToThisFrameList();
  void RemoveFromList();
  void VerifyCurrentShader(s32 idx) {};
  static void FrameDone();
  static void EnableTextureTimeout();
  static void DisableTextureTimeout();
};
} // namespace metaforce
