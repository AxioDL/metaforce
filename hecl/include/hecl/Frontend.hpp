#ifndef HECLFRONTEND_HPP
#define HECLFRONTEND_HPP

#include <string>
#include <vector>
#include <forward_list>
#include <athena/Types.hpp>
#include <athena/DNA.hpp>
#include <hecl/hecl.hpp>
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

namespace hecl::Frontend
{

using namespace std::literals;

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
    void reset(std::string_view name, std::string_view source) {m_name = name; m_source = source;}
    void reset(std::string_view name) {m_name = name; m_source.clear();}
    void setBackend(std::string_view backend) {m_backend = backend;}
    void reportScannerErr(const SourceLocation& l, const char* format, ...);
    void reportParserErr(const SourceLocation& l, const char* format, ...);
    void reportBackendErr(const SourceLocation& l, const char* format, ...);

    std::string_view getName() const {return m_name;}
    std::string_view getSource() const {return m_source;}
};

struct Token
{
    enum class Kind
    {
        None,
        Eof,
        Lf,
        Plus,
        Minus,
        Times,
        Div,
        Lpar,
        Rpar,
        Comma,
        Period,
        Ident,
        Number
    };

    static std::string_view KindToStr(Kind k)
    {
        switch (k)
        {
        case Kind::None: return "none"sv;
        case Kind::Eof: return "eof"sv;
        case Kind::Lf: return "lf"sv;
        case Kind::Plus: return "+"sv;
        case Kind::Minus: return "-"sv;
        case Kind::Times: return "*"sv;
        case Kind::Div: return "/"sv;
        case Kind::Lpar: return "("sv;
        case Kind::Rpar: return ")"sv;
        case Kind::Comma: return ","sv;
        case Kind::Period: return "."sv;
        case Kind::Ident: return "ident"sv;
        case Kind::Number: return "number"sv;
        }
    }

    Kind kind = Kind::None;
    std::string str;
    SourceLocation loc;

    Token() = default;
    Token(Kind kind, const SourceLocation& loc)
        : kind(kind), loc(loc) {}
    Token(Kind kind, std::string&& str, const SourceLocation& loc)
        : kind(kind), str(std::move(str)), loc(loc) {}

    std::string toString() const
    {
        if (str.empty())
            return hecl::Format("%d:%d: %s", loc.line, loc.col, KindToStr(kind).data());
        else
            return hecl::Format("%d:%d: %s (%s)", loc.line, loc.col, KindToStr(kind).data(), str.c_str());
    }
};

class Scanner
{
    friend class Parser;
    static constexpr char LF = '\n';
    static constexpr char COMMENT = '#';

    Diagnostics& m_diag;
    std::string_view m_source;
    std::string_view::const_iterator m_sourceIt;
    char ch;
    SourceLocation loc;
    int lfcol;

    std::string lastLine;
    std::string currentLine;

    Token::Kind CharToTokenKind(char ch)
    {
        switch (ch)
        {
        case '(': return Token::Kind::Lpar;
        case ')': return Token::Kind::Rpar;
        case ',': return Token::Kind::Comma;
        case '.': return Token::Kind::Period;
        case '+': return Token::Kind::Plus;
        case '-': return Token::Kind::Minus;
        case '*': return Token::Kind::Times;
        case '/': return Token::Kind::Div;
        default: return Token::Kind::None;
        }
    }

    template <class... Args>
    void error(const SourceLocation& loc, const char* s, Args&&... args)
    {
        m_diag.reportScannerErr(loc, s, args...);
    }

    static bool isDigit(char c)
    {
        return c >= '0' && c <= '9';
    }

    static bool isStartIdent(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
               (c == '_');
    }

    static bool isMidIdent(char c)
    {
        return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
               (c == '_') || isDigit(c);
    }

    int _read();
    bool read();

    static char chr(char c) { return (c < 32 || c > 127) ? '.' : c; }

public:
    Scanner(Diagnostics& diag) : m_diag(diag) {}

    void reset(std::string_view in)
    {
        m_source = in;
        m_sourceIt = in.cbegin();
        ch = 0;
        loc.line = 1;
        loc.col = 0;
        lfcol = 0;
        lastLine = std::string();
        currentLine = std::string();
        read();
    }

    Token next();
};

struct IRNode
{
    friend struct IR;

    enum class Op
    {
        Add, Sub, Mul, Div
    };
    enum class Kind
    {
        None, Call, Imm, Binop, Swizzle
    };
    Kind kind = Kind::None;
    std::string str;
    float val;
    Op op;
    std::unique_ptr<IRNode> left;
    std::unique_ptr<IRNode> right;
    std::list<IRNode> children;
    SourceLocation loc;

    static std::string_view OpToStr(Op op)
    {
        switch (op)
        {
        case Op::Add: return "+"sv;
        case Op::Sub: return "-"sv;
        case Op::Mul: return "*"sv;
        case Op::Div: return "/"sv;
        }
    }

