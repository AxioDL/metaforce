#ifndef __URDE_CPLAYMOVIE_HPP__
#define __URDE_CPLAYMOVIE_HPP__

#include "CPlayMovieBase.hpp"

namespace urde
{
namespace MP1
{

extern const char* kMovies[];

class CPlayMovie : public CPlayMovieBase
{
public:
    enum class EWhichMovie
    {
        WinGame,
        LoseGame,
        Two,
        AfterCredits
    };
private:
    EWhichMovie x14_which;
public:
    CPlayMovie(EWhichMovie which) : CPlayMovieBase("CPlayMovie", kMovies[int(which)]), x14_which(which) {}
};

}
}

#endif // __URDE_CPLAYMOVIE_HPP__
