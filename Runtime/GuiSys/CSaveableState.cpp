#include "CSaveableState.hpp"
#include "CRasterFont.hpp"

namespace urde
{

bool CSaveableState::IsFinishedLoading() const
{
    if (!x48_font || !x48_font.IsLoaded())
        return false;
    return x48_font->IsFinishedLoading();
}

}
