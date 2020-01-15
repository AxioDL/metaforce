#include "Runtime/Character/CMetaTransFactory.hpp"

#include "Runtime/Character/CMetaTransMetaAnim.hpp"
#include "Runtime/Character/CMetaTransPhaseTrans.hpp"
#include "Runtime/Character/CMetaTransSnap.hpp"
#include "Runtime/Character/CMetaTransTrans.hpp"

namespace urde {

std::shared_ptr<IMetaTrans> CMetaTransFactory::CreateMetaTrans(CInputStream& in) {
  EMetaTransType type = EMetaTransType(in.readUint32Big());

  switch (type) {
  case EMetaTransType::MetaAnim:
    return std::make_shared<CMetaTransMetaAnim>(in);
  case EMetaTransType::Trans:
    return std::make_shared<CMetaTransTrans>(in);
  case EMetaTransType::PhaseTrans:
    return std::make_shared<CMetaTransPhaseTrans>(in);
  case EMetaTransType::Snap:
    return std::make_shared<CMetaTransSnap>();
  default:
    break;
  }

  return {};
}
} // namespace urde
