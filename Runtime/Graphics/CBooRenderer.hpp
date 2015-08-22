#ifndef __RETRO_CBOORENDERER_HPP__
#define __RETRO_CBOORENDERER_HPP__

namespace Retro
{
class IObjectStore;
class COsContext;
class CMemorySys;
class CResFactory;

class CBooRenderer
{
public:
    CBooRenderer(IObjectStore&, COsContext&, CMemorySys&, CResFactory&)
    {
    }
};

}

#endif // __RETRO_CBOORENDERER_HPP__
