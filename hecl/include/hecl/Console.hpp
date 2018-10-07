#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <functional>
#include "boo/IWindow.hpp"
#include "logvisor/logvisor.hpp"

namespace hecl
{
class CVarManager;
class CVar;
struct SConsoleCommand
{
    enum class ECommandFlags
    {
        Normal = 0,
        Cheat = (1 << 0),
        Developer = (1 << 1)
    };
    std::string m_displayName;
    std::string m_helpString;
    std::string m_usage;
    std::function<void(class Console*, const std::vector<std::string>&)> m_func;
    ECommandFlags m_flags;
};
ENABLE_BITWISE_ENUM(SConsoleCommand::ECommandFlags)

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

    enum State
    {
        Closed,
        Closing,
        Opened,
        Opening
    };

private:
    CVarManager* m_cvarMgr;
    std::unordered_map<std::string, SConsoleCommand> m_commands;
    std::vector<std::pair<std::string, Level>> m_log;
    int m_logOffset;
    std::string m_commandString;
    std::vector<std::string> m_commandHistory;
    int m_cursorPosition = -1;
    int m_currentCommand = -1;
    size_t m_maxLines = 0;
    bool m_overwrite : 1;
    bool m_cursorAtEnd : 1;
    State m_state = State::Closed;
    CVar* m_conSpeed;
    CVar* m_conHeight;
    float m_cachedConSpeed;
    float m_cachedConHeight;
    bool m_showCursor = true;
    float m_cursorTime = 0.f;
public:
    Console(CVarManager*);
    void registerCommand(std::string_view name, std::string_view helpText, std::string_view usage, const std::function<void(Console*, const std::vector<std::string>&)>&& func, SConsoleCommand::ECommandFlags cmdFlags = SConsoleCommand::ECommandFlags::Normal);
    void unregisterCommand(std::string_view name);

    void executeString(const std::string& strToExec);

    void help(Console* con, const std::vector<std::string>& args);
    void listCommands(Console* con, const std::vector<std::string>& args);
    bool commandExists(std::string_view cmd);

    void report(Level level, const char *fmt, va_list list);
    void report(Level level, const char* fmt, ...);

    void proc();
    void draw(boo::IGraphicsCommandQueue* gfxQ);
    void handleCharCode(unsigned long chr, boo::EModifierKey mod, bool repeat);
    void handleSpecialKeyDown(boo::ESpecialKey sp, boo::EModifierKey mod, bool repeat);
    void handleSpecialKeyUp(boo::ESpecialKey sp, boo::EModifierKey mod);
    void dumpLog();
    static Console* instance();
    static void RegisterLogger(Console* con);
    bool isOpen() { return m_state == State::Opened; }
};
}

