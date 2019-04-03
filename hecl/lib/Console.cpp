
#include <hecl/Console.hpp>

#include "hecl/Console.hpp"
#include "hecl/CVarManager.hpp"
#include "hecl/CVar.hpp"
#include "hecl/hecl.hpp"
#include "boo/graphicsdev/IGraphicsCommandQueue.hpp"
#include "athena/Utility.hpp"

namespace hecl {
Console* Console::m_instance = nullptr;
Console::Console(CVarManager* cvarMgr) : m_cvarMgr(cvarMgr), m_overwrite(false), m_cursorAtEnd(false) {
  m_instance = this;
  registerCommand("help", "Prints information about a given function", "<command>",
                  std::bind(&Console::help, this, std::placeholders::_1, std::placeholders::_2));
  registerCommand("listCommands", "Prints a list of all available Commands", "",
                  std::bind(&Console::listCommands, this, std::placeholders::_1, std::placeholders::_2));
  registerCommand("listCVars", "Lists all available CVars", "",
                  std::bind(&CVarManager::list, m_cvarMgr, std::placeholders::_1, std::placeholders::_2));
  registerCommand("setCVar", "Sets a given Console Variable to the specified value", "<cvar> <value>",
                  std::bind(&CVarManager::setCVar, m_cvarMgr, std::placeholders::_1, std::placeholders::_2));
  registerCommand("getCVar", "Prints the value stored in the specified Console Variable", "<cvar>",
                  std::bind(&CVarManager::getCVar, m_cvarMgr, std::placeholders::_1, std::placeholders::_2));
  m_conSpeed = cvarMgr->findOrMakeCVar("con_speed",
                                       "Speed at which the console opens and closes, calculated as pixels per second",
                                       1.f, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive);
  m_conHeight = cvarMgr->findOrMakeCVar("con_height",
                                        "Maximum absolute height of the console, height is calculated from the top of "
                                        "the window, expects values ranged from [0.f,1.f]",
                                        0.5f, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive);
}

void Console::registerCommand(std::string_view name, std::string_view helpText, std::string_view usage,
                              const std::function<void(Console*, const std::vector<std::string>&)>&& func,
                              SConsoleCommand::ECommandFlags cmdFlags) {
  std::string lowName = name.data();
  athena::utility::tolower(lowName);
  if (m_commands.find(lowName) == m_commands.end())
    m_commands[lowName] = SConsoleCommand{name.data(), helpText.data(), usage.data(), std::move(func), cmdFlags};
}

void Console::unregisterCommand(std::string_view name) {
  std::string lowName = name.data();
  athena::utility::tolower(lowName);
  if (m_commands.find(lowName) != m_commands.end())
    m_commands.erase(m_commands.find(lowName));
}

void Console::executeString(const std::string& str) {
  if (str.empty())
    return;

  /* First let's split semi-colon delimited commands */
  std::vector<std::string> commands = athena::utility::split(str, ';');

  if (commands.empty())
    return;

  for (std::string command : commands) {
    command = athena::utility::trim(command);
    std::vector<std::string> tmpArgs = athena::utility::split(command, ' ');
    if (tmpArgs.empty())
      continue;
    std::vector<std::string> args;
    args.reserve(tmpArgs.size());
    /* detect string literals */
    bool isInLiteral = false;
    std::string curLiteral;
    int depth = 0;
    for (std::string arg : tmpArgs) {
      if ((arg.front() == '\'' || arg.front() == '"')) {
        ++depth;
        isInLiteral = true;
        curLiteral += arg;
      } else if ((arg.back() == '\'' || arg.back() == '"') && isInLiteral) {
        --depth;
        curLiteral += arg;
        args.push_back(curLiteral);
        if (depth <= 0) {
          depth = 0;
          isInLiteral = false;
          curLiteral.clear();
        }
      } else if (isInLiteral)
        curLiteral += arg;
      else
        args.push_back(arg);
    }

    if (isInLiteral) {
      if ((curLiteral.back() != '\'' && curLiteral.back() != '"') || depth > 1) {
        report(Level::Warning, "Unterminated string literal");
        return;
      }
      args.push_back(curLiteral);
    }

    std::string commandName = args[0];
    args.erase(args.begin());

    std::string lowComName = commandName;
    athena::utility::tolower(lowComName);
    if (m_commands.find(lowComName) != m_commands.end()) {
      const SConsoleCommand& cmd = m_commands[lowComName];
      if (bool(cmd.m_flags & SConsoleCommand::ECommandFlags::Developer) && !com_developer->toBoolean()) {
        report(Level::Error, "This command can only be executed in developer mode", commandName.c_str());
        return;
      }

      if (bool(cmd.m_flags & SConsoleCommand::ECommandFlags::Cheat) && !com_enableCheats->toBoolean()) {
        report(Level::Error, "This command can only be executed with cheats enabled", commandName.c_str());
        return;
      }
      m_commands[lowComName].m_func(this, args);
    } else if (const CVar* cv = m_cvarMgr->findCVar(commandName)) {
      args.insert(args.begin(), commandName);
      if (args.size() > 1)
        m_cvarMgr->setCVar(this, args);
      else
        m_cvarMgr->getCVar(this, args);
    } else
      report(Level::Error, "Command '%s' is not valid!", commandName.c_str());
  }
}

void Console::help(Console* /*con*/, const std::vector<std::string>& args) {
  if (args.empty()) {
    report(Level::Info, "Expected usage: help <command>");
    return;
  }
  std::string cmd = args.front();
  athena::utility::tolower(cmd);
  auto it = m_commands.find(cmd);
  if (it == m_commands.end()) {
    report(Level::Error, "No such command '%s'", args.front().c_str());
    return;
  }

  report(Level::Info, "%s: %s", it->second.m_displayName.c_str(), it->second.m_helpString.c_str());
  if (!it->second.m_usage.empty())
    report(Level::Info, "Usage: %s %s", it->second.m_displayName.c_str(), it->second.m_usage.c_str());
}

void Console::listCommands(Console* /*con*/, const std::vector<std::string>& /*args*/) {
  for (const auto& comPair : m_commands)
    report(Level::Info, "'%s': %s", comPair.second.m_displayName.c_str(), comPair.second.m_helpString.c_str());
}

bool Console::commandExists(std::string_view cmd) {
  std::string cmdName = cmd.data();
  athena::utility::tolower(cmdName);

  return m_commands.find(cmdName) != m_commands.end();
}

void Console::report(Level level, const char* fmt, va_list list) {
  char tmp[2048];
  vsnprintf(tmp, 2048, fmt, list);
  std::vector<std::string> lines = athena::utility::split(tmp, '\n');
  for (const std::string& line : lines) {
    std::string v = athena::utility::sprintf("%s", line.c_str());
    m_log.emplace_back(v, level);
  }
  printf("%s\n", tmp);
}

void Console::report(Level level, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  report(level, fmt, ap);
  va_end(ap);
}
void Console::init(boo::IWindow* window) {
  m_window = window;
}

void Console::proc() {
  if (m_conHeight->isModified()) {
    m_cachedConHeight = m_conHeight->toFloat();
    m_conHeight->clearModified();
  }

  if (m_conSpeed->isModified()) {
    m_cachedConSpeed = m_conSpeed->toFloat();
    m_conSpeed->clearModified();
  }

  if (m_state == State::Opened) {
    printf("\r%s                                   ", m_commandString.c_str());
    fflush(stdout);
  } else if (m_state == State::Opening)
    m_state = State::Opened;
  else if (m_state == State::Closing) {
    m_state = State::Closed;
    m_commandString.clear();
  }

  if (m_cursorPosition > int(m_commandString.size() - 1))
    m_cursorPosition = int(m_commandString.size() - 1);
  if (m_cursorPosition < -1)
    m_cursorPosition = -1;

  if (m_logOffset > int(m_log.size() - 1))
    m_logOffset = int(m_log.size() - 1);
  if (m_logOffset < 0)
    m_logOffset = 0;
}

void Console::draw(boo::IGraphicsCommandQueue* /* gfxQ */) {
}

void Console::handleCharCode(unsigned long chr, boo::EModifierKey /*mod*/, bool /*repeat*/) {
  if (chr == U'`' || chr == U'~') {
    if (m_state == State::Closed || m_state == State::Closing)
      m_state = State::Opening;
    else
      m_state = State::Closing;
  }

  if (m_state == State::Opened) {
    if (!m_commandString.empty() && m_cursorPosition + 1 < int(m_commandString.size())) {
      if (m_overwrite)
        m_commandString[unsigned(m_cursorPosition + 1)] = char(chr);
      else
        m_commandString.insert(m_commandString.begin() + m_cursorPosition + 1, char(chr));
    } else
      m_commandString += char(chr);

    ++m_cursorPosition;
  }
}

void Console::handleSpecialKeyDown(boo::ESpecialKey sp, boo::EModifierKey mod, bool /*repeat*/) {
  if (m_state != Opened)
    return;

  switch (sp) {
  case boo::ESpecialKey::Insert:
    m_overwrite ^= 1;
    break;
  case boo::ESpecialKey::Backspace: {
    if (!m_commandString.empty()) {
      if (True(mod & boo::EModifierKey::Ctrl)) {
        size_t index = m_commandString.rfind(' ', size_t(m_cursorPosition - 1));

        if (index == std::string::npos) {
          m_commandString.clear();
          m_cursorPosition = -1;
        } else {
          m_commandString.erase(index, (index - m_commandString.size()));
          m_cursorPosition = int(index);
        }
        break;
      }
      if (m_cursorPosition < 0)
        break;

      m_commandString.erase(size_t(m_cursorPosition), 1);
      --m_cursorPosition;
    }
    break;
  }
  case boo::ESpecialKey::Delete: {
    if (!m_commandString.empty()) {
      // Don't try to delete if the cursor is at the end of the line
      if ((m_cursorPosition + 1) >= int(m_commandString.size()))
        break;

      if (True(mod & boo::EModifierKey::Ctrl)) {
        size_t index = m_commandString.find_first_of(' ', size_t(m_cursorPosition + 1));
        if (index != std::string::npos)
          m_commandString.erase(size_t(m_cursorPosition + 1), index + 1);
        else
          m_commandString.erase(size_t(m_cursorPosition + 1), size_t(m_cursorPosition + 1) - m_commandString.size());
        break;
      }
      m_commandString.erase(size_t(m_cursorPosition + 1), 1);
    }
    break;
  }
  case boo::ESpecialKey::PgUp: {
    if (m_logOffset < int(m_log.size() - m_maxLines) - 1)
      m_logOffset++;
    break;
  }
  case boo::ESpecialKey::PgDown: {
    if (m_logOffset > 0)
      m_logOffset--;
    break;
  }
  case boo::ESpecialKey::Enter: {
    printf("\n");
    executeString(m_commandString);
    m_cursorPosition = -1;
    m_commandHistory.insert(m_commandHistory.begin(), m_commandString);
    m_commandString.clear();
    m_showCursor = true;
    m_cursorTime = 0.f;
    break;
  }
  case boo::ESpecialKey::Left: {
    if (m_cursorPosition < 0)
      break;

    if (True(mod & boo::EModifierKey::Ctrl))
      m_cursorPosition = int(m_commandString.rfind(' ', size_t(m_cursorPosition) - 1));
    else
      m_cursorPosition--;

    m_showCursor = true;
    m_cursorTime = 0.f;
    break;
  }
  case boo::ESpecialKey::Right: {
    if (m_cursorPosition >= int(m_commandString.size() - 1))
      break;

    if (True(mod & boo::EModifierKey::Ctrl)) {
      if (m_commandString[size_t(m_cursorPosition)] == ' ')
        m_cursorPosition++;

      size_t tmpPos = m_commandString.find(' ', size_t(m_cursorPosition));
      if (tmpPos == std::string::npos)
        m_cursorPosition = int(m_commandString.size() - 1);
      else
        m_cursorPosition = int(tmpPos);
    } else
      m_cursorPosition++;

    m_showCursor = true;
    m_cursorTime = 0.f;
    break;
  }

  case boo::ESpecialKey::Up: {
    if (m_commandHistory.size() == 0)
      break;

    m_currentCommand++;

    if (m_currentCommand > int(m_commandHistory.size() - 1))
      m_currentCommand = int(m_commandHistory.size() - 1);

    m_commandString = m_commandHistory[size_t(m_currentCommand)];
    m_cursorPosition = int(m_commandString.size() - 1);
    break;
  }
  case boo::ESpecialKey::Down: {
    if (m_commandHistory.empty())
      break;
    m_currentCommand--;
    if (m_currentCommand >= 0) {
      m_commandString = m_commandHistory[size_t(m_currentCommand)];
    } else if (m_currentCommand <= -1) {
      m_currentCommand = -1;
      m_commandString.clear();
    }
    m_cursorPosition = int(m_commandString.size());
    break;
  }
  case boo::ESpecialKey::Home:
    m_cursorPosition = -1;
    break;
  case boo::ESpecialKey::End:
    m_cursorPosition = int(m_commandString.size() - 1);
    break;
  default:
    break;
  }
}

void Console::handleSpecialKeyUp(boo::ESpecialKey /*sp*/, boo::EModifierKey /*mod*/) {}

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity, const char* format, va_list ap) {
  char tmp[2048];
  vsnprintf(tmp, 2048, format, ap);
  std::vector<std::string> lines = athena::utility::split(tmp, '\n');
  for (const std::string& line : lines) {
    std::string v = athena::utility::sprintf("[%s] %s", modName, line.c_str());
    m_con->m_log.emplace_back(v, Console::Level(severity));
  }
}

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity, const wchar_t* format,
                                      va_list ap) {
  wchar_t tmp[2048];
  vswprintf(tmp, 2048, format, ap);
  std::vector<std::string> lines = athena::utility::split(athena::utility::wideToUtf8(tmp), '\n');
  for (const std::string& line : lines) {
    std::string v = athena::utility::sprintf("[%s] %s", modName, line.c_str());
    m_con->m_log.emplace_back(v, Console::Level(severity));
  }
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file,
                                            unsigned linenum, const char* format, va_list ap) {
  char tmp[2048];
  vsnprintf(tmp, 2048, format, ap);
  std::string v = athena::utility::sprintf("[%s] %s %s:%i", modName, tmp, file, linenum);
  m_con->m_log.emplace_back(v, Console::Level(severity));
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file,
                                            unsigned linenum, const wchar_t* format, va_list ap) {
  wchar_t tmp[2048];
  vswprintf(tmp, 2048, format, ap);
  std::vector<std::string> lines = athena::utility::split(athena::utility::wideToUtf8(tmp), '\n');
  for (const std::string& line : lines) {
    std::string v = athena::utility::sprintf("[%s] %s %s:%i", modName, line.c_str(), file, linenum);
    m_con->m_log.emplace_back(v, Console::Level(severity));
  }
}

void Console::dumpLog() {
  for (const auto& l : m_log) {
    switch (l.second) {
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

void Console::RegisterLogger(Console* con) { logvisor::MainLoggers.emplace_back(new LogVisorAdapter(con)); }

Console* Console::instance() { return m_instance; }
} // namespace hecl
