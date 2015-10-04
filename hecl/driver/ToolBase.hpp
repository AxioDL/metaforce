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

#include "HECL/Database.hpp"
#include "LogVisor/LogVisor.hpp"

extern LogVisor::LogModule LogModule;

struct ToolPassInfo
{
    HECL::SystemString pname;
    HECL::SystemString cwd;
    std::vector<HECL::SystemString> args;
    HECL::SystemString output;
    HECL::Database::Project* project = nullptr;
    unsigned verbosityLevel = 0;
    bool force = false;
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
        HECL::VerbosityLevel = info.verbosityLevel;
    }
    virtual ~ToolBase() {}
    virtual HECL::SystemString toolName() const=0;
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
    HECL::SystemString m_wrapBuffer;

    void _wrapBuf(HECL::SystemString& string)
    {
        int counter;
        HECL::SystemString::iterator it = string.begin();

        while (it != string.end())
        {
            HECL::SystemString::iterator v=it;

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
                for (HECL::SystemString::iterator k=it ; k!=string.begin() ; --k)
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
    : m_sout(NULL), m_helpFunc(helpFunc), m_lineWidth(HECL::ConsoleWidth())
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

    void print(const HECL::SystemChar* str)
    {
        HECL::FPrintf(m_sout, _S("%s"), str);
    }

    void printBold(const HECL::SystemChar* str)
    {
        if (XTERM_COLOR)
            HECL::FPrintf(m_sout, _S("" BOLD "%s" NORMAL ""), str);
        else
            HECL::FPrintf(m_sout, _S("%s"), str);
    }

    void secHead(const HECL::SystemChar* headName)
    {
        if (XTERM_COLOR)
            HECL::FPrintf(m_sout, _S("" BOLD "%s" NORMAL "\n"), headName);
        else
            HECL::FPrintf(m_sout, _S("%s\n"), headName);
    }

    void optionHead(const HECL::SystemChar* flag, const HECL::SystemChar* synopsis)
    {
        if (XTERM_COLOR)
            HECL::FPrintf(m_sout, _S("" BOLD "%s" NORMAL " (%s)\n"), flag, synopsis);
        else
            HECL::FPrintf(m_sout, _S("%s (%s)\n"), flag, synopsis);
    }

    void beginWrap()
    {
        m_wrapBuffer.clear();
    }

    void wrap(const HECL::SystemChar* str)
    {
        m_wrapBuffer += str;
    }

    void wrapBold(const HECL::SystemChar* str)
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
        HECL::FPrintf(m_sout, _S("%s"), m_wrapBuffer.c_str());
        m_wrapBuffer.clear();
    }
};

static HECL::SystemString MakePathArgAbsolute(const HECL::SystemString& arg,
                                              const HECL::SystemString& cwd)
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

void ToolPrintProgress(const HECL::SystemChar* message, const HECL::SystemChar* submessage,
                       int lidx, float factor, int& lineIdx)
{
    bool blocks = factor >= 0.0;
    factor = std::max(0.0f, std::min(1.0f, factor));
    int iFactor = factor * 100.0;
    if (XTERM_COLOR)
        HECL::Printf(_S("" HIDE_CURSOR ""));

    if (lidx > lineIdx)
    {
        HECL::Printf(_S("\n  "));
        lineIdx = lidx;
    }
    else
        HECL::Printf(_S("  "));

    int width = HECL::ConsoleWidth();
    int half;
    if (blocks)
        half = width / 2 - 2;
    else
        half = width - 4;

    if (!message)
        message = _S("");
    size_t messageLen = HECL::StrLen(message);
    if (!submessage)
        submessage = _S("");
    size_t submessageLen = HECL::StrLen(submessage);
    if (half - messageLen < submessageLen-2)
        submessageLen = 0;

    if (submessageLen)
    {
        if (messageLen > half-submessageLen-1)
            HECL::Printf(_S("%.*s... "), half-int(submessageLen)-4, message);
        else
        {
            HECL::Printf(_S("%s"), message);
            for (int i=half-messageLen-submessageLen-1 ; i>=0 ; --i)
                HECL::Printf(_S(" "));
            HECL::Printf(_S("%s "), submessage);
        }
    }
    else
    {
        if (messageLen > half)
            HECL::Printf(_S("%.*s... "), half-3, message);
        else
        {
            HECL::Printf(_S("%s"), message);
            for (int i=half-messageLen ; i>=0 ; --i)
                HECL::Printf(_S(" "));
        }
    }

    if (blocks)
    {
        if (XTERM_COLOR)
        {
            size_t blocks = half - 7;
            size_t filled = blocks * factor;
            size_t rem = blocks - filled;
            HECL::Printf(_S("" BOLD "%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                HECL::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                HECL::Printf(_S("-"));
            HECL::Printf(_S("]" NORMAL ""));
        }
        else
        {
            size_t blocks = half - 7;
            size_t filled = blocks * factor;
            size_t rem = blocks - filled;
            HECL::Printf(_S("%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                HECL::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                HECL::Printf(_S("-"));
            HECL::Printf(_S("]"));
        }
    }

    HECL::Printf(_S("\r"));
    if (XTERM_COLOR)
        HECL::Printf(_S("" SHOW_CURSOR ""));
    fflush(stdout);
}

#endif // CTOOL_BASE
