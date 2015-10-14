#include "HECL/HECL.hpp"
#include "HECL/Frontend.hpp"

/* Combined lexer and semantic analysis system */

namespace HECL
{
namespace Frontend
{

static IR::Instruction::ArithmeticOpType ArithType(int aChar)
{
    switch (aChar)
    {
    case '+':
        return IR::Instruction::ArithmeticOpAdd;
    case '-':
        return IR::Instruction::ArithmeticOpSubtract;
    case '*':
        return IR::Instruction::ArithmeticOpMultiply;
    case '/':
        return IR::Instruction::ArithmeticOpDivide;
    default:
        return IR::Instruction::ArithmeticOpNone;
    }
}

void Lexer::ReconnectArithmetic(OperationNode* sn, OperationNode** lastSub, OperationNode** newSub) const
{
    sn->m_sub = sn->m_prev;
    sn->m_prev = nullptr;
    sn->m_sub->m_prev = nullptr;

    sn->m_sub->m_next = sn->m_next;
    sn->m_next = sn->m_next->m_next;
    sn->m_sub->m_next->m_prev = sn->m_sub;
    sn->m_sub->m_next->m_next = nullptr;

    if (*lastSub)
    {
        (*lastSub)->m_next = sn;
        sn->m_prev = *lastSub;
    }
    *lastSub = sn;

    if (!*newSub)
        *newSub = sn;
}

void Lexer::PrintChain(const Lexer::OperationNode* begin, const Lexer::OperationNode* end)
{
    for (const Lexer::OperationNode* n = begin ; n != end ; n = n->m_next)
    {
        printf("%3d %s %s\n", n->m_tok.m_location.col, n->m_tok.typeString(),
               n->m_tok.m_tokenString.c_str());
    }
}

void Lexer::PrintTree(const Lexer::OperationNode* node, int indent)
{
    for (const Lexer::OperationNode* n = node ; n ; n = n->m_next)
    {
        for (int i=0 ; i<indent ; ++i)
            printf("  ");
        printf("%3d %s %s\n", n->m_tok.m_location.col, n->m_tok.typeString(),
               n->m_tok.m_tokenString.c_str());
        if (n->m_sub)
            PrintTree(n->m_sub, indent + 1);
    }
}

void Lexer::reset()
{
    m_root = nullptr;
    m_pool.clear();
}

void Lexer::consumeAllTokens(Parser& parser)
{
    reset();
    Parser::Token firstTok = parser.consumeToken();
    if (firstTok.m_type != Parser::TokenSourceBegin)
    {
        m_diag.reportLexerErr(firstTok.m_location, "expected start token");
        return;
    }

    m_pool.emplace_front(std::move(firstTok));
    Lexer::OperationNode* firstNode = &m_pool.front();
    Lexer::OperationNode* lastNode = firstNode;

    /* Build linked-list of nodes parsed in-order */
    {
        std::vector<SourceLocation> funcStack;
        std::vector<SourceLocation> groupStack;
        while (lastNode->m_tok.m_type != Parser::TokenSourceEnd)
        {
            Parser::Token tok = parser.consumeToken();
            switch (tok.m_type)
            {
            case Parser::TokenEvalGroupStart:
                groupStack.push_back(tok.m_location);
                break;
            case Parser::TokenEvalGroupEnd:
                if (groupStack.empty())
                {
                    m_diag.reportLexerErr(tok.m_location, "unbalanced group detected");
                    return;
                }
                groupStack.pop_back();
                break;
            case Parser::TokenFunctionStart:
                funcStack.push_back(tok.m_location);
                break;
            case Parser::TokenFunctionEnd:
                if (funcStack.empty())
                {
                    m_diag.reportLexerErr(tok.m_location, "unbalanced function detected");
                    return;
                }
                funcStack.pop_back();
                break;
            case Parser::TokenSourceEnd:
            case Parser::TokenNumLiteral:
            case Parser::TokenVectorSwizzle:
            case Parser::TokenFunctionArgDelim:
            case Parser::TokenArithmeticOp:
                break;
            default:
                m_diag.reportLexerErr(tok.m_location, "invalid token");
                return;
            }
            m_pool.emplace_front(std::move(tok));
            lastNode->m_next = &m_pool.front();
            m_pool.front().m_prev = lastNode;
            lastNode = &m_pool.front();
        }

        /* Ensure functions and groups are balanced */
        if (funcStack.size())
        {
            m_diag.reportLexerErr(funcStack.back(), "unclosed function detected");
            return;
        }
        if (groupStack.size())
        {
            m_diag.reportLexerErr(groupStack.back(), "unclosed group detected");
            return;
        }
    }

    /* Ensure first non-start node is a function */
    if (firstNode->m_next->m_tok.m_type != Parser::TokenFunctionStart)
    {
        m_diag.reportLexerErr(firstNode->m_tok.m_location, "expected root function");
        return;
    }

    /* Organize marked function args into implicit groups */
    for (Lexer::OperationNode* n = firstNode ; n != lastNode ; n = n->m_next)
    {
        if (n->m_tok.m_type == Parser::TokenFunctionStart)
        {
            if (n->m_next->m_tok.m_type != Parser::TokenFunctionEnd)
            {
                if (n->m_next->m_tok.m_type == Parser::TokenFunctionArgDelim)
                {
                    m_diag.reportLexerErr(n->m_next->m_tok.m_location, "empty function arg");
                    return;
                }
                m_pool.emplace_front(std::move(
                Parser::Token(Parser::TokenEvalGroupStart, n->m_next->m_tok.m_location)));
                Lexer::OperationNode* grp = &m_pool.front();
                grp->m_next = n->m_next;
                grp->m_prev = n;
                n->m_next->m_prev = grp;
                n->m_next = grp;
            }
        }
        else if (n->m_tok.m_type == Parser::TokenFunctionEnd)
        {
            if (n->m_prev->m_tok.m_type != Parser::TokenFunctionStart)
            {
                m_pool.emplace_front(std::move(
                Parser::Token(Parser::TokenEvalGroupEnd, n->m_tok.m_location)));
                Lexer::OperationNode* grp = &m_pool.front();
                grp->m_next = n;
                grp->m_prev = n->m_prev;
                n->m_prev->m_next = grp;
                n->m_prev = grp;
            }
        }
        else if (n->m_tok.m_type == Parser::TokenFunctionArgDelim)
        {
            if (n->m_next->m_tok.m_type == Parser::TokenFunctionArgDelim ||
                n->m_next->m_tok.m_type == Parser::TokenFunctionEnd)
            {
                m_diag.reportLexerErr(n->m_next->m_tok.m_location, "empty function arg");
                return;
            }

            m_pool.emplace_front(std::move(
            Parser::Token(Parser::TokenEvalGroupEnd, n->m_tok.m_location)));
            Lexer::OperationNode* egrp = &m_pool.front();

            m_pool.emplace_front(std::move(
            Parser::Token(Parser::TokenEvalGroupStart, n->m_next->m_tok.m_location)));
            Lexer::OperationNode* sgrp = &m_pool.front();

            egrp->m_next = sgrp;
            sgrp->m_prev = egrp;

            sgrp->m_next = n->m_next;
            egrp->m_prev = n->m_prev;
            n->m_next->m_prev = sgrp;
            n->m_prev->m_next = egrp;
        }
    }

    /* Organize marked groups into tree-hierarchy */
    {
        std::vector<Lexer::OperationNode*> groupStack;
        for (Lexer::OperationNode* n = firstNode ; n != lastNode ; n = n->m_next)
        {
            if (n->m_tok.m_type == Parser::TokenEvalGroupStart)
                groupStack.push_back(n);
            else if (n->m_tok.m_type == Parser::TokenEvalGroupEnd)
            {
                Lexer::OperationNode* start = groupStack.back();
                groupStack.pop_back();
                if (n->m_prev == start)
                {
                    m_diag.reportLexerErr(start->m_tok.m_location, "empty group");
                    return;
                }
                start->m_sub = start->m_next;
                start->m_next = n->m_next;
                if (n->m_next)
                    n->m_next->m_prev = start;
                n->m_prev->m_next = nullptr;
            }
        }
    }

    /* Organize functions into tree-hierarchy */
    for (Lexer::OperationNode& n : m_pool)
    {
        if (n.m_tok.m_type == Parser::TokenFunctionStart)
        {
            for (Lexer::OperationNode* sn = n.m_next ; sn ; sn = sn->m_next)
            {
                if (sn->m_tok.m_type == Parser::TokenFunctionEnd)
                {
                    n.m_sub = n.m_next;
                    if (n.m_next == sn)
                        n.m_sub = nullptr;
                    n.m_next = sn->m_next;
                    if (sn->m_next)
                        sn->m_next->m_prev = &n;
                    if (n.m_sub)
                        n.m_sub->m_prev = nullptr;
                    if (sn->m_prev)
                        sn->m_prev->m_next = nullptr;
                    break;
                }
            }
        }
    }

    /* Organize vector swizzles into tree-hierarchy */
    for (Lexer::OperationNode& n : m_pool)
    {
        if (n.m_tok.m_type == Parser::TokenVectorSwizzle)
        {
            if (n.m_prev->m_tok.m_type != Parser::TokenFunctionStart)
            {
                m_diag.reportLexerErr(n.m_tok.m_location,
                                      "vector swizzles may only follow functions");
                return;
            }
            Lexer::OperationNode* func = n.m_prev;
            n.m_sub = func;
            n.m_prev = func->m_prev;
            func->m_prev->m_next = &n;
            func->m_next = nullptr;
            func->m_prev = nullptr;
        }
    }

    /* Ensure evaluation groups have proper arithmetic usage */
    for (Lexer::OperationNode& n : m_pool)
    {
        if (n.m_tok.m_type == Parser::TokenEvalGroupStart)
        {
            int idx = 0;
            for (Lexer::OperationNode* sn = n.m_sub ; sn ; sn = sn->m_next, ++idx)
            {
                if ((sn->m_tok.m_type == Parser::TokenArithmeticOp && !(idx & 1)) ||
                    (sn->m_tok.m_type != Parser::TokenArithmeticOp && (idx & 1)))
                {
                    m_diag.reportLexerErr(sn->m_tok.m_location, "improper arithmetic expression");
                    return;
                }
            }
        }
    }

    /* Organize arithmetic usage into tree-hierarchy */
    for (Lexer::OperationNode& n : m_pool)
    {
        if (n.m_tok.m_type == Parser::TokenEvalGroupStart)
        {
            Lexer::OperationNode* newSub = nullptr;
            Lexer::OperationNode* lastSub = nullptr;
            for (Lexer::OperationNode* sn = n.m_sub ; sn ; sn = sn->m_next)
            {
                if (sn->m_tok.m_type == Parser::TokenArithmeticOp)
                {
                    IR::Instruction::ArithmeticOpType op = ArithType(sn->m_tok.m_tokenInt);
                    if (op == IR::Instruction::ArithmeticOpMultiply ||
                        op == IR::Instruction::ArithmeticOpDivide)
                        ReconnectArithmetic(sn, &lastSub, &newSub);
                }
            }
            for (Lexer::OperationNode* sn = n.m_sub ; sn ; sn = sn->m_next)
            {
                if (sn->m_tok.m_type == Parser::TokenArithmeticOp)
                {
                    IR::Instruction::ArithmeticOpType op = ArithType(sn->m_tok.m_tokenInt);
                    if (op == IR::Instruction::ArithmeticOpAdd ||
                        op == IR::Instruction::ArithmeticOpSubtract)
                        ReconnectArithmetic(sn, &lastSub, &newSub);
                }
            }
            if (newSub)
                n.m_sub = newSub;
        }
    }

    if (HECL::VerbosityLevel)
    {
        printf("%s\n", m_diag.getSource().c_str());
        PrintTree(firstNode);
        printf("\n");
    }

    /* Done! */
    m_root = firstNode->m_next;
}

void Lexer::EmitVec3(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const
{
    /* Optimization case: if empty call, emit zero imm load */
    const Lexer::OperationNode* gn = funcNode->m_sub;
    if (!gn)
    {
        ir.m_instructions.emplace_back(IR::OpLoadImm, funcNode->m_tok.m_location);
        ir.m_instructions.back().m_loadImm.m_immVec = {};
        return;
    }

    /* Optimization case: if all numeric literals, emit vector imm load */
    bool opt = true;
    const Parser::Token* imms[3];
    for (int i=0 ; i<3 ; ++i)
    {
        if (!gn->m_sub || gn->m_sub->m_tok.m_type != Parser::TokenNumLiteral)
        {
            opt = false;
            break;
        }
        imms[i] = &gn->m_sub->m_tok;
        gn = gn->m_next;
    }
    if (opt)
    {
        ir.m_instructions.emplace_back(IR::OpLoadImm, funcNode->m_tok.m_location);
        atVec4f& vec = ir.m_instructions.back().m_loadImm.m_immVec;
        vec.vec[0] = imms[0]->m_tokenFloat;
        vec.vec[1] = imms[1]->m_tokenFloat;
        vec.vec[2] = imms[2]->m_tokenFloat;
        vec.vec[3] = 1.0;
        return;
    }

    /* Otherwise treat as normal function */
    RecursiveFuncCompile(ir, funcNode, target);
}

void Lexer::EmitVec4(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const
{
    /* Optimization case: if empty call, emit zero imm load */
    const Lexer::OperationNode* gn = funcNode->m_sub;
    if (!gn)
    {
        ir.m_instructions.emplace_back(IR::OpLoadImm, funcNode->m_tok.m_location);
        ir.m_instructions.back().m_loadImm.m_immVec = {};
        return;
    }

    /* Optimization case: if all numeric literals, emit vector imm load */
    bool opt = true;
    const Parser::Token* imms[4];
    for (int i=0 ; i<4 ; ++i)
    {
        if (!gn->m_sub || gn->m_sub->m_tok.m_type != Parser::TokenNumLiteral)
        {
            opt = false;
            break;
        }
        imms[i] = &gn->m_sub->m_tok;
        gn = gn->m_next;
    }
    if (opt)
    {
        ir.m_instructions.emplace_back(IR::OpLoadImm, funcNode->m_tok.m_location);
        atVec4f& vec = ir.m_instructions.back().m_loadImm.m_immVec;
        vec.vec[0] = imms[0]->m_tokenFloat;
        vec.vec[1] = imms[1]->m_tokenFloat;
        vec.vec[2] = imms[2]->m_tokenFloat;
        vec.vec[3] = imms[3]->m_tokenFloat;
        return;
    }

    /* Otherwise treat as normal function */
    RecursiveFuncCompile(ir, funcNode, target);
}

void Lexer::EmitArithmetic(IR& ir, const Lexer::OperationNode* arithNode, IR::RegID target) const
{
    /* Evaluate operands */
    atVec4f* opt[2] = {nullptr};
    size_t instCount = ir.m_instructions.size();
    const Lexer::OperationNode* on = arithNode->m_sub;
    IR::RegID tgt = target;
    size_t argInsts[2];
    for (int i=0 ; i<2 ; ++i, ++tgt)
    {
        const Parser::Token& tok = on->m_tok;
        switch (tok.m_type)
        {
        case Parser::TokenFunctionStart:
            if (!tok.m_tokenString.compare("vec3"))
                EmitVec3(ir, on, tgt);
            else if (!tok.m_tokenString.compare("vec4"))
                EmitVec4(ir, on, tgt);
            else
                RecursiveFuncCompile(ir, on, tgt);
            break;
        case Parser::TokenEvalGroupStart:
            RecursiveGroupCompile(ir, on, tgt);
            break;
        case Parser::TokenNumLiteral:
        {
            ir.m_instructions.emplace_back(IR::OpLoadImm, arithNode->m_tok.m_location);
            IR::Instruction& inst = ir.m_instructions.back();
            inst.m_target = tgt;
            inst.m_loadImm.m_immVec.vec[0] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[1] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[2] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[3] = tok.m_tokenFloat;
            break;
        }
        case Parser::TokenVectorSwizzle:
            EmitVectorSwizzle(ir, on, tgt);
            break;
        default:
            m_diag.reportCompileErr(tok.m_location, "invalid lexer node for IR");
            break;
        };
        argInsts[i] = ir.m_instructions.size() - 1;
        if (ir.m_instructions.back().m_op == IR::OpLoadImm)
            opt[i] = &ir.m_instructions.back().m_loadImm.m_immVec;
        on = on->m_next;
    }

    /* Optimization case: if both operands imm load, pre-evalulate */
    if (opt[0] && opt[1] && (ir.m_instructions.size() - instCount == 2))
    {
        atVec4f eval;
        switch (ArithType(arithNode->m_tok.m_tokenInt))
        {
        case IR::Instruction::ArithmeticOpAdd:
            eval.vec[0] = opt[0]->vec[0] + opt[1]->vec[0];
            eval.vec[1] = opt[0]->vec[1] + opt[1]->vec[1];
            eval.vec[2] = opt[0]->vec[2] + opt[1]->vec[2];
            eval.vec[3] = opt[0]->vec[3] + opt[1]->vec[3];
            break;
        case IR::Instruction::ArithmeticOpSubtract:
            eval.vec[0] = opt[0]->vec[0] - opt[1]->vec[0];
            eval.vec[1] = opt[0]->vec[1] - opt[1]->vec[1];
            eval.vec[2] = opt[0]->vec[2] - opt[1]->vec[2];
            eval.vec[3] = opt[0]->vec[3] - opt[1]->vec[3];
            break;
        case IR::Instruction::ArithmeticOpMultiply:
            eval.vec[0] = opt[0]->vec[0] * opt[1]->vec[0];
            eval.vec[1] = opt[0]->vec[1] * opt[1]->vec[1];
            eval.vec[2] = opt[0]->vec[2] * opt[1]->vec[2];
            eval.vec[3] = opt[0]->vec[3] * opt[1]->vec[3];
            break;
        case IR::Instruction::ArithmeticOpDivide:
            eval.vec[0] = opt[0]->vec[0] / opt[1]->vec[0];
            eval.vec[1] = opt[0]->vec[1] / opt[1]->vec[1];
            eval.vec[2] = opt[0]->vec[2] / opt[1]->vec[2];
            eval.vec[3] = opt[0]->vec[3] / opt[1]->vec[3];
            break;
        default:
            m_diag.reportCompileErr(arithNode->m_tok.m_location, "invalid arithmetic type");
            break;
        }
        ir.m_instructions.pop_back();
        ir.m_instructions.pop_back();
        ir.m_instructions.emplace_back(IR::OpLoadImm, arithNode->m_tok.m_location);
        IR::Instruction& inst = ir.m_instructions.back();
        inst.m_target = target;
        inst.m_loadImm.m_immVec = eval;
    }
    else
    {
        ir.m_instructions.emplace_back(IR::OpArithmetic, arithNode->m_tok.m_location);
        IR::Instruction& inst = ir.m_instructions.back();
        inst.m_target = target;
        inst.m_arithmetic.m_instIdxs[0] = argInsts[0];
        inst.m_arithmetic.m_instIdxs[1] = argInsts[1];
        inst.m_arithmetic.m_op = ArithType(arithNode->m_tok.m_tokenInt);
        if (tgt > ir.m_regCount)
            ir.m_regCount = tgt;
    }
}

static int SwizzleCompIdx(char aChar, Diagnostics& diag, const SourceLocation& loc)
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
        diag.reportCompileErr(loc, "invalid swizzle char %c", aChar);
    }
    return -1;
}

void Lexer::EmitVectorSwizzle(IR& ir, const Lexer::OperationNode* swizNode, IR::RegID target) const
{
    const std::string& str = swizNode->m_tok.m_tokenString;
    if (str.size() != 1 && str.size() != 3 && str.size() != 4)
        m_diag.reportCompileErr(swizNode->m_tok.m_location, "%d component swizzles not supported", int(str.size()));

    size_t instCount = ir.m_instructions.size();
    const Lexer::OperationNode* on = swizNode->m_sub;
    const Parser::Token& tok = on->m_tok;
    switch (tok.m_type)
    {
    case Parser::TokenFunctionStart:
        if (!tok.m_tokenString.compare("vec3"))
            EmitVec3(ir, on, target);
        else if (!tok.m_tokenString.compare("vec4"))
            EmitVec4(ir, on, target);
        else
            RecursiveFuncCompile(ir, on, target);
        break;
    case Parser::TokenEvalGroupStart:
        RecursiveGroupCompile(ir, on, target);
        break;
    case Parser::TokenNumLiteral:
    {
        ir.m_instructions.emplace_back(IR::OpLoadImm, swizNode->m_tok.m_location);
        IR::Instruction& inst = ir.m_instructions.back();
        inst.m_target = target;
        inst.m_loadImm.m_immVec.vec[0] = tok.m_tokenFloat;
        inst.m_loadImm.m_immVec.vec[1] = tok.m_tokenFloat;
        inst.m_loadImm.m_immVec.vec[2] = tok.m_tokenFloat;
        inst.m_loadImm.m_immVec.vec[3] = tok.m_tokenFloat;
        break;
    }
    case Parser::TokenVectorSwizzle:
        EmitVectorSwizzle(ir, on, target);
        break;
    default:
        m_diag.reportCompileErr(tok.m_location, "invalid lexer node for IR");
        break;
    };

    /* Optimization case: if operand imm load, pre-evalulate */
    if (ir.m_instructions.back().m_op == IR::OpLoadImm && (ir.m_instructions.size() - instCount == 1))
    {
        atVec4f* opt = &ir.m_instructions.back().m_loadImm.m_immVec;
        const SourceLocation& loc = ir.m_instructions.back().m_loc;
        atVec4f eval;
        switch (str.size())
        {
        case 1:
            eval = {opt->vec[SwizzleCompIdx(str[0], m_diag, loc)]};
            break;
        case 3:
            eval.vec[0] = opt->vec[SwizzleCompIdx(str[0], m_diag, loc)];
            eval.vec[1] = opt->vec[SwizzleCompIdx(str[1], m_diag, loc)];
            eval.vec[2] = opt->vec[SwizzleCompIdx(str[2], m_diag, loc)];
            eval.vec[3] = 1.0;
            break;
        case 4:
            eval.vec[0] = opt->vec[SwizzleCompIdx(str[0], m_diag, loc)];
            eval.vec[1] = opt->vec[SwizzleCompIdx(str[1], m_diag, loc)];
            eval.vec[2] = opt->vec[SwizzleCompIdx(str[2], m_diag, loc)];
            eval.vec[3] = opt->vec[SwizzleCompIdx(str[3], m_diag, loc)];
            break;
        default:
            break;
        }

        ir.m_instructions.pop_back();
        ir.m_instructions.emplace_back(IR::OpLoadImm, swizNode->m_tok.m_location);
        IR::Instruction& inst = ir.m_instructions.back();
        inst.m_target = target;
        inst.m_loadImm.m_immVec = eval;
    }
    else
    {
        ir.m_instructions.emplace_back(IR::OpSwizzle, swizNode->m_tok.m_location);
        IR::Instruction& inst = ir.m_instructions.back();
        inst.m_swizzle.m_instIdx = ir.m_instructions.size() - 2;
        inst.m_target = target;
        for (int i=0 ; i<str.size() ; ++i)
            inst.m_swizzle.m_idxs[i] = SwizzleCompIdx(str[i], m_diag, swizNode->m_tok.m_location);
    }
}

void Lexer::RecursiveGroupCompile(IR& ir, const Lexer::OperationNode* groupNode, IR::RegID target) const
{
    IR::RegID tgt = target;
    for (const Lexer::OperationNode* sn = groupNode->m_sub ; sn ; sn = sn->m_next, ++tgt)
    {
        const Parser::Token& tok = sn->m_tok;
        switch (tok.m_type)
        {
        case Parser::TokenFunctionStart:
            if (!tok.m_tokenString.compare("vec3"))
                EmitVec3(ir, sn, tgt);
            else if (!tok.m_tokenString.compare("vec4"))
                EmitVec4(ir, sn, tgt);
            else
                RecursiveFuncCompile(ir, sn, tgt);
            break;
        case Parser::TokenEvalGroupStart:
            RecursiveGroupCompile(ir, sn, tgt);
            break;
        case Parser::TokenNumLiteral:
        {
            ir.m_instructions.emplace_back(IR::OpLoadImm, tok.m_location);
            IR::Instruction& inst = ir.m_instructions.back();
            inst.m_target = tgt;
            inst.m_loadImm.m_immVec.vec[0] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[1] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[2] = tok.m_tokenFloat;
            inst.m_loadImm.m_immVec.vec[3] = tok.m_tokenFloat;
            break;
        }
        case Parser::TokenArithmeticOp:
            EmitArithmetic(ir, sn, tgt);
            break;
        case Parser::TokenVectorSwizzle:
            EmitVectorSwizzle(ir, sn, tgt);
            break;
        default:
            m_diag.reportCompileErr(tok.m_location, "invalid lexer node for IR");
            break;
        };
    }
    if (tgt > ir.m_regCount)
        ir.m_regCount = tgt;
}

void Lexer::RecursiveFuncCompile(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const
{
    IR::RegID tgt = target;
    std::vector<size_t> instIdxs;
    for (const Lexer::OperationNode* gn = funcNode->m_sub ; gn ; gn = gn->m_next, ++tgt)
    {
        RecursiveGroupCompile(ir, gn, tgt);
        instIdxs.push_back(ir.m_instructions.size() - 1);
    }
    ir.m_instructions.emplace_back(IR::OpCall, funcNode->m_tok.m_location);
    IR::Instruction& inst = ir.m_instructions.back();
    inst.m_call.m_name = funcNode->m_tok.m_tokenString;
    inst.m_call.m_argInstIdxs = std::move(instIdxs);
    inst.m_target = target;
    if (tgt > ir.m_regCount)
        ir.m_regCount = tgt;
}

IR Lexer::compileIR() const
{
    if (!m_root)
        m_diag.reportCompileErr(SourceLocation(), "unable to compile HECL-IR for invalid source");

    IR ir;
    RecursiveFuncCompile(ir, m_root, 0);
    return ir;
}

}
}

