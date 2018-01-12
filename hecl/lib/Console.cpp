#include "hecl/Console.hpp"
#include "hecl/CVarManager.hpp"
#include "hecl/CVar.hpp"
#include "athena/Utility.hpp"

namespace hecl
{
Console* Console::m_instance = nullptr;
Console::Console(CVarManager* cvarMgr)
{
    m_instance = this;
    registerCommand("help", "Prints information about a given function", "<command>", std::bind(&Console::help, this, std::placeholders::_1, std::placeholders::_2));
    registerCommand("listCommands", "Prints a list of all available Commands", "", std::bind(&Console::listCommands, this, std::placeholders::_1, std::placeholders::_2));
    registerCommand("listCVars", "Lists all available CVars", "", std::bind(&CVarManager::list, cvarMgr, std::placeholders::_1, std::placeholders::_2));
    registerCommand("setCVar", "Sets a given Console Variable to the specified value", "<cvar> <value>", std::bind(&CVarManager::setCVar, cvarMgr, std::placeholders::_1, std::placeholders::_2));
    registerCommand("getCVar", "Prints the value stored in the specified Console Variable", "<cvar>", std::bind(&CVarManager::getCVar, cvarMgr, std::placeholders::_1, std::placeholders::_2));
}

void Console::registerCommand(std::string_view name, std::string_view helpText, std::string_view usage, const std::function<void(Console*, const std::vector<std::string> &)>&& func)
{
    std::string lowName = name.data();
    athena::utility::tolower(lowName);
    if (m_commands.find(lowName) == m_commands.end())
        m_commands[lowName] = SConsoleCommand{name.data(), helpText.data(), usage.data(), std::move(func)};
}

void Console::executeString(const std::string& str)
{
    if (str.empty())
        return;

    /* First let's split semi-colon delimited commands */
    std::vector<std::string> commands = athena::utility::split(str, ';');

    if (commands.empty())
        return;

    for (std::string command : commands)
    {
        command = athena::utility::trim(command);
        std::vector<std::string> args = athena::utility::split(command, ' ');

        if (args.empty())
            continue;

        std::string commandName = args[0];
        args.erase(args.begin());

        std::string lowComName = commandName;
        athena::utility::tolower(lowComName);
        if (m_commands.find(lowComName) != m_commands.end())
            m_commands[lowComName].m_func(this, args);
        else
            report(Level::Error, "Command '%s' is not valid!", commandName.c_str());
    }
}

void Console::help(Console* /*con*/, const std::vector<std::string>& args)
{
    if (args.empty())
    {
        report(Level::Info, "Expected usage: help <command>");
        return;
    }
    std::string cmd = args.front();
    athena::utility::tolower(cmd);
    auto it = m_commands.find(cmd);
    if (it == m_commands.end())
    {
        report(Level::Error, "No such command '%s'", args.front().c_str());
        return;
    }

    report(Level::Info, "%s: %s", it->second.m_displayName.c_str(), it->second.m_helpString.c_str());
    if (!it->second.m_usage.empty())
        report(Level::Info, "Usage: %s %s", it->second.m_displayName.c_str(), it->second.m_usage.c_str());
}

void Console::listCommands(Console* /*con*/, const std::vector<std::string>& /*args*/)
{
    for (const auto& comPair : m_commands)
        report(Level::Info, "'%s': %s", comPair.second.m_displayName.c_str(), comPair.second.m_helpString.c_str());
}

bool Console::commandExists(std::string_view cmd)
{
    std::string cmdName = cmd.data();
    athena::utility::tolower(cmdName);

    return m_commands.find(cmdName) != m_commands.end();
}

void Console::report(Level level, const char* fmt, va_list list)
{
    char tmp[2048];
    vsnprintf(tmp, 2048, fmt, list);
    m_log.emplace_back(std::string(tmp), level);
}

void Console::report(Level level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    report(level, fmt, ap);
    va_end(ap);
}

void Console::visorReport(Console::Level level, const char* mod, const wchar_t* fmt, va_list list)
{
    wchar_t tmp[2048];
    vswprintf(tmp, 2048, fmt, list);
    std::string v = athena::utility::sprintf("[%s] %s", mod, athena::utility::wideToUtf8(tmp).c_str());
    m_log.emplace_back(athena::utility::wideToUtf8(tmp), level);
}

void Console::visorReport(Console::Level level, const char* mod, const wchar_t* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    report(level, mod, fmt, ap);
    va_end(ap);
}

void Console::visorReportSource(Console::Level level, const char* mod, const char* file, unsigned line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    visorReportSource(level, mod, file, line, fmt, ap);
    va_end(ap);
}

void Console::visorReportSource(Console::Level level, const char *mod, const char *file, unsigned line, const wchar_t *fmt, va_list ap)
{
    wchar_t tmp[2048];
    vswprintf(tmp, 2048, fmt, ap);
    std::string v = athena::utility::sprintf("[%s] %s %s:%i", mod, athena::utility::wideToUtf8(tmp).c_str(), file, line);
    m_log.emplace_back(v, level);
}

void Console::visorReportSource(Console::Level level, const char *mod, const char* file, unsigned line, const wchar_t* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    visorReportSource(level, mod, file, line, fmt, ap);
    va_end(ap);
}

void Console::visorReport(Console::Level level, const char* mod, const char* fmt, va_list ap)
{
    char tmp[2048];
    vsnprintf(tmp, 2048, fmt, ap);
    std::string v = athena::utility::sprintf("[%s] %s", mod, tmp);
    m_log.emplace_back(v, level);
}

void Console::visorReport(Console::Level level, const char* mod, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    report(level, mod, fmt, ap);
    va_end(ap);
}

void Console::visorReportSource(Console::Level level, const char* mod, const char* file, unsigned line, const char* fmt, va_list ap)
{
    char tmp[2048];
    vsnprintf(tmp, 2048, fmt, ap);
    std::string v = athena::utility::sprintf("[%s] %s %s:%i", mod, tmp, file, line);
    m_log.emplace_back(v, level);
}

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity, const char *format, va_list ap)
{
    m_con->visorReport(Console::Level(severity), modName, format, ap);
}

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity, const wchar_t* format, va_list ap)
{
    m_con->visorReport(Console::Level(severity), modName, format, ap);
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file, unsigned linenum, const char* format, va_list ap)
{
    m_con->visorReportSource(Console::Level(severity), modName, file, linenum, format, ap);
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file, unsigned linenum, const wchar_t* format, va_list ap)
{
    wchar_t tmp[2048];
    vswprintf(tmp, 2048, format, ap);
    std::string v = athena::utility::wideToUtf8(tmp);
    m_con->visorReportSource(Console::Level(severity), modName, file, linenum, format, ap);
}

void Console::dumpLog()
{
    for (const auto& l : m_log)
    {
        switch(l.second)
        {
        case Level::Info:
            printf("%s\n", l.first.c_str());
        break;
        case Level::Warning:
            printf("[Warning] %s\n", l.first.c_str());
            break;
        case Level::Error:
            printf("[ Error ] %s\n", l.first.c_str());
            break;
        case Level::Fatal:
            printf("[ Fatal ] %s\n", l.first.c_str());
            break;
        }
    }
}

void Console::RegisterLogger(Console* con)
{
    /* Determine if console logger already added */
    for (auto& logger : logvisor::MainLoggers)
    {
        if (typeid(logger.get()) == typeid(LogVisorAdapter))
            return;
    }

    /* Otherwise construct new console logger */
    logvisor::MainLoggers.emplace_back(new LogVisorAdapter(con));
}

Console* Console::instance()
{
    return m_instance;
}
}
