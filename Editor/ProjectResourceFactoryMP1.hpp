#pragma once

#include "ProjectResourceFactoryBase.hpp"
#include "CToken.hpp"

namespace urde {
class MP1OriginalIDs;
class CSimplePool;

class ProjectResourceFactoryMP1 : public ProjectResourceFactoryBase {
public:
  ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc);
  void IndexMP1Resources(hecl::Database::Project& proj, CSimplePool& sp);
};

} // namespace urde
