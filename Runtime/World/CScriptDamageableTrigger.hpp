#ifndef CSCRIPTDAMAGEABLETRIGGER_HPP
#define CSCRIPTDAMAGEABLETRIGGER_HPP

#include "CActor.hpp"

namespace urde
{
class CVisorParameters;
class CScriptDamageableTrigger : public CActor
{
public:
    enum class ECanOrbit
    {
        NoOrbit,
        Orbit,
    };

    zeus::CAABox x14c_bounds;
public:
    CScriptDamageableTrigger(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                             const CHealthInfo&, const CDamageVulnerability&, u32, CAssetId, CAssetId, CAssetId, ECanOrbit, bool,
                             const CVisorParameters&);

    void Accept(IVisitor& visitor);
};
}

#endif // CSCRIPTDAMAGEABLETRIGGER_HPP
