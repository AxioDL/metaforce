#ifndef CSCRIPTHUDMEMO_HPP
#define CSCRIPTHUDMEMO_HPP

#include "CEntity.hpp"
#include "CHUDMemoParms.hpp"

namespace urde
{
class CScriptHUDMemo : public CEntity
{
public:
    enum class EDisplayType
    {
        StatusMessage,
        MessageBox,
    };

private:
public:
    CScriptHUDMemo(TUniqueId, const std::string&, const CEntityInfo&, const CHUDMemoParms&,
                   CScriptHUDMemo::EDisplayType, ResId, bool);
};
}

#endif // CSCRIPTHUDMEMO_HPP
