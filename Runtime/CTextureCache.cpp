#include "Runtime/CTextureCache.hpp"
#include "Runtime/CToken.hpp"

namespace urde {
CTextureCache::CTextureCache(CInputStream& in) {
  u32 textureCount = in.readUint32Big();
  for (u32 i = 0; i < textureCount; ++i) {
    CAssetId uid(in);
    if (m_textureInfo.find(uid) == m_textureInfo.end())
      m_textureInfo.emplace(uid, CTextureInfo(in));
  }
}

const CTextureInfo* CTextureCache::GetTextureInfo(CAssetId id) const {
  auto it = m_textureInfo.find(id);
  if (it == m_textureInfo.end())
    return nullptr;
  return &it->second;
}

CFactoryFnReturn FTextureCacheFactory(const SObjectTag& tag, CInputStream& in,
                                      const CVParamTransfer& vparms, CObjectReference* selfRef) {
  return TToken<CTextureCache>::GetIObjObjectFor(std::make_unique<CTextureCache>(in));
}
}