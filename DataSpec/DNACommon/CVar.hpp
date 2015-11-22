#ifndef _DNACOMMON_CVAR_HPP_
#define _DNACOMMON_CVAR_HPP_

#include <Athena/Global.hpp>
#include <type_traits>
#include "DNACommon.hpp"

namespace Retro
{
namespace DNACVAR
{
enum class EType
{
    Boolean,
    Integer,
    Float,
    Literal,
    Color,
    Bind
};

enum EFlags
{
    All      = -1, // NOTE: is this really necessary? It seems rather overkill
    System   = (1 << 0),
    Game     = (1 << 1),
    Gui      = (1 << 2),
    Cheat    = (1 << 3),
    Hidden   = (1 << 4),
    ReadOnly = (1 << 5),
    Archive  = (1 << 6),
    Modified = (1 << 7)
};
ENABLE_BITWISE_ENUM(EFlags)

class CVar : BigYAML
{
public:
    DECL_YAML
    String<-1>    m_name;
    String<-1>    m_value;
    Value<EType>  m_type;
    Value<EFlags> m_flags;
};

}
}

#endif
