#include "CMetaAnimFactory.hpp"
#include "CMetaAnimPlay.hpp"
#include "CMetaAnimBlend.hpp"
#include "CMetaAnimPhaseBlend.hpp"
#include "CMetaAnimRandom.hpp"
#include "CMetaAnimSequence.hpp"

namespace urde
{

std::shared_ptr<IMetaAnim> CMetaAnimFactory::CreateMetaAnim(CInputStream& in)
{
    EMetaAnimType type = EMetaAnimType(in.readUint32Big());

    switch (type)
    {
    case EMetaAnimType::Primitive:
        return std::make_shared<CMetaAnimPlay>(in);
    case EMetaAnimType::Blend:
        return std::make_shared<CMetaAnimBlend>(in);
    case EMetaAnimType::PhaseBlend:
        return std::make_shared<CMetaAnimPhaseBlend>(in);
    case EMetaAnimType::Random:
        return std::make_shared<CMetaAnimRandom>(in);
    case EMetaAnimType::Sequence:
        return std::make_shared<CMetaAnimSequence>(in);
    default: break;
    }

    return {};
}

}
