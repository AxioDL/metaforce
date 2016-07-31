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

enum class TexGenSrc
{
    Position,
    Normal,
    UV
};

struct TextureInfo
{
    TexGenSrc src;
    int uvIdx;
    int mtx;
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
