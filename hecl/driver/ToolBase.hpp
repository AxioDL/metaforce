#ifndef CTOOL_BASE
#define CTOOL_BASE

#include <string>
#include <vector>
#include <list>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#include <unistd.h>
#endif

#include "hecl/Database.hpp"
#include "logvisor/logvisor.hpp"

extern logvisor::Module LogModule;

struct ToolPassInfo
{
    hecl::SystemString pname;
    hecl::SystemString cwd;
    std::vector<hecl::SystemString> args;
    std::vector<hecl::SystemChar> flags;
    hecl::SystemString output;
    hecl::Database::Project* project = nullptr;
    unsigned verbosityLevel = 0;
    bool force = false;
    bool yes = false;
};

class ToolBase
{
protected:
    const ToolPassInfo& m_info;
    bool m_good = false;
public:
    ToolBase(const ToolPassInfo& info)
    : m_info(info)
    {
        hecl::VerbosityLevel = info.verbosityLevel;
    }
    virtual ~ToolBase() {}
    virtual hecl::SystemString toolName() const=0;
    virtual int run()=0;
    inline operator bool() const {return m_good;}
};

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define BOLD "\033[1m"
#define NORMAL "\033[0m"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#define WRAP_INDENT 4

extern bool XTERM_COLOR;

class HelpOutput
{
public:
    typedef void(*HelpFunc)(HelpOutput&);
private:
    FILE* m_sout;
    HelpFunc m_helpFunc;
    int m_lineWidth;
    hecl::SystemString m_wrapBuffer;

    void _wrapBuf(hecl::SystemString& string)
    {
        int counter;
        hecl::SystemString::iterator it = string.begin();

        while (it != string.end())
        {
            hecl::SystemString::iterator v=it;

            /* copy string until the end of the line is reached */
            for (counter=WRAP_INDENT ; counter < m_lineWidth ; ++counter)
            {
                if (*it == _S('\n'))
                {
                    counter = WRAP_INDENT;
                    ++it;
                }
                if (counter == WRAP_INDENT)
                {
                    for (int i=0 ; i<WRAP_INDENT ; ++i)
                        it = string.insert(it, _S(' ')) + 1;
                }
                if (it >= string.end())
                    return;
                if (*it != _S('\n'))
                    ++it;
            }
            /* check for whitespace */
            if (isspace(*it))
            {
                *it = _S('\n');
                counter = WRAP_INDENT;
                ++it;
            }
            else
            {
                /* check for nearest whitespace back in string */
                for (hecl::SystemString::iterator k=it ; k!=string.begin() ; --k)
                {
                    if (isspace(*k))
                    {
                        counter = WRAP_INDENT;
                        if (k < v)
                        {
                            k = it;
                            string.insert(k, _S('\n'));
                        }
                        else
                            *k = _S('\n');
                        it = k + 1;
                        break;
                    }
                }
            }
        }
    }

public:

    HelpOutput(HelpFunc helpFunc)
    : m_sout(NULL), m_helpFunc(helpFunc), m_lineWidth(hecl::ConsoleWidth())
    {}

    void go()
    {
#if _WIN32
        m_sout = stdout;
        m_helpFunc(*this);
#else
        m_sout = popen("less -R", "w");
        if (m_sout)
        {
            m_helpFunc(*this);
            pclose(m_sout);
        }
        else
        {
            m_sout = stdout;
            m_helpFunc(*this);
        }
#endif
    }

    void print(const hecl::SystemChar* str)
    {
        hecl::FPrintf(m_sout, _S("%s"), str);
    }

    void printBold(const hecl::SystemChar* str)
    {
        if (XTERM_COLOR)
            hecl::FPrintf(m_sout, _S("" BOLD "%s" NORMAL ""), str);
        else
            hecl::FPrintf(m_sout, _S("%s"), str);
    }

    void secHead(const hecl::SystemChar* headName)
    {
        if (XTERM_COLOR)
            hecl::FPrintf(m_sout, _S("" BOLD "%s" NORMAL "\n"), headName);
        else
            hecl::FPrintf(m_sout, _S("%s\n"), headName);
    }

