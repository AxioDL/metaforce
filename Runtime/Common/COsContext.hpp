#ifndef __RETRO_COSCONTEXT_HPP__
#define __RETRO_COSCONTEXT_HPP__

namespace Retro
{
namespace Common
{

class COsContext
{
public:
    COsContext();
    int OpenWindow(const char* name, int x, int y, int w, int h);
};

}
}

#endif // __RETRO_COSCONTEXT_HPP__
