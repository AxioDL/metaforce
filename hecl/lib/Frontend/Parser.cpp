#include "HECL/HECL.hpp"
#include "HECL/Frontend.hpp"

/* Syntatical token parsing system */

namespace HECL
{
namespace Frontend
{

void Parser::skipWhitespace(std::string::const_iterator& it)
{
    while (true)
    {
        while (isspace(*it) && it != m_source->cend())
            ++it;

        /* Skip comment line */
        if (*it == '#')
        {
            while (*it != '\n' && it != m_source->cend())
                ++it;
            if (*it == '\n')
                ++it;
            continue;
        }
        break;
    }
}

void Parser::reset(const std::string& source)
{
    m_source = &source;
    m_sourceIt = m_source->cbegin();
    m_parenStack.clear();
    m_reset = true;
}

Parser::Token Parser::consumeToken()
{
    if (!m_source)
        return Parser::Token(TokenNone, SourceLocation());

    /* If parser has just been reset, emit begin token */
    if (m_reset)
    {
        m_reset = false;
        return Parser::Token(TokenSourceBegin, getLocation());
    }

    /* Skip whitespace */
    skipWhitespace(m_sourceIt);

    /* Check for source end */
    if (m_sourceIt == m_source->cend())
        return Parser::Token(TokenSourceEnd, getLocation());

    /* Check for numeric literal */
    {
        char* strEnd;
        float val = strtof(&*m_sourceIt, &strEnd);
        if (&*m_sourceIt != strEnd)
        {
            Parser::Token tok(TokenNumLiteral, getLocation());
            tok.m_tokenFloat = val;
            m_sourceIt += (strEnd - &*m_sourceIt);
            return tok;
        }
    }

    /* Check for swizzle op */
    if (*m_sourceIt == '.')
    {
        int count = 0;
        std::string::const_iterator tmp = m_sourceIt + 1;
        if (tmp != m_source->cend())
        {
            for (int i=0 ; i<4 ; ++i)
            {
                std::string::const_iterator tmp2 = tmp + i;
                if (tmp2 == m_source->cend())
                    break;
                char ch = tolower(*tmp2);
                if (ch >= 'w' && ch <= 'z')
                    ++count;
                else if (ch == 'r' || ch == 'g' || ch == 'b' || ch == 'a')
                    ++count;
                else
                    break;
            }
        }
        if (count)
        {
            Parser::Token tok(TokenVectorSwizzle, getLocation());
            for (int i=0 ; i<count ; ++i)
            {
                std::string::const_iterator tmp2 = tmp + i;
                tok.m_tokenString += tolower(*tmp2);
            }
            m_sourceIt = tmp + count;
            return tok;
        }
    }

    /* Check for arithmetic op */
    if (*m_sourceIt == '+' || *m_sourceIt == '-' || *m_sourceIt == '*' || *m_sourceIt == '/')
    {
        Parser::Token tok(TokenArithmeticOp, getLocation());
        tok.m_tokenInt = *m_sourceIt;
        ++m_sourceIt;
        return tok;
    }

    /* Check for parenthesis end (group or function call) */
    if (*m_sourceIt == ')')
    {
        if (m_parenStack.empty())
        {
            m_diag.reportParserErr(getLocation(), "unexpected ')' while parsing");
            return Parser::Token(TokenNone, SourceLocation());
        }
        Parser::Token tok(m_parenStack.back(), getLocation());
        ++m_sourceIt;
        m_parenStack.pop_back();
        return tok;
    }

    /* Check for group start */
    if (*m_sourceIt == '(')
    {
        m_parenStack.push_back(TokenEvalGroupEnd);
        Parser::Token tok(TokenEvalGroupStart, getLocation());
        ++m_sourceIt;
        return tok;
    }

    /* Check for function start */
    if (isalpha(*m_sourceIt) || *m_sourceIt == '_')
    {
        std::string::const_iterator tmp = m_sourceIt + 1;
        while (tmp != m_source->cend() && (isalnum(*tmp) || *tmp == '_') && *tmp != '(')
            ++tmp;
        std::string::const_iterator nameEnd = tmp;
        skipWhitespace(tmp);
        if (*tmp == '(')
        {
            Parser::Token tok(TokenFunctionStart, getLocation());
            tok.m_tokenString.assign(m_sourceIt, nameEnd);
            m_sourceIt = tmp + 1;
            m_parenStack.push_back(TokenFunctionEnd);
            return tok;
        }
    }

    /* Check for function arg delimitation */
    if (*m_sourceIt == ',')
    {
        if (m_parenStack.empty() || m_parenStack.back() != TokenFunctionEnd)
        {
            m_diag.reportParserErr(getLocation(), "unexpected ',' while parsing");
            return Parser::Token(TokenNone, SourceLocation());
        }
        Parser::Token tok(TokenFunctionArgDelim, getLocation());
        ++m_sourceIt;
        return tok;
    }

    /* Error condition if reached */
    m_diag.reportParserErr(getLocation(), "unexpected token while parsing");
    return Parser::Token(TokenNone, SourceLocation());
}

SourceLocation Parser::getLocation() const
{
    if (!m_source)
        return SourceLocation();
    std::string::const_iterator it = m_source->cbegin();
    int line = 0;
    int col = 0;
    for (; it != m_sourceIt ; ++it)
    {
        ++col;
        if (*it == '\n')
        {
            ++line;
            col = 0;
        }
    }
    return {line+1, col+1};
}

}
}
