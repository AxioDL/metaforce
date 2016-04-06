#include "Parameters.hpp"
#include "../ANCS.hpp"

namespace DataSpec
{
namespace DNAMP1
{

UniqueID32 AnimationParameters::getCINF(PAKRouter<PAKBridge>& pakRouter) const
{
    if (!animationCharacterSet)
        return UniqueID32();
    const nod::Node* node;
    const PAK::Entry* ancsEnt = pakRouter.lookupEntry(animationCharacterSet, &node);
    ANCS ancs(ancsEnt->id);
    {
        PAKEntryReadStream rs = ancsEnt->beginReadStream(*node);
        ancs.read(rs);
    }
    return ancs.characterSet.characters.at(character).cinf.getBaseId();
}

}
}
