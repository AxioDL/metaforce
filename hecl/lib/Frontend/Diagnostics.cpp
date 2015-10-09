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
        LogModule.report(LogVisor::FatalError, RED "Error parsing" NORMAL " '%s' " YELLOW "@%d:%d " NORMAL "\n%s",
                         m_name.c_str(), l.line, l.col, result);
    else
        LogModule.report(LogVisor::FatalError, "Error parsing '%s' @%d:%d\n%s",
                         m_name.c_str(), l.line, l.col, result);
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
        LogModule.report(LogVisor::FatalError, RED "Error lexing" NORMAL " '%s' " YELLOW "@%d:%d " NORMAL "\n%s",
                         m_name.c_str(), l.line, l.col, result);
    else
        LogModule.report(LogVisor::FatalError, "Error lexing '%s' @%d:%d\n%s",
                         m_name.c_str(), l.line, l.col, result);
    free(result);
}

}
}
