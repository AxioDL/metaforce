#ifndef _DNAMP1_CTWEAKGAME_HPP_
#define _DNAMP1_CTWEAKGAME_HPP_

#include "../../DNACommon/Tweaks/ITweakGame.hpp"

namespace Retro
{
namespace DNAMP1
{

struct CTweakGame : ITweakGame
{
    DECL_YAML
    String<-1> m_worldPrefix;
    String<-1> m_ruinsArea; // ????

    virtual const std::string& GetWorldPrefix() const { return m_worldPrefix; }
    CTweakGame(Athena::io::IStreamReader& in) { this->read(in); }
};
}
}

#endif
