#pragma once

#include "Editor/ProjectResourceFactoryBase.hpp"
#include "Runtime/CToken.hpp"

namespace metaforce {
class MP1OriginalIDs;
class CSimplePool;

class ProjectResourceFactoryMP1 : public ProjectResourceFactoryBase {
public:
  ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc);
  void IndexMP1Resources(hecl::Database::Project& proj, CSimplePool& sp);
};

} // namespace metaforce
