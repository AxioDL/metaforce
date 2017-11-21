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

void IR::Instruction::read(athena::io::IStreamReader& reader)
{
    m_op = OpType(reader.readUByte());
    m_target = reader.readUint16Big();
    switch (m_op)
    {
    default: break;
    case OpType::Call:
        m_call.read(reader);
        break;
    case OpType::LoadImm:
        m_loadImm.read(reader);
        break;
    case OpType::Arithmetic:
        m_arithmetic.read(reader);
        break;
    case OpType::Swizzle:
        m_swizzle.read(reader);
        break;
    }
}

void IR::Instruction::write(athena::io::IStreamWriter& writer) const
{
    writer.writeUByte(m_op);
    writer.writeUint16Big(m_target);
    switch (m_op)
    {
    default: break;
    case OpType::Call:
        m_call.write(writer);
        break;
    case OpType::LoadImm:
        m_loadImm.write(writer);
        break;
    case OpType::Arithmetic:
        m_arithmetic.write(writer);
        break;
    case OpType::Swizzle:
        m_swizzle.write(writer);
        break;
    }
}

size_t IR::Instruction::binarySize(size_t sz) const
{
    sz += 3;
    switch (m_op)
    {
    default: break;
    case OpType::Call:
        sz = m_call.binarySize(sz);
        break;
    case OpType::LoadImm:
        sz = m_loadImm.binarySize(sz);
        break;
    case OpType::Arithmetic:
        sz = m_arithmetic.binarySize(sz);
        break;
    case OpType::Swizzle:
        sz = m_swizzle.binarySize(sz);
        break;
    }
    return sz;
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
            for (i=0 ; i<3 ; ++i, ++it)
            {
                if (it->kind != IRNode::Kind::Imm)
                    break;
                vec.vec[i] = it->val;
            }
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
            for (i=0 ; i<4 ; ++i, ++it)
            {
                if (it->kind != IRNode::Kind::Imm)
                    break;
                vec.vec[i] = it->val;
            }
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
        inst.m_immVec.vec[0] = n.val;
        inst.m_immVec.vec[1] = n.val;
        inst.m_immVec.vec[2] = n.val;
        inst.m_immVec.vec[3] = n.val;
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

void IR::read(athena::io::IStreamReader& reader)
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

void IR::write(athena::io::IStreamWriter& writer) const
{
    writer.writeUint64Big(m_hash);
    writer.writeUint16Big(m_regCount);
    writer.writeUint16Big(m_instructions.size());
    for (const Instruction& inst : m_instructions)
        inst.write(writer);
}

size_t IR::binarySize(size_t sz) const
{
    sz += 12;
    for (const Instruction& inst : m_instructions)
        sz = inst.binarySize(sz);
    return sz;
}

IR Frontend::compileSource(std::string_view source, std::string_view diagName)
{
    Hash hash(source);
    m_diag.reset(diagName, source);
    m_parser.reset(source);
    auto insts = m_parser.parse();
    IR ir;
    ir.m_hash = hash.val64();
    for (auto& inst : insts)
        ir.addInstruction(inst, 0);
    return ir;
}

}
