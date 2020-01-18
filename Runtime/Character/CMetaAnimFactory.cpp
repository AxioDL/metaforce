#include "Runtime/Character/CMetaAnimFactory.hpp"

#include "Runtime/Character/CMetaAnimBlend.hpp"
#include "Runtime/Character/CMetaAnimPhaseBlend.hpp"
#include "Runtime/Character/CMetaAnimPlay.hpp"
#include "Runtime/Character/CMetaAnimRandom.hpp"
#include "Runtime/Character/CMetaAnimSequence.hpp"

namespace urde {

std::shared_ptr<IMetaAnim> CMetaAnimFactory::CreateMetaAnim(CInputStream& in) {
  EMetaAnimType type = EMetaAnimType(in.readUint32Big());

  switch (type) {
  case EMetaAnimType::Play:
    return std::make_shared<CMetaAnimPlay>(in);
  case EMetaAnimType::Blend:
    return std::make_shared<CMetaAnimBlend>(in);
  case EMetaAnimType::PhaseBlend:
    return std::make_shared<CMetaAnimPhaseBlend>(in);
  case EMetaAnimType::Random:
    return std::make_shared<CMetaAnimRandom>(in);
  case EMetaAnimType::Sequence:
    return std::make_shared<CMetaAnimSequence>(in);
  default:
    break;
  }

  return {};
}

} // namespace urde
