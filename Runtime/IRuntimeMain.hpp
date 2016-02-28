#ifndef IRUNTIMEMAIN_HPP
#define IRUNTIMEMAIN_HPP

namespace pshag
{
struct IRuntimeMain
{
    void init() = 0;
    int  proc() = 0;
    void stop() = 0;
};
}

#endif // IRUNTIMEMAIN_HPP
