#ifndef __PSHAG_CPLAYMOVIEBASE_HPP__
#define __PSHAG_CPLAYMOVIEBASE_HPP__

#include "CIOWin.hpp"
#include "CMoviePlayer.hpp"

namespace pshag
{

class CPlayMovieBase : public CIOWin
{
    CMoviePlayer x18_moviePlayer;
public:
    CPlayMovieBase(const char* iowName, const char* path)
    : CIOWin(iowName), x18_moviePlayer(path, 0.0, false) {}
    EMessageReturn OnMessage(const CArchitectureMessage&, CArchitectureQueue&) {return EMessageReturn::Normal;}
    void Draw() const {}
};

}

#endif // __PSHAG_CPLAYMOVIEBASE_HPP__