    static std::string_view KindToStr(Kind k)
    {
        switch (k)
        {
        case Kind::None: return "none"sv;
        case Kind::Call: return "call"sv;
        case Kind::Imm: return "imm"sv;
        case Kind::Binop: return "binop"sv;
        case Kind::Swizzle: return "swizzle"sv;
        }
    }

    IRNode() = default;
    IRNode(Kind kind, std::string&& str, const SourceLocation& loc)
        : kind(kind), str(std::move(str)), loc(loc) {}
    IRNode(Kind kind, float val, const SourceLocation& loc)
        : kind(kind), val(val), loc(loc) {}
    IRNode(Kind kind, std::string&& str, std::list<IRNode>&& children, const SourceLocation& loc)
        : kind(kind), str(std::move(str)), children(std::move(children)), loc(loc) {}
    IRNode(Op op, IRNode&& left, IRNode&& right, const SourceLocation& loc)
        : kind(Kind::Binop), op(op), left(new IRNode(std::move(left))), right(new IRNode(std::move(right))), loc(loc) {}
    IRNode(Kind kind, std::string&& str, IRNode&& node, const SourceLocation& loc)
        : kind(kind), str(std::move(str)), left(new IRNode(std::move(node))), loc(loc) {}

    std::string toString() const { return fmt(0); }

private:
    static std::string rep(int n, std::string_view s);
    std::string fmt(int level) const;
    std::string describe() const;
};

class Parser
{
    Scanner m_scanner;

    Token t;
    Token la;
    Token::Kind sym;

    void scan()
    {
        t = la;
        la = m_scanner.next();
        sym = la.kind;
    }

    template <class... Args>
    void error(const char* s, Args&&... args)
    {
        m_scanner.m_diag.reportParserErr(la.loc, s, args...);
    }

    void check(Token::Kind expected);
    IRNode call();
    static bool imm(const IRNode& a, const IRNode& b);
    IRNode expr();
    IRNode sum();
    IRNode factor();
    IRNode value();

public:
    Parser(Diagnostics& diag)
    : m_scanner(diag) {}

    void reset(std::string_view in) { la = Token(); m_scanner.reset(in); }
    std::list<IRNode> parse();
};

using BigDNA = athena::io::DNA<athena::BigEndian>;

struct IR : BigDNA
{
    Delete _d;

    enum OpType : uint8_t
    {
        None,       /**< NOP */
        Call,       /**< Deferred function insertion for HECL backend using specified I/O regs */
        LoadImm,    /**< Load a constant (numeric literal) into register */
        Arithmetic, /**< Perform binary arithmetic between registers */
        Swizzle     /**< Vector insertion/extraction/swizzling operation */
    };

    using RegID = atUint16;

    struct Instruction : BigDNA
    {
        Delete _d;

        OpType m_op = OpType::None;
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
            None,
            Add,
            Subtract,
            Multiply,
            Divide
        };

        struct Arithmetic : BigDNA
        {
            DECL_DNA
            Value<ArithmeticOpType> m_op = ArithmeticOpType::None;
            Value<atUint16> m_instIdxs[2];
        } m_arithmetic;

        struct Swizzle : BigDNA
        {
            DECL_DNA
            Value<atInt8> m_idxs[4] = {-1, -1, -1, -1};
            Value<atUint16> m_instIdx;
        } m_swizzle;

        Instruction(OpType type, RegID target, const SourceLocation& loc)
            : m_op(type), m_target(target), m_loc(loc) {}
        int getChildCount() const;
        const IR::Instruction& getChildInst(const IR& ir, size_t idx) const;
        const atVec4f& getImmVec() const;
        void read(athena::io::IStreamReader& reader);
        void write(athena::io::IStreamWriter& writer) const;
        size_t binarySize(size_t sz) const;

        Instruction(athena::io::IStreamReader& reader) {read(reader);}
    };

    atUint64 m_hash = 0;
    atUint16 m_regCount = 0;
    std::vector<Instruction> m_instructions;

    boo::BlendFactor m_blendSrc = boo::BlendFactor::One;
    boo::BlendFactor m_blendDst = boo::BlendFactor::Zero;
    bool m_doAlpha = false;

    static atInt8 swizzleCompIdx(char aChar);
    int addInstruction(const IRNode& n, IR::RegID target);
    void read(athena::io::IStreamReader& reader);
    void write(athena::io::IStreamWriter& writer) const;
    size_t binarySize(size_t sz) const;
};

class Frontend
{
    Diagnostics m_diag;
    Parser m_parser;
public:
    IR compileSource(std::string_view source, std::string_view diagName);
    Diagnostics& getDiagnostics() { return m_diag; }
    Frontend() : m_parser(m_diag) {}
};

}

#endif // HECLFRONTEND_HPP
