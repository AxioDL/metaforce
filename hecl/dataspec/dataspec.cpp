#include "HECLDatabase.hpp"

namespace HECL
{
namespace Database
{

IDataSpec* NewDataSpec_little();
IDataSpec* NewDataSpec_big();
IDataSpec* NewDataSpec_revolution();
IDataSpec* NewDataSpec_cafe();

IDataSpec* NewDataSpec_mp1();
IDataSpec* NewDataSpec_mp2();
IDataSpec* NewDataSpec_mp3();

const DataSpecEntry DATA_SPEC_REGISTRY[] =
{
    {_S("hecl-little"), _S("Targets little-endian PC apps using the HECL runtime"), NewDataSpec_little},
    {_S("hecl-big"), _S("Targets big-endian PC apps using the HECL runtime"), NewDataSpec_big},
    {_S("hecl-revolution"), _S("Targets Wii apps using the HECL runtime"), NewDataSpec_revolution},
    {_S("hecl-cafe"), _S("Targets Wii U apps using the HECL runtime"), NewDataSpec_cafe},
    {_S("mp1"), _S("Targets original Metroid Prime engine"), NewDataSpec_mp1},
    {_S("mp2"), _S("Targets original Metroid Prime 2 engine"), NewDataSpec_mp2},
    {_S("mp3"), _S("Targets original Metroid Prime 3 engine"), NewDataSpec_mp3},
    {}
};

}
}
