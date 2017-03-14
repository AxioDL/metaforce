#ifndef HECLBACKEND_HPP
#define HECLBACKEND_HPP

#include "hecl/Frontend.hpp"

namespace hecl
{
namespace Backend
{

using IR = Frontend::IR;
using Diagnostics = Frontend::Diagnostics;
using SourceLocation = Frontend::SourceLocation;
using ArithmeticOp = IR::Instruction::ArithmeticOpType;

enum class TexGenSrc : uint8_t
{
    Position,
    Normal,
    UV
};

enum class BlendFactor : uint8_t
{
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    DstColor,
    InvDstColor,
    SrcAlpha,
    InvSrcAlpha,
    DstAlpha,
    InvDstAlpha,
    SrcColor1,
    InvSrcColor1,
    Original = 0xff
};

enum class ZTest : uint8_t
{
    None,
    LEqual,
    Greater,
    Equal,
    Original = 0xff
};

struct TextureInfo
{
    TexGenSrc src;
    int mapIdx;
    int uvIdx;
    int mtxIdx;
    bool normalize;
};

class IBackend
{
public:
    virtual void reset(const IR& ir, Diagnostics& diag)=0;
};

}
}

#endif // HECLBACKEND_HPP
