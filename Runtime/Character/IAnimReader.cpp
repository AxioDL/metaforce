#include "IAnimReader.hpp"
#include "CCharAnimTime.hpp"

namespace urde
{

SAdvancementResults IAnimReader::VGetAdvancementResults(const CCharAnimTime& a, const CCharAnimTime& b) const
{
    SAdvancementResults ret;
    ret.x0_animTime = a;
    return ret;
}

}
