#include "HECLDatabase.hpp"

#include "CMDL.hpp"
#include "MREA.hpp"

namespace HECL
{
namespace Database
{

class MP2DataSpec : public IDataSpec
{

};

IDataSpec* NewDataSpec_mp2()
{
    return new MP2DataSpec();
}

}
}
