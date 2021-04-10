#pragma once
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Graphics/CTexture.hpp"
namespace metaforce {
class CPaletteInfo {
  u32 m_format;
  u32 m_elementCount;
  u64 m_dolphinHash;

public:
  explicit CPaletteInfo(CInputStream& in)
      : m_format(in.readUint32Big()), m_elementCount(in.readUint32Big()), m_dolphinHash(in.readUint64Big()) {}
};
class CTextureInfo {
  ETexelFormat m_format;
  u32 m_mipCount;
  u16 m_width;
  u16 m_height;
  u64 m_dolphinHash;
  std::optional<CPaletteInfo> m_paletteInfo;

public:
  explicit CTextureInfo(CInputStream& in)
      : m_format(ETexelFormat(in.readUint32Big()))
      , m_mipCount(in.readUint32Big())
      , m_width(in.readUint16Big())
      , m_height(in.readUint16Big())
      , m_dolphinHash(in.readUint64Big()) {
    bool hasPal = in.readBool();
    if (hasPal)
      m_paletteInfo.emplace(in);
  }
};
class CTextureCache {
public:
  std::map<CAssetId, CTextureInfo> m_textureInfo;

public:
  explicit CTextureCache(CInputStream& in);


  const CTextureInfo* GetTextureInfo(CAssetId id) const;
};

CFactoryFnReturn FTextureCacheFactory(const metaforce::SObjectTag& tag, CInputStream& in,
                                      const metaforce::CVParamTransfer& vparms, CObjectReference* selfRef);
}