#include "HECL/HECL.hpp"
#include "HECL/Frontend.hpp"

namespace HECL
{
namespace Frontend
{

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
            if (n->m_prev->m_tok.m_type != Parser::TokenEvalGroupStart)
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
                    n.m_next = sn->m_next;
                    sn->m_next->m_prev = &n;
                    n.m_sub->m_prev = nullptr;
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

    /* Done! */
    m_root = firstNode->m_next;
}

IR Lexer::compileIR() const
{
    if (!m_root)
        LogModule.report(LogVisor::FatalError, "unable to compile HECL-IR for invalid source");

    IR ir;

    /* Determine maximum float regs */
    for (const Lexer::OperationNode& n : m_pool)
    {
        if (n.m_tok.m_type == Parser::TokenFunctionStart)
        {
            for (Lexer::OperationNode* sn = n.m_sub ; sn ; sn = sn->m_next)
            {

            }
        }
    }

    return ir;
}

}
}

