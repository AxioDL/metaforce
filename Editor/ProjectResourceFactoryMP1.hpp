#ifndef URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP

#include "ProjectResourceFactoryBase.hpp"

namespace urde
{

class ProjectResourceFactoryMP1 : public ProjectResourceFactoryBase
{
public:
    ProjectResourceFactoryMP1(hecl::ClientProcess& clientProc);
    void IndexMP1Resources(hecl::Database::Project& proj);
    SObjectTag BuildTagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const;
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP
