#include "HECLDatabase.hpp"

#include "CMDL.hpp"
#include "MREA.hpp"

namespace HECL
{
namespace Database
{

class MP1DataSpec : public IDataSpec
{

};

IDataSpec* NewDataSpec_mp1()
{
    return new MP1DataSpec();
}

}
}
