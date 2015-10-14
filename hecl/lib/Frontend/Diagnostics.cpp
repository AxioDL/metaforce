#include "HECL/HECL.hpp"
#include "HECL/Frontend.hpp"

#include <stdarg.h>

/* ANSI sequences */
#define RED "\x1b[1;31m"
#define YELLOW "\x1b[1;33m"
#define GREEN "\x1b[1;32m"
#define MAGENTA "\x1b[1;35m"
#define CYAN "\x1b[1;36m"
#define BOLD "\x1b[1m"
#define NORMAL "\x1b[0m"

namespace HECL
{
namespace Frontend
{

std::string Diagnostics::sourceDiagString(const SourceLocation& l, bool ansi) const
{
    std::string::const_iterator it = m_source.begin();
    for (int i=1 ; i<l.line ; ++i)
    {
        while (*it != '\n' && it != m_source.end())
            ++it;
        if (*it == '\n')
            ++it;
    }
    std::string::const_iterator begin = it;
    while (*it != '\n' && it != m_source.end())
        ++it;
    std::string::const_iterator end = it;

    std::string retval(begin, end);
    retval += '\n';
    for (int i=1 ; i<l.col ; ++i)
        retval += ' ';
    if (ansi)
        retval += GREEN "^" NORMAL;
    else
        retval += '^';
    return retval;
}

void Diagnostics::reportParserErr(const SourceLocation& l, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* result = nullptr;
#ifdef _WIN32
    int length = _vscprintf(fmt, ap);
    result = (char*)malloc(length);
    vsnprintf(result, length, fmt, ap);
#else
    vasprintf(&result, fmt, ap);
#endif
    va_end(ap);
    if (LogVisor::XtermColor)
        LogModule.report(LogVisor::FatalError, CYAN "[Parser]" NORMAL " %s " YELLOW "@%d:%d " NORMAL "\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, true).c_str());
    else
        LogModule.report(LogVisor::FatalError, "[Parser] %s @%d:%d\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, false).c_str());
    free(result);
}

void Diagnostics::reportLexerErr(const SourceLocation& l, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* result = nullptr;
#ifdef _WIN32
    int length = _vscprintf(fmt, ap);
    result = (char*)malloc(length);
    vsnprintf(result, length, fmt, ap);
#else
    vasprintf(&result, fmt, ap);
#endif
    va_end(ap);
    if (LogVisor::XtermColor)
        LogModule.report(LogVisor::FatalError, CYAN "[Lexer]" NORMAL " %s " YELLOW "@%d:%d " NORMAL "\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, true).c_str());
    else
        LogModule.report(LogVisor::FatalError, "[Lexer] %s @%d:%d\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, false).c_str());
    free(result);
}

void Diagnostics::reportCompileErr(const SourceLocation& l, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* result = nullptr;
#ifdef _WIN32
    int length = _vscprintf(fmt, ap);
    result = (char*)malloc(length);
    vsnprintf(result, length, fmt, ap);
#else
    vasprintf(&result, fmt, ap);
#endif
    va_end(ap);
    if (LogVisor::XtermColor)
        LogModule.report(LogVisor::FatalError, CYAN "[Compiler]" NORMAL " %s " YELLOW "@%d:%d " NORMAL "\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, true).c_str());
    else
        LogModule.report(LogVisor::FatalError, "[Compiler] %s @%d:%d\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, false).c_str());
    free(result);
}

void Diagnostics::reportBackendErr(const SourceLocation& l, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char* result = nullptr;
#ifdef _WIN32
    int length = _vscprintf(fmt, ap);
    result = (char*)malloc(length);
    vsnprintf(result, length, fmt, ap);
#else
    vasprintf(&result, fmt, ap);
#endif
    va_end(ap);
    if (LogVisor::XtermColor)
        LogModule.report(LogVisor::FatalError, CYAN "[Backend]" NORMAL " %s " YELLOW "@%d:%d " NORMAL "\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, true).c_str());
    else
        LogModule.report(LogVisor::FatalError, "[Backend] %s @%d:%d\n%s\n%s",
                         m_name.c_str(), l.line, l.col, result, sourceDiagString(l, false).c_str());
    free(result);
}

}
}
