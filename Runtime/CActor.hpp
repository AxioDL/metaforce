#ifndef __RETRO_CACTOR_HPP__
#define __RETRO_CACTOR_HPP__

#include "CEntity.hpp"

namespace Retro
{
class CTransform;
class CModelData;
class CMaterialList;
class CActorParameters;

class CActor : public CEntity
{
public:
    CActor(TUniqueId, bool, const std::string&, const CEntityInfo&,
           const CTransform&, const CModelData&, const CMaterialList&,
           const CActorParameters&, TUniqueId);
};

}

#endif // __RETRO_CACTOR_HPP__