    void optionHead(const hecl::SystemChar* flag, const hecl::SystemChar* synopsis)
    {
        if (XTERM_COLOR)
            hecl::FPrintf(m_sout, _S("" BOLD "%s" NORMAL " (%s)\n"), flag, synopsis);
        else
            hecl::FPrintf(m_sout, _S("%s (%s)\n"), flag, synopsis);
    }

    void beginWrap()
    {
        m_wrapBuffer.clear();
    }

    void wrap(const hecl::SystemChar* str)
    {
        m_wrapBuffer += str;
    }

    void wrapBold(const hecl::SystemChar* str)
    {
        if (XTERM_COLOR)
            m_wrapBuffer += _S("" BOLD "");
        m_wrapBuffer += str;
        if (XTERM_COLOR)
            m_wrapBuffer += _S("" NORMAL "");
    }

    void endWrap()
    {
        _wrapBuf(m_wrapBuffer);
        m_wrapBuffer += _S('\n');
        hecl::FPrintf(m_sout, _S("%s"), m_wrapBuffer.c_str());
        m_wrapBuffer.clear();
    }
};

static hecl::SystemString MakePathArgAbsolute(const hecl::SystemString& arg,
                                              const hecl::SystemString& cwd)
{
#if _WIN32
    if (arg.size() >= 2 && iswalpha(arg[0]) && arg[1] == _S(':'))
        return arg;
    if (arg[0] == _S('\\') || arg[0] == _S('/'))
        return arg;
    return cwd + _S('\\') + arg;
#else
    if (arg[0] == _S('/') || arg[0] == _S('\\'))
        return arg;
    return cwd + _S('/') + arg;
#endif
}

void ToolPrintProgress(const hecl::SystemChar* message, const hecl::SystemChar* submessage,
                       int lidx, float factor, int& lineIdx)
{
    bool blocks = factor >= 0.0;
    factor = std::max(0.0f, std::min(1.0f, factor));
    int iFactor = factor * 100.0;
    if (XTERM_COLOR)
        hecl::Printf(_S("" HIDE_CURSOR ""));

    if (lidx > lineIdx)
    {
        hecl::Printf(_S("\n  "));
        lineIdx = lidx;
    }
    else
        hecl::Printf(_S("  "));

    int width = std::max(80, hecl::ConsoleWidth());
    int half;
    if (blocks)
        half = width / 2 - 2;
    else
        half = width - 4;

    if (!message)
        message = _S("");
    size_t messageLen = hecl::StrLen(message);
    if (!submessage)
        submessage = _S("");
    size_t submessageLen = hecl::StrLen(submessage);
    if (half - messageLen < submessageLen-2)
        submessageLen = 0;

    if (submessageLen)
    {
        if (messageLen > half-submessageLen-1)
            hecl::Printf(_S("%.*s... %s "), half-int(submessageLen)-4, message, submessage);
        else
        {
            hecl::Printf(_S("%s"), message);
            for (int i=half-messageLen-submessageLen-1 ; i>=0 ; --i)
                hecl::Printf(_S(" "));
            hecl::Printf(_S("%s "), submessage);
        }
    }
    else
    {
        if (messageLen > half)
            hecl::Printf(_S("%.*s... "), half-3, message);
        else
        {
            hecl::Printf(_S("%s"), message);
            for (int i=half-messageLen ; i>=0 ; --i)
                hecl::Printf(_S(" "));
        }
    }

    if (blocks)
    {
        if (XTERM_COLOR)
        {
            size_t blocks = half - 7;
            size_t filled = blocks * factor;
            size_t rem = blocks - filled;
            hecl::Printf(_S("" BOLD "%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                hecl::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                hecl::Printf(_S("-"));
            hecl::Printf(_S("]" NORMAL ""));
        }
        else
        {
            size_t blocks = half - 7;
            size_t filled = blocks * factor;
            size_t rem = blocks - filled;
            hecl::Printf(_S("%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                hecl::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                hecl::Printf(_S("-"));
            hecl::Printf(_S("]"));
        }
    }

    hecl::Printf(_S("\r"));
    if (XTERM_COLOR)
        hecl::Printf(_S("" SHOW_CURSOR ""));
    fflush(stdout);
}

#endif // CTOOL_BASE
