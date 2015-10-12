#ifndef HECLBACKEND_HPP
#define HECLBACKEND_HPP

namespace HECL
{
namespace Frontend {struct IR;}
namespace Backend
{

class IBackend
{
public:
    virtual void reset(const Frontend::IR& ir)=0;
};

}
}

#endif // HECLBACKEND_HPP
