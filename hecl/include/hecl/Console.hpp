#ifndef __URDE_CONSOLE_HPP__
#define __URDE_CONSOLE_HPP__

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace hecl
{

struct SConsoleCommand
{
    std::string m_displayName;
    std::string m_helpString;
    std::string m_usage;
    std::function<void(const std::vector<std::string>&)> m_func;
};

class Console
{
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
public:
    Console(class CVarManager*);
    void registerCommand(std::string_view name, std::string_view helpText, std::string_view usage, const std::function<void(const std::vector<std::string>&)>&& func);

    void executeString(const std::string& strToExec);

    void help(const std::vector<std::string>& args);
    void listCommands(const std::vector<std::string>& args);
    bool commandExists(std::string_view cmd);


    void print(Level level, const char *fmt, va_list list);
    void print(Level, const char* fmt, ...);
    void dumpLog();
    static Console* instance();
};
}

#endif // __URDE_CONSOLE_HPP__
