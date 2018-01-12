#ifndef __URDE_CONSOLE_HPP__
#define __URDE_CONSOLE_HPP__

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "logvisor/logvisor.hpp"

namespace hecl
{

struct SConsoleCommand
{
    std::string m_displayName;
    std::string m_helpString;
    std::string m_usage;
    std::function<void(class Console*, const std::vector<std::string>&)> m_func;
};

class Console
{
    friend class LogVisorAdapter;
    struct LogVisorAdapter : logvisor::ILogger
    {
        Console* m_con;
        LogVisorAdapter(Console* con)
            : m_con(con) {}

        ~LogVisorAdapter() {}
        void report(const char* modName, logvisor::Level severity,
                            const char* format, va_list ap);
        void report(const char* modName, logvisor::Level severity,
                            const wchar_t* format, va_list ap);
        void reportSource(const char* modName, logvisor::Level severity,
                                  const char* file, unsigned linenum,
                                  const char* format, va_list ap);
        void reportSource(const char* modName, logvisor::Level severity,
                                  const char* file, unsigned linenum,
                                  const wchar_t* format, va_list ap);
    };

public:
    static Console* m_instance;
    enum class Level
    {
        Info,        /**< Non-error informative message */
        Warning,     /**< Non-error warning message */
        Error,       /**< Recoverable error message */
        Fatal        /**< Non-recoverable error message (Kept for compatibility with logvisor) */
    };
private:
    std::unordered_map<std::string, SConsoleCommand> m_commands;
    std::vector<std::pair<std::string, Level>> m_log;
    void visorReport(Level level, const char* mod, const char* fmt, va_list list);
    void visorReport(Level level, const char* mod, const char* fmt, ...);
    void visorReportSource(Level, const char* mod, const char* file, unsigned line, const char* fmt, va_list);
    void visorReportSource(Level, const char* mod, const char* file, unsigned line, const char* fmt, ...);
    void visorReport(Level level, const char* mod, const wchar_t* fmt, va_list list);
    void visorReport(Level level, const char* mod, const wchar_t* fmt, ...);
    void visorReportSource(Level, const char* mod, const char* file, unsigned line, const wchar_t* fmt, va_list);
    void visorReportSource(Level, const char* mod, const char* file, unsigned line, const wchar_t* fmt, ...);
public:
    Console(class CVarManager*);
    void registerCommand(std::string_view name, std::string_view helpText, std::string_view usage, const std::function<void(Console*, const std::vector<std::string>&)>&& func);

    void executeString(const std::string& strToExec);

    void help(Console* con, const std::vector<std::string>& args);
    void listCommands(Console* con, const std::vector<std::string>& args);
    bool commandExists(std::string_view cmd);

    void report(Level level, const char *fmt, va_list list);
    void report(Level, const char* fmt, ...);
    void dumpLog();
    static Console* instance();
    static void RegisterLogger(Console* con);
};
}

#endif // __URDE_CONSOLE_HPP__
