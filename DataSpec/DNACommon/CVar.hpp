#ifndef _DNACOMMON_CVAR_HPP_
#define _DNACOMMON_CVAR_HPP_

#include <Athena/Global.hpp>
#include "DNACommon.hpp"

namespace Retro
{
namespace DNACVAR
{
enum class EType : atUint8
{
    Boolean,
    Integer,
    Float,
    Literal,
    Color
};

enum EFlags
{
    All      = -1, // NOTE: is this really necessary? It seems rather overkill
    System   = (1 << 0),
    Game     = (1 << 1),
    Editor   = (1 << 2),
    Gui      = (1 << 3),
    Cheat    = (1 << 4),
    Hidden   = (1 << 5),
    ReadOnly = (1 << 6),
    Archive  = (1 << 7),
    Modified = (1 << 8)
};
ENABLE_BITWISE_ENUM(EFlags)

class CVar : BigYAML
{
public:
    DECL_YAML
    String<-1>    m_name;
    String<-1>    m_value;
    Value<EType>  m_type;
};

struct CVarContainer : BigYAML
{
    DECL_YAML
    Value<atUint32> magic = 'CVAR';
    Value<atUint32> cvarCount;
    Vector<CVar, DNA_COUNT(cvarCount)> cvars;
};

}
}

#endif
