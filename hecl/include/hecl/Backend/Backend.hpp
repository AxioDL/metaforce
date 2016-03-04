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

class IBackend
{
public:
    virtual void reset(const IR& ir, Diagnostics& diag)=0;
};

}
}

#endif // HECLBACKEND_HPP
