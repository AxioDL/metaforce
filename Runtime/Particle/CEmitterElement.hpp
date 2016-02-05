#ifndef __RETRO_CEMITTERELEMENT_HPP__
#define __RETRO_CEMITTERELEMENT_HPP__

#include "IElement.hpp"

namespace Retro
{

class CEmitterElement : public IElement
{
};

class CEESimpleEmitter : public CEmitterElement
{
};

class CVESphere : public CEmitterElement
{
};

class CVEAngularSphere : public CEmitterElement
{
};


}

#endif // __RETRO_CEMITTERELEMENT_HPP__
