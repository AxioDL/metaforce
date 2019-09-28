#pragma once

#include "Editor/ProjectResourceFactoryBase.hpp"
#include "Runtime/CToken.hpp"

namespace urde {
class MP1OriginalIDs;
class CSimplePool;

class ProjectResourceFactoryMP1 : public ProjectResourceFactoryBase {
  TLockedToken<MP1OriginalIDs> m_origIds;

public:
  ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc);
  void IndexMP1Resources(hecl::Database::Project& proj, CSimplePool& sp);
  void Shutdown();

  CAssetId TranslateOriginalToNew(CAssetId id) const override;
  CAssetId TranslateNewToOriginal(CAssetId id) const override;
};

} // namespace urde
