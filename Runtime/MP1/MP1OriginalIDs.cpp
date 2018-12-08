#include "MP1OriginalIDs.hpp"
#include "CToken.hpp"

namespace urde {

MP1OriginalIDs::MP1OriginalIDs(CInputStream& in) {
  u32 count = in.readUint32Big();
  m_origToNew.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    CAssetId a = in.readUint32Big();
    CAssetId b = in.readUint32Big();
    m_origToNew.push_back(std::make_pair(a, b));
  }
  m_newToOrig.reserve(count);
  for (u32 i = 0; i < count; ++i) {
    CAssetId a = in.readUint32Big();
    CAssetId b = in.readUint32Big();
    m_newToOrig.push_back(std::make_pair(a, b));
  }
}

CAssetId MP1OriginalIDs::TranslateOriginalToNew(CAssetId id) const {
  auto search =
      rstl::binary_find(m_origToNew.cbegin(), m_origToNew.cend(), id, [](const auto& id) { return id.first; });
  if (search == m_origToNew.cend())
    return {};
  return search->second;
}

CAssetId MP1OriginalIDs::TranslateNewToOriginal(CAssetId id) const {
  auto search =
      rstl::binary_find(m_newToOrig.cbegin(), m_newToOrig.cend(), id, [](const auto& id) { return id.first; });
  if (search == m_newToOrig.cend())
    return {};
  return search->second;
}

CFactoryFnReturn FMP1OriginalIDsFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& param,
                                        CObjectReference* selfRef) {
  return TToken<MP1OriginalIDs>::GetIObjObjectFor(std::make_unique<MP1OriginalIDs>(in));
}

} // namespace urde
