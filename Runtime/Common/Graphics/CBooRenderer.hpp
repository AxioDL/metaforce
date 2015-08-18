#ifndef __RETRO_CBOORENDERER_HPP__
#define __RETRO_CBOORENDERER_HPP__

#include "../CFactoryStore.hpp"
#include "../COsContext.hpp"
#include "../CMemory.hpp"
#include "../CResFactory.hpp"

namespace Retro
{
namespace Common
{

class CBooRenderer
{
public:
    CBooRenderer(IObjectStore&, COsContext&, CMemorySys&, CResFactory&)
    {
    }
};

}
}

#endif // __RETRO_CBOORENDERER_HPP__
