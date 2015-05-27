#ifndef CTOOL_BASE
#define CTOOL_BASE

#include <string>
#include <vector>
#include <HECLDatabase.hpp>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

struct SToolPassInfo
{
    std::string pname;
    std::string cwd;
    std::vector<std::string> args;
    std::string output;
    unsigned verbosityLevel = 0;
    bool force = false;
};

class CToolBase
{
protected:
    const SToolPassInfo& m_info;
public:
    CToolBase(const SToolPassInfo& info)
    : m_info(info) {}
    virtual ~CToolBase() {}
    virtual std::string toolName() const=0;
    virtual int run()=0;
};

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define CYAN "\033[0;36m"
#define BOLD "\033[1m"
#define NORMAL "\033[0m"

#define WRAP_INDENT 4

extern bool XTERM_COLOR;

class CHelpOutput
{
public:
    typedef void(*THelpFunc)(CHelpOutput&);
private:
    FILE* m_sout;
    THelpFunc m_helpFunc;
    int m_lineWidth;
    std::string m_wrapBuffer;

    void _wrapBuf(std::string& string)
    {
        int counter;
        std::string::iterator it = string.begin();

        while (it != string.end())
        {
            std::string::iterator v=it;

            /* copy string until the end of the line is reached */
            for (counter=WRAP_INDENT ; counter < m_lineWidth ; ++counter)
            {
                if (*it == '\n')
                {
                    counter = WRAP_INDENT;
                    ++it;
                }
                if (counter == WRAP_INDENT)
                    it = string.insert(it, WRAP_INDENT, ' ') + WRAP_INDENT;
                if (it >= string.end())
                    return;
                if (*it != '\n')
                    ++it;
            }
            /* check for whitespace */
            if (isspace(*it))
            {
                *it = '\n';
                counter = WRAP_INDENT;
                ++it;
            }
            else
            {
                /* check for nearest whitespace back in string */
                for (std::string::iterator k=it ; k!=string.begin() ; --k)
                {
                    if (isspace(*k))
                    {
                        counter = WRAP_INDENT;
                        if (k < v)
                        {
                            k = it;
                            string.insert(k, '\n');
                        }
                        else
                            *k = '\n';
                        it = k + 1;
                        break;
                    }
                }
            }
        }
    }

public:

    CHelpOutput(THelpFunc helpFunc)
    : m_sout(NULL), m_helpFunc(helpFunc)
    {
        struct winsize w;
        m_lineWidth = 80;
        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) != -1)
            m_lineWidth = w.ws_col;
        if (m_lineWidth < 10)
            m_lineWidth = 10;
    }

    void go()
    {
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
    }

    void print(const char* str)
    {
        fputs(str, m_sout);
    }

    void printBold(const char* str)
    {
        if (XTERM_COLOR)
            fprintf(m_sout, BOLD "%s" NORMAL, str);
        else
            fputs(str, m_sout);
    }

    void secHead(const char* headName)
    {
        if (XTERM_COLOR)
            fprintf(m_sout, BOLD "%s" NORMAL "\n", headName);
        else
            fprintf(m_sout, "%s\n", headName);
    }

    void optionHead(const char* flag, const char* synopsis)
    {
        if (XTERM_COLOR)
            fprintf(m_sout, BOLD "%s" NORMAL " (%s)\n", flag, synopsis);
        else
            fprintf(m_sout, "%s (%s)\n", flag, synopsis);
    }

    void beginWrap()
    {
        m_wrapBuffer.clear();
    }

    void wrap(const char* str)
    {
        m_wrapBuffer += str;
    }

    void wrapBold(const char* str)
    {
        m_wrapBuffer += BOLD;
        m_wrapBuffer += str;
        m_wrapBuffer += NORMAL;
    }

    void endWrap()
    {
        _wrapBuf(m_wrapBuffer);
        m_wrapBuffer += '\n';
        fputs(m_wrapBuffer.c_str(), m_sout);
        m_wrapBuffer.clear();
    }
};

#endif // CTOOL_BASE
