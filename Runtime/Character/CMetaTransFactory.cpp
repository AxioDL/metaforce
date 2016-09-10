#include "CMetaTransFactory.hpp"
#include "CMetaTransMetaAnim.hpp"
#include "CMetaTransTrans.hpp"
#include "CMetaTransPhaseTrans.hpp"
#include "CMetaTransSnap.hpp"

namespace urde
{

std::shared_ptr<IMetaTrans> CMetaTransFactory::CreateMetaTrans(CInputStream& in)
{
    EMetaTransType type = EMetaTransType(in.readUint32Big());

    switch (type)
    {
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
}
