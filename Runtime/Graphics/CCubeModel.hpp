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

#pragma region CModel
class CModel {
public:
  struct SShader {
    std::vector<TCachedToken<CTexture>> x0_textures;
    const u8* x10_data;

    explicit SShader(const u8* data) : x10_data(data) {}

    void UnlockTextures(){};
  };

private:
  static u32 sTotalMemory;
  static u32 sFrameCounter;
  static bool sIsTextureTimeoutEnabled;
  static CModel* sThisFrameList;
  static CModel* sOneFrameList;
  static CModel* sTwoFrameList;

  std::unique_ptr<u8[]> x0_data;
  u32 x4_dataLen;
  std::vector<CCubeSurface*> x8_surfaces;
  std::vector<SShader> x18_matSets;
  std::unique_ptr<CCubeModel> x28_modelInst = nullptr;
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
  void VerifyCurrentShader(s32 idx){};
  static void FrameDone();
  static void EnableTextureTimeout();
  static void DisableTextureTimeout();
};

#pragma endregion

#pragma region CCubeModel
class CCubeModel {
public:
  CCubeModel(const std::vector<CCubeSurface*>* surfaces, const std::vector<TCachedToken<CTexture>>* textures,
             const u8* materialData, const u8* positions, const u8* normals, const u8* vtxColors, const u8* floatUvs,
             const u8* shortUVs, const zeus::CAABox* aabox, u8 flags, bool b1, u32 w1) {}

  void UnlockTextures();

  static void MakeTexturesFromMats(const u8* ptr, std::vector<TCachedToken<CTexture>>& texture, bool b1);
};
#pragma endregion

#pragma region CCubeSurface
class CCubeSurface {
public:
  enum class ECookie {

  };

public:
  bool IsValid() const;
  static CCubeSurface* FromCookieValue(u32);
  void GetCookie(ECookie cookie);
  void SetCookie(ECookie cookie, u32 value);
  u32 GetCookieValue() const;
  s32 GetMaterialIndex() const;
  s32 GetDisplayListSize() const;
  u8* GetDisplayList() const;
  u32 GetSurfaceHeaderSize() const;
  zeus::CVector3f GetCenter() const;
  u32 GetNormalHint() const;
  zeus::CAABox GetBounds() const;
};
#pragma endregion

} // namespace metaforce
