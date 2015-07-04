#ifndef CTOOL_BASE
#define CTOOL_BASE

#include <string>
#include <vector>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#include "HECL/Database.hpp"

struct ToolPassInfo
{
    HECL::SystemString pname;
    HECL::SystemString cwd;
    std::vector<HECL::SystemString> args;
    HECL::SystemString output;
    HECL::Database::Project* project = NULL;
    unsigned verbosityLevel = 0;
    bool force = false;
};

class ToolBase
{
protected:
    const ToolPassInfo& m_info;
public:
    ToolBase(const ToolPassInfo& info)
    : m_info(info) {}
    virtual ~ToolBase() {}
    virtual HECL::SystemString toolName() const=0;
    virtual int run()=0;
};

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define BOLD "\033[1m"
#define NORMAL "\033[0m"

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
                    it = string.insert(it, WRAP_INDENT, _S(' ')) + WRAP_INDENT;
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
    : m_sout(NULL), m_helpFunc(helpFunc)
    {
#if _WIN32
        CONSOLE_SCREEN_BUFFER_INFO info;
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
        m_lineWidth = info.dwSize.X;
#else
        struct winsize w;
        m_lineWidth = 80;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
            m_lineWidth = w.ws_col;
#endif
        if (m_lineWidth < 10)
            m_lineWidth = 10;
    }

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

#endif // CTOOL_BASE
