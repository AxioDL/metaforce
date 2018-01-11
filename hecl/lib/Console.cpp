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
    registerCommand("help", "Prints information about a given function", "<command>", std::bind(&Console::help, this, std::placeholders::_1));
    registerCommand("listCommands", "Prints a list of all available Commands", "", std::bind(&Console::listCommands, this, std::placeholders::_1));
    registerCommand("listCVars", "Lists all available CVars", "", std::bind(&CVarManager::list, cvarMgr, std::placeholders::_1));
    registerCommand("setCVar", "Sets a given Console Variable to the specified value", "<cvar> <value>", std::bind(&CVarManager::setCVar, cvarMgr, std::placeholders::_1));
    registerCommand("getCVar", "Prints the value stored in the specified Console Variable", "<cvar>", std::bind(&CVarManager::getCVar, cvarMgr, std::placeholders::_1));
}

void Console::registerCommand(std::string_view name, std::string_view helpText, std::string_view usage, const std::function<void(const std::vector<std::__cxx11::string> &)>&& func)
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
            m_commands[lowComName].m_func(args);
        else
            print(Level::Error, "Command '%s' is not valid!", commandName.c_str());
    }
}

void Console::help(const std::vector<std::string>& args)
{
    if (args.empty())
    {
        print(Level::Info, "Expected usage: help <command>");
        return;
    }
    std::string cmd = args.front();
    athena::utility::tolower(cmd);
    auto it = m_commands.find(cmd);
    if (it == m_commands.end())
    {
        print(Level::Error, "No such command '%s'", args.front().c_str());
        return;
    }

    print(Level::Info, "%s: %s", it->second.m_displayName.c_str(), it->second.m_helpString.c_str());
    if (!it->second.m_usage.empty())
        print(Level::Info, "Usage: %s %s", it->second.m_displayName.c_str(), it->second.m_usage.c_str());
}

void Console::listCommands(const std::vector<std::string>& /*args*/)
{
    for (const auto& comPair : m_commands)
        print(Level::Info, "'%s': %s", comPair.second.m_displayName.c_str(), comPair.second.m_helpString.c_str());
}

bool Console::commandExists(std::string_view cmd)
{
    std::string cmdName = cmd.data();
    athena::utility::tolower(cmdName);

    return m_commands.find(cmdName) != m_commands.end();
}

void Console::print(Level level, const char* fmt, va_list list)
{
    char tmp[2048];
    vsnprintf(tmp, 2048, fmt, list);
    m_log.emplace_back(std::string(tmp), level);
}

void Console::print(Level level, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    print(level, fmt, ap);
    va_end(ap);
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

Console* Console::instance()
{
    return m_instance;
}
}
