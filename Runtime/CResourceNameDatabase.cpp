#include "Runtime/CResourceNameDatabase.hpp"

#include "Runtime/CBasics.hpp"
#include "Runtime/ConsoleVariables/FileStoreManager.hpp"
#include "Runtime/Streams/CMemoryInStream.hpp"

namespace metaforce {
constexpr std::string_view kDatabaseName = "mp_resource_names_confirmed.bin";
CResourceNameDatabase* CResourceNameDatabase::m_instance = nullptr;

CResourceNameDatabase::CResourceNameDatabase(FileStoreManager& store) {
  m_instance = this;
  const std::string filename = std::string(store.getStoreRoot()) + "/" + std::string(kDatabaseName);
  if (!CBasics::IsFile(filename.c_str())) {
    return;
  }

  CBasics::Sstat st;
  if (CBasics::Stat(filename.c_str(), &st) != 0) {
    return;
  }

  std::unique_ptr<u8> const inBuf(new u8[st.st_size]);
  auto* file = fopen(filename.c_str(), "rb");
  (void)fread(inBuf.get(), 1, st.st_size, file);
  (void)fclose(file);
  CMemoryInStream mem(inBuf.get(), st.st_size, CMemoryInStream::EOwnerShip::NotOwned);

  u32 count = mem.Get<u32>();
  while ((count--) != 0u) {
    const CAssetId assetId = mem.Get<u32>();
    const auto name = mem.Get<std::string>();
    m_assetNames[assetId] = name;
  }
}
} // namespace metaforce