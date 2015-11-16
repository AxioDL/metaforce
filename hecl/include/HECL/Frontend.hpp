#ifndef HECLFRONTEND_HPP
#define HECLFRONTEND_HPP

#include <string>
#include <vector>
#include <forward_list>
#include <Athena/Types.hpp>
#include <Athena/DNA.hpp>
#include <HECL/HECL.hpp>

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
    std::string m_source;
    std::string m_backend = "Backend";
    std::string sourceDiagString(const SourceLocation& l, bool ansi=false) const;
public:
    void reset(const std::string& name, const std::string& source) {m_name = name; m_source = source;}
    void reset(const std::string& name) {m_name = name; m_source.clear();}
    void setBackend(const std::string& backend) {m_backend = backend;}
    void setBackend(const char* backend) {m_backend = backend;}
    void reportParserErr(const SourceLocation& l, const char* format, ...);
    void reportLexerErr(const SourceLocation& l, const char* format, ...);
    void reportCompileErr(const SourceLocation& l, const char* format, ...);
    void reportBackendErr(const SourceLocation& l, const char* format, ...);

    const std::string& getName() const {return m_name;}
    const std::string& getSource() const {return m_source;}
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
        const char* typeString() const
        {
            switch (m_type)
            {
            case TokenNone:
                return "None";
            case TokenSourceBegin:
                return "SourceBegin";
            case TokenSourceEnd:
                return "SourceEnd";
            case TokenNumLiteral:
                return "NumLiteral";
            case TokenVectorSwizzle:
                return "VectorSwizzle";
            case TokenEvalGroupStart:
                return "EvalGroupStart";
            case TokenEvalGroupEnd:
                return "EvalGroupEnd";
            case TokenFunctionStart:
                return "FunctionStart";
            case TokenFunctionEnd:
                return "FunctionEnd";
            case TokenFunctionArgDelim:
                return "FunctionArgDelim";
            case TokenArithmeticOp:
                return "ArithmeticOp";
            default: break;
            }
            return nullptr;
        }
    };
    void reset(const std::string& source);
    Token consumeToken();
    SourceLocation getLocation() const;

    Parser(Diagnostics& diag) : m_diag(diag) {}
};

using BigDNA = Athena::io::DNA<Athena::BigEndian>;

struct IR : BigDNA
{
    Delete _d;

    enum OpType : uint8_t
    {
        OpNone,       /**< NOP */
        OpCall,       /**< Deferred function insertion for HECL backend using specified I/O regs */
        OpLoadImm,    /**< Load a constant (numeric literal) into register */
        OpArithmetic, /**< Perform binary arithmetic between registers */
        OpSwizzle     /**< Vector insertion/extraction/swizzling operation */
    };

    using RegID = atUint16;

    struct Instruction : BigDNA
    {
        Delete _d;

        OpType m_op = OpNone;
        RegID m_target = RegID(-1);
        SourceLocation m_loc;

        struct Call : BigDNA
        {
            DECL_DNA
            String<-1> m_name;
            Value<atUint16> m_argInstCount;
            Vector<atUint16, DNA_COUNT(m_argInstCount)> m_argInstIdxs;
        } m_call;

        struct LoadImm : BigDNA
        {
            DECL_DNA
            Value<atVec4f> m_immVec = {};
        } m_loadImm;

        enum ArithmeticOpType : uint8_t
        {
            ArithmeticOpNone,
            ArithmeticOpAdd,
            ArithmeticOpSubtract,
            ArithmeticOpMultiply,
            ArithmeticOpDivide
        };

        struct Arithmetic : BigDNA
        {
            DECL_DNA
            Value<ArithmeticOpType> m_op = ArithmeticOpNone;
            Value<atUint16> m_instIdxs[2];
        } m_arithmetic;

        struct Swizzle : BigDNA
        {
            DECL_DNA
            Value<atInt8> m_idxs[4] = {-1, -1, -1, -1};
            Value<atUint16> m_instIdx;
        } m_swizzle;

        Instruction(OpType type, const SourceLocation& loc) : m_op(type), m_loc(loc) {}

        int getChildCount() const
        {
            switch (m_op)
            {
            case OpCall:
                return m_call.m_argInstIdxs.size();
            case OpArithmetic:
                return 2;
            case OpSwizzle:
                return 1;
            default:
                LogModule.report(LogVisor::FatalError, "invalid op type");
            }
            return -1;
        }

        const IR::Instruction& getChildInst(const IR& ir, size_t idx) const
        {
            switch (m_op)
            {
            case OpCall:
                return ir.m_instructions.at(m_call.m_argInstIdxs.at(idx));
            case OpArithmetic:
                if (idx > 1)
                    LogModule.report(LogVisor::FatalError, "arithmetic child idx must be 0 or 1");
                return ir.m_instructions.at(m_arithmetic.m_instIdxs[idx]);
            case OpSwizzle:
                if (idx > 0)
                    LogModule.report(LogVisor::FatalError, "swizzle child idx must be 0");
                return ir.m_instructions.at(m_swizzle.m_instIdx);
            default:
                LogModule.report(LogVisor::FatalError, "invalid op type");
            }
            return *this;
        }

        const atVec4f& getImmVec() const
        {
            if (m_op != OpLoadImm)
                LogModule.report(LogVisor::FatalError, "invalid op type");
            return m_loadImm.m_immVec;
        }

        void read(Athena::io::IStreamReader& reader)
        {
            m_op = OpType(reader.readUByte());
            m_target = reader.readUint16Big();
            switch (m_op)
            {
            default: break;
            case OpCall:
                m_call.read(reader);
                break;
            case OpLoadImm:
                m_loadImm.read(reader);
                break;
            case OpArithmetic:
                m_arithmetic.read(reader);
                break;
            case OpSwizzle:
                m_swizzle.read(reader);
                break;
            }
        }

        void write(Athena::io::IStreamWriter& writer) const
        {
            writer.writeUByte(m_op);
            writer.writeUint16Big(m_target);
            switch (m_op)
            {
            default: break;
            case OpCall:
                m_call.write(writer);
                break;
            case OpLoadImm:
                m_loadImm.write(writer);
                break;
            case OpArithmetic:
                m_arithmetic.write(writer);
                break;
            case OpSwizzle:
                m_swizzle.write(writer);
                break;
            }
        }

        size_t binarySize(size_t sz) const
        {
            sz += 3;
            switch (m_op)
            {
            default: break;
            case OpCall:
                sz = m_call.binarySize(sz);
                break;
            case OpLoadImm:
                sz = m_loadImm.binarySize(sz);
                break;
            case OpArithmetic:
                sz = m_arithmetic.binarySize(sz);
                break;
            case OpSwizzle:
                sz = m_swizzle.binarySize(sz);
                break;
            }
            return sz;
        }

        Instruction(Athena::io::IStreamReader& reader) {read(reader);}
    };

    atUint64 m_hash = 0;
    atUint16 m_regCount = 0;
    std::vector<Instruction> m_instructions;

    void read(Athena::io::IStreamReader& reader)
    {
        m_hash = reader.readUint64Big();
        m_regCount = reader.readUint16Big();
        atUint16 instCount = reader.readUint16Big();
        m_instructions.clear();
        m_instructions.reserve(instCount);
        for (atUint16 i=0 ; i<instCount ; ++i)
            m_instructions.emplace_back(reader);
    }

    void write(Athena::io::IStreamWriter& writer) const
    {
        writer.writeUint64Big(m_hash);
        writer.writeUint16Big(m_regCount);
        writer.writeUint16Big(m_instructions.size());
        for (const Instruction& inst : m_instructions)
            inst.write(writer);
    }

    size_t binarySize(size_t sz) const
    {
        sz += 12;
        for (const Instruction& inst : m_instructions)
            sz = inst.binarySize(sz);
        return sz;
    }
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
    void ReconnectArithmetic(OperationNode* sn, OperationNode** lastSub, OperationNode** newSub) const;

    /* Recursive IR compile funcs */
    void RecursiveFuncCompile(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const;
    void RecursiveGroupCompile(IR& ir, const Lexer::OperationNode* groupNode, IR::RegID target) const;
    void EmitVec3(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const;
    void EmitVec4(IR& ir, const Lexer::OperationNode* funcNode, IR::RegID target) const;
    void EmitArithmetic(IR& ir, const Lexer::OperationNode* arithNode, IR::RegID target) const;
    void EmitVectorSwizzle(IR& ir, const Lexer::OperationNode* swizNode, IR::RegID target) const;

    static void PrintChain(const Lexer::OperationNode* begin, const Lexer::OperationNode* end);
    static void PrintTree(const Lexer::OperationNode* node, int indent=0);

public:
    void reset();
    void consumeAllTokens(Parser& parser);
    IR compileIR(atUint64 hash) const;

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
        Hash hash(source);
        m_diag.reset(diagName, source);
        m_parser.reset(source);
        m_lexer.consumeAllTokens(m_parser);
        return m_lexer.compileIR(hash);
    }

    Diagnostics& getDiagnostics() {return m_diag;}

    Frontend() : m_parser(m_diag), m_lexer(m_diag) {}
};

}
}

#endif // HECLFRONTEND_HPP
