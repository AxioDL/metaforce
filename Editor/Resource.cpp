#include "Resource.hpp"

namespace urde
{

Space::Class Resource::DeduceDefaultSpaceClass(const hecl::ProjectPath& path)
{
    athena::io::FileReader r(path.getAbsolutePath(), 32*1024, false);
    if (r.hasError())
        return Space::Class::None;
    return Space::Class::None;
}

}
