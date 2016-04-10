#ifndef __PSHAG_CMETAANIMFACTORY_HPP__
#define __PSHAG_CMETAANIMFACTORY_HPP__

#include "IOStreams.hpp"
#include "IMetaAnim.hpp"

namespace urde
{

class CMetaAnimFactory
{
public:
    static std::shared_ptr<IMetaAnim> CreateMetaAnim(CInputStream& in);
};

}

#endif // __PSHAG_CMETAANIMFACTORY_HPP__
