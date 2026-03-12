#pragma once
#include "RetroTypes.hpp"

#include <unordered_map>

namespace metaforce {
class FileStoreManager;
class CResourceNameDatabase {
public:
  explicit CResourceNameDatabase(FileStoreManager& store);

  bool hasAssetName(const CAssetId asset) const { return m_assetNames.contains(asset); }

  const std::string* assetName(const CAssetId asset) const {
    if (!hasAssetName(asset)) {
      return nullptr;
    }

    return &m_assetNames.at(asset);
  }

  static CResourceNameDatabase* instance() { return m_instance; }

private:
  std::unordered_map<CAssetId, std::string> m_assetNames;

  static CResourceNameDatabase* m_instance;
};
} // namespace metaforce