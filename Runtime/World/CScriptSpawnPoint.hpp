#ifndef __CSCRIPTSPAWNPOINT_HPP__
#define __CSCRIPTSPAWNPOINT_HPP__

#include "CEntity.hpp"
#include "zeus/CTransform.hpp"

namespace urde
{

class CScriptSpawnPoint : public CEntity
{
public:
    CScriptSpawnPoint(TUniqueId, const std::string& name, const CEntityInfo& info,
                      const zeus::CTransform& xf, const std::vector<u32>& itemCounts,
                      bool, bool, bool);
};

}

#endif // __CSCRIPTSPAWNPOINT_HPP__
