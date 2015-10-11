#ifndef HECLFRONTEND_HPP
#define HECLFRONTEND_HPP

#include <string>
#include <vector>
#include <forward_list>
#include <Athena/Types.hpp>

namespace HECL
{
namespace Frontend
{

struct SourceLocation
{
    int line = -1;
    int col = -1;
    SourceLocation() = default;
    SourceLocation(int l, int c) : line(l), col(c) {}
};

class Diagnostics
{
    std::string m_name;
public:
    void setName(const std::string& name) {m_name = name;}
    void reportParserErr(const SourceLocation& l, const char* format, ...);
    void reportLexerErr(const SourceLocation& l, const char* format, ...);
};

class Parser
{
public:
    enum TokenType
    {
        TokenNone,
        TokenSourceBegin,
        TokenSourceEnd,
        TokenNumLiteral,
        TokenVectorSwizzle,
        TokenEvalGroupStart,
        TokenEvalGroupEnd,
        TokenFunctionStart,
        TokenFunctionEnd,
        TokenFunctionArgDelim,
        TokenArithmeticOp,
    };
private:
    Diagnostics& m_diag;
    const std::string* m_source = nullptr;
    std::string::const_iterator m_sourceIt;
    std::vector<TokenType> m_parenStack;
    bool m_reset = false;
    void skipWhitespace(std::string::const_iterator& it);
public:
    struct Token
    {
        TokenType m_type;
        SourceLocation m_location;
        std::string m_tokenString;
        int m_tokenInt = 0;
        float m_tokenFloat = 0.0;
        Token() : m_type(TokenNone) {}
        Token(TokenType type, SourceLocation loc) : m_type(type), m_location(loc) {}
    };
    void reset(const std::string& source);
    Token consumeToken();
    SourceLocation getLocation() const;

    Parser(Diagnostics& diag) : m_diag(diag) {}
};

struct IR
{
    enum OpType
    {
        OpNone,       /**< NOP */
        OpCall,       /**< Deferred function insertion for HECL backend using specified I/O regs */
        OpLoadImm,    /**< Load a constant (numeric literal) into register */
        OpArithmetic, /**< Perform binary arithmetic between registers */
        OpSwizzle     /**< Vector insertion/extraction/swizzling operation */
    };

    using RegID = ssize_t;

    struct Instruction
    {
        OpType m_op = OpNone;
        RegID m_target = -1;

        struct
        {
            std::string m_name;
            std::vector<size_t> m_argInstIdxs;
        } m_call;

        struct
        {
            atVec4f m_immVec;
        } m_loadImm;

        enum ArithmeticOpType
        {
            ArithmeticOpNone,
            ArithmeticOpAdd,
            ArithmeticOpSubtract,
            ArithmeticOpMultiply,
            ArithmeticOpDivide
        };

        struct
        {
            ArithmeticOpType m_op = ArithmeticOpNone;
            size_t m_instIdxs[2];
        } m_arithmetic;

        struct
        {
            ssize_t m_idxs[4] = {-1};
            size_t m_instIdx;
        } m_swizzle;

        Instruction(OpType type) : m_op(type) {}
    };

    size_t m_regCount = 0;
    std::vector<Instruction> m_instructions;
};

class Lexer
{
    friend class OperationNode;
    Diagnostics& m_diag;

    /* Intermediate tree-node for organizing tokens into operations */
    struct OperationNode
    {
        Parser::Token m_tok;
        OperationNode* m_prev = nullptr;
        OperationNode* m_next = nullptr;
        OperationNode* m_sub = nullptr;

        OperationNode() {}
        OperationNode(Parser::Token&& tok) : m_tok(std::move(tok)) {}
    };

    /* Pool of nodes to keep ownership (forward_list so pointers aren't invalidated) */
    std::forward_list<OperationNode> m_pool;

    /* Final lexed root function (IR comes from this) */
    OperationNode* m_root = nullptr;

    /* Helper for relinking operator precedence */
    static void ReconnectArithmetic(OperationNode* sn, OperationNode** lastSub, OperationNode** newSub);

    /* Recursive IR compile funcs */
    static void RecursiveFuncCompile(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target);
    static void RecursiveGroupCompile(IR& ir, const Lexer::OperationNode* groupNode, IR::RegID target);
    static void EmitVec3(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target);
    static void EmitVec4(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target);
    static void EmitArithmetic(IR& ir, const Lexer::OperationNode* arithNode, IR::RegID target);
    static void EmitVectorSwizzle(IR& ir, const Lexer::OperationNode* swizNode, IR::RegID target);

public:
    void reset();
    void consumeAllTokens(Parser& parser);
    IR compileIR() const;

    Lexer(Diagnostics& diag) : m_diag(diag) {}
};

class Frontend
{
    Diagnostics m_diag;
    Parser m_parser;
    Lexer m_lexer;
public:
    IR compileSource(const std::string& source, const std::string& diagName)
    {
        m_diag.setName(diagName);
        m_parser.reset(source);
        m_lexer.consumeAllTokens(m_parser);
        return m_lexer.compileIR();
    }

    Frontend() : m_parser(m_diag), m_lexer(m_diag) {}
};

}
}

#endif // HECLFRONTEND_HPP
