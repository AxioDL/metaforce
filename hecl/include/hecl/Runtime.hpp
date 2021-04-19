#pragma once

#include <cstddef>
#include <memory>

#include "hecl/SystemChar.hpp"

namespace hecl {
struct HMDLMeta;

namespace Runtime {

/**
 * @brief Per-platform file store resolution
 */
class FileStoreManager {
  SystemString m_domain;
  SystemString m_storeRoot;

public:
  FileStoreManager(SystemStringView domain);
  SystemStringView getDomain() const { return m_domain; }
  /**
   * @brief Returns the full path to the file store, including domain
   * @return Full path to store e.g /home/foo/.hecl/bar
   */
  SystemStringView getStoreRoot() const { return m_storeRoot; }
};

} // namespace Runtime
} // namespace hecl
