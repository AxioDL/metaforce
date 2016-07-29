#include "CTweakAutoMapper.hpp"
namespace urde
{
namespace MP1
{
CTweakAutoMapper::CTweakAutoMapper(CInputStream& in)
{
    x4_24_ = in.readBool();
    x4_25_ = in.readBool();
    x4_26_ = in.readBool();
    x8_ = in.readFloatBig();
    xc_ = in.readFloatBig();
    x10_ = in.readFloatBig();
    x14_ = in.readFloatBig();
    x18_ = in.readFloatBig();
    x1c_ = in.readFloatBig();
    x20_ = in.readFloatBig();
    x24_.readRGBABig(in);
    x28_ = in.readFloatBig();
    x2c_ = in.readFloatBig();
    x30_ = in.readFloatBig();
    x34_ = in.readFloatBig();
    x38_.readRGBABig(in);
    x3c_.readRGBABig(in);
    x40_.readRGBABig(in);
    x44_.readRGBABig(in);
    x48_.readRGBABig(in);
    x4c_.readRGBABig(in);
    x50_.readRGBABig(in);
    x54_ = in.readFloatBig();
    x58_ = in.readFloatBig();
    x5c_ = in.readFloatBig();
    x64_ = in.readFloatBig();
    x68_ = in.readFloatBig();
    x6c_ = in.readFloatBig();
    x70_ = in.readFloatBig();
    x74_ = in.readFloatBig();
    x78_ = in.readFloatBig();
    x7c_.readRGBABig(in);
    x80_.readRGBABig(in);
    x84_ = in.readFloatBig();
    x88_ = in.readFloatBig();
    x8c_ = in.readFloatBig();
    x90_ = in.readFloatBig();
    x94_ = in.readFloatBig();
    x98_ = in.readFloatBig();
    x9c_ = in.readFloatBig();
    xa0_ = in.readFloatBig();
    /* Originally 4 separate floats */
    xa4_doorCenter.readBig(in);
    xb0_ = in.readFloatBig();
    xb4_ = in.readFloatBig();
    xb8_ = in.readFloatBig();
    xbc_ = in.readFloatBig();
    xc0_ = in.readFloatBig();
    xc4_ = in.readFloatBig();
    xc8_ = in.readFloatBig();
    xcc_ = in.readBool();
    xd0_ = in.readFloatBig();
    xd4_ = in.readFloatBig();
    xd8_ = in.readFloatBig();
    xdc_ = in.readFloatBig();
    xe0_ = in.readFloatBig();
    xe4_ = in.readFloatBig();
    xe8_ = in.readFloatBig();
    xec_ = in.readFloatBig();
    xf0_.readRGBABig(in);
    xf4_.readRGBABig(in);
    xf8_.readRGBABig(in);
    xfc_.readRGBABig(in);
    x100_doorColorCount = in.readUint32Big();
    for (u32 i = 0 ; i<x100_doorColorCount ; ++i)
    {
        x104_doorColors.emplace_back();
        x104_doorColors.back().readRGBABig(in);
    }
    x118_doorBorderColor.readRGBABig(in);
    x11c_openDoorColor.readRGBABig(in);
}
}
}
