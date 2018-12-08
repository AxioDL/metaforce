#include "hecl/Frontend.hpp"

namespace hecl::Frontend
{

int IR::Instruction::getChildCount() const
{
    switch (m_op)
    {
    case OpType::Call:
        return m_call.m_argInstIdxs.size();
    case OpType::Arithmetic:
        return 2;
    case OpType::Swizzle:
        return 1;
    default:
        LogModule.report(logvisor::Fatal, "invalid op type");
    }
    return -1;
}

const IR::Instruction& IR::Instruction::getChildInst(const IR& ir, size_t idx) const
{
    switch (m_op)
    {
    case OpType::Call:
        return ir.m_instructions.at(m_call.m_argInstIdxs.at(idx));
    case OpType::Arithmetic:
        if (idx > 1)
            LogModule.report(logvisor::Fatal, "arithmetic child idx must be 0 or 1");
        return ir.m_instructions.at(m_arithmetic.m_instIdxs[idx]);
    case OpType::Swizzle:
        if (idx > 0)
            LogModule.report(logvisor::Fatal, "swizzle child idx must be 0");
        return ir.m_instructions.at(m_swizzle.m_instIdx);
    default:
        LogModule.report(logvisor::Fatal, "invalid op type");
    }
    return *this;
}

const atVec4f& IR::Instruction::getImmVec() const
{
    if (m_op != OpType::LoadImm)
        LogModule.report(logvisor::Fatal, "invalid op type");
    return m_loadImm.m_immVec;
}

template <class Op>
void IR::Instruction::Enumerate(typename Op::StreamT& s)
{
    Do<Op>({"op"}, m_op, s);
    Do<Op>({"target"}, m_target, s);
    switch (m_op)
    {
    default: break;
    case OpType::Call:
        Do<Op>({"call"}, m_call, s);
        break;
    case OpType::LoadImm:
        Do<Op>({"loadImm"}, m_loadImm, s);
        break;
    case OpType::Arithmetic:
        Do<Op>({"arithmetic"}, m_arithmetic, s);
        break;
    case OpType::Swizzle:
        Do<Op>({"swizzle"}, m_swizzle, s);
        break;
    }
}

atInt8 IR::swizzleCompIdx(char aChar)
{
    switch (aChar)
    {
    case 'x':
    case 'r':
        return 0;
    case 'y':
    case 'g':
        return 1;
    case 'z':
    case 'b':
        return 2;
    case 'w':
    case 'a':
        return 3;
    default:
        break;
    }
    return -1;
}

int IR::addInstruction(const IRNode& n, IR::RegID target)
{
    if (n.kind == IRNode::Kind::None)
        return -1;
    switch (n.kind)
    {
    case IRNode::Kind::Call:
    {
        if (!n.str.compare("vec3") && n.children.size() >= 3)
        {
            atVec4f vec = {};
            auto it = n.children.cbegin();
            int i;
            athena::simd_floats f;
            for (i=0 ; i<3 ; ++i, ++it)
            {
                if (it->kind != IRNode::Kind::Imm)
                    break;
                f[i] = it->val;
            }
            vec.simd.copy_from(f);
            if (i == 3)
            {
                m_instructions.emplace_back(OpType::LoadImm, target, n.loc);
                Instruction::LoadImm& inst = m_instructions.back().m_loadImm;
                inst.m_immVec = vec;
                return m_instructions.size() - 1;
            }
        }
        else if (!n.str.compare("vec4") && n.children.size() >= 4)
        {
            atVec4f vec = {};
            auto it = n.children.cbegin();
            int i;
            athena::simd_floats f;
            for (i=0 ; i<4 ; ++i, ++it)
            {
                if (it->kind != IRNode::Kind::Imm)
                    break;
                f[i] = it->val;
            }
            vec.simd.copy_from(f);
            if (i == 4)
            {
                m_instructions.emplace_back(OpType::LoadImm, target, n.loc);
                Instruction::LoadImm& inst = m_instructions.back().m_loadImm;
                inst.m_immVec = vec;
                return m_instructions.size() - 1;
            }
        }

        std::vector<atUint16> argInstIdxs;
        argInstIdxs.reserve(n.children.size());
        IR::RegID tgt = target;
        for (auto& c : n.children)
            argInstIdxs.push_back(addInstruction(c, tgt++));
        m_instructions.emplace_back(OpType::Call, target, n.loc);
        Instruction::Call& inst = m_instructions.back().m_call;
        inst.m_name = n.str;
        inst.m_argInstCount = atUint16(argInstIdxs.size());
        inst.m_argInstIdxs = argInstIdxs;
        return m_instructions.size() - 1;
    }
    case IRNode::Kind::Imm:
    {
        m_instructions.emplace_back(OpType::LoadImm, target, n.loc);
        Instruction::LoadImm& inst = m_instructions.back().m_loadImm;
        inst.m_immVec.simd = athena::simd<float>(n.val);
        return m_instructions.size() - 1;
    }
    case IRNode::Kind::Binop:
    {
        atUint16 left = addInstruction(*n.left, target);
        atUint16 right = addInstruction(*n.right, target + 1);
        m_instructions.emplace_back(OpType::Arithmetic, target, n.loc);
        Instruction::Arithmetic& inst = m_instructions.back().m_arithmetic;
        inst.m_op = Instruction::ArithmeticOpType(int(n.op) + 1);
        inst.m_instIdxs[0] = left;
        inst.m_instIdxs[1] = right;
        return m_instructions.size() - 1;
    }
    case IRNode::Kind::Swizzle:
    {
        atUint16 left = addInstruction(*n.left, target);
        m_instructions.emplace_back(OpType::Swizzle, target, n.loc);
        Instruction::Swizzle& inst = m_instructions.back().m_swizzle;
        for (int i=0 ; i<n.str.size() && i<4 ; ++i)
            inst.m_idxs[i] = swizzleCompIdx(n.str[i]);
        inst.m_instIdx = left;
        return m_instructions.size() - 1;
    }
    default:
        return -1;
    }
}

template <>
void IR::Enumerate<BigDNA::Read>(typename Read::StreamT& reader)
{
    m_hash = reader.readUint64Big();
    m_regCount = reader.readUint16Big();
    atUint16 instCount = reader.readUint16Big();
    m_instructions.clear();
    m_instructions.reserve(instCount);
    for (atUint16 i=0 ; i<instCount ; ++i)
        m_instructions.emplace_back(reader);

    /* Pre-resolve blending mode */
    const IR::Instruction& rootCall = m_instructions.back();
    m_doAlpha = false;
    if (!rootCall.m_call.m_name.compare("HECLOpaque"))
    {
        m_blendSrc = boo::BlendFactor::One;
        m_blendDst = boo::BlendFactor::Zero;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAlpha"))
    {
        m_blendSrc = boo::BlendFactor::SrcAlpha;
        m_blendDst = boo::BlendFactor::InvSrcAlpha;
        m_doAlpha = true;
    }
    else if (!rootCall.m_call.m_name.compare("HECLAdditive"))
    {
        m_blendSrc = boo::BlendFactor::SrcAlpha;
        m_blendDst = boo::BlendFactor::One;
        m_doAlpha = true;
    }
}

template <>
void IR::Enumerate<BigDNA::Write>(typename Write::StreamT& writer)
{
    writer.writeUint64Big(m_hash);
    writer.writeUint16Big(m_regCount);
    writer.writeUint16Big(m_instructions.size());
    for (const Instruction& inst : m_instructions)
        inst.write(writer);
}

template <>
void IR::Enumerate<BigDNA::BinarySize>(typename BinarySize::StreamT& sz)
{
    sz += 12;
    for (const Instruction& inst : m_instructions)
        inst.binarySize(sz);
}

IR Frontend::compileSource(std::string_view source, std::string_view diagName)
{
    m_diag.reset(diagName, source);
    m_parser.reset(source);
    auto insts = m_parser.parse();
    IR ir;
    std::string stripString;
    if (!insts.empty())
        stripString = insts.front().toString(true);
    ir.m_hash = Hash(stripString).val64();
    for (auto& inst : insts)
        ir.addInstruction(inst, 0);
    return ir;
}

}
