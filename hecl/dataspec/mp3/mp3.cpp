#include "HECLDatabase.hpp"

#include "CMDL.hpp"
#include "MREA.hpp"

namespace HECL
{
namespace Database
{

class MP3DataSpec : public IDataSpec
{

};

IDataSpec* NewDataSpec_mp3()
{
    return new MP3DataSpec();
}

}
}
