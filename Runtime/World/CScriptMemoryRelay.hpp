#ifndef CSCRIPTMEMORYRELAY_HPP
#define CSCRIPTMEMORYRELAY_HPP

#include "CEntity.hpp"

namespace urde
{
class CScriptMemoryRelay : public CEntity
{
    union
    {
        struct
        {
            bool x34_24_;
            bool x34_25_;
            bool x34_26_;
        };
        u8 dummy = 0;
    };
public:
    CScriptMemoryRelay(TUniqueId, const std::string& name, const CEntityInfo&, bool, bool, bool);
};
}

#endif // CSCRIPTMEMORYRELAY_HPP
