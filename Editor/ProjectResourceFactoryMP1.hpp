#ifndef URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP

#include "ProjectResourceFactoryBase.hpp"

namespace urde
{

class ProjectResourceFactoryMP1 : public ProjectResourceFactoryBase
{
public:
    ProjectResourceFactoryMP1();
    void IndexMP1Resources(const hecl::Database::Project& proj);
    SObjectTag TagFromPath(const hecl::ProjectPath& path) const;
    hecl::ProjectPath GetCookedPath(const SObjectTag& tag,
                                    const hecl::ProjectPath& working,
                                    bool pcTarget) const;
    bool DoCook(const SObjectTag& tag, const hecl::ProjectPath& working,
                const hecl::ProjectPath& cooked,
                bool pcTarget);
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_MP1_HPP
