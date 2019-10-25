#include <hecl/Console.hpp>

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

#include "hecl/CVar.hpp"
#include "hecl/CVarManager.hpp"
#include "hecl/hecl.hpp"

#include <athena/Utility.hpp>
#include <boo/IWindow.hpp>
#include <boo/graphicsdev/IGraphicsCommandQueue.hpp>
#include <logvisor/logvisor.hpp>

namespace hecl {
Console* Console::m_instance = nullptr;
Console::Console(CVarManager* cvarMgr) : m_cvarMgr(cvarMgr), m_overwrite(false), m_cursorAtEnd(false) {
  m_instance = this;
  registerCommand("help", "Prints information about a given function", "<command>",
                  [this](Console* console, const std::vector<std::string>& args) { help(console, args); });
  registerCommand("listCommands", "Prints a list of all available Commands", "",
                  [this](Console* console, const std::vector<std::string>& args) { listCommands(console, args); });
  registerCommand("listCVars", "Lists all available CVars", "",
                  [this](Console* console, const std::vector<std::string>& args) { m_cvarMgr->list(console, args); });
  registerCommand(
      "setCVar", "Sets a given Console Variable to the specified value", "<cvar> <value>",
      [this](Console* console, const std::vector<std::string>& args) { m_cvarMgr->setCVar(console, args); });
  registerCommand(
      "getCVar", "Prints the value stored in the specified Console Variable", "<cvar>",
      [this](Console* console, const std::vector<std::string>& args) { m_cvarMgr->getCVar(console, args); });
  m_conSpeed = cvarMgr->findOrMakeCVar("con_speed",
                                       "Speed at which the console opens and closes, calculated as pixels per second",
                                       1.f, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive);
  m_conHeight = cvarMgr->findOrMakeCVar("con_height",
                                        "Maximum absolute height of the console, height is calculated from the top of "
                                        "the window, expects values ranged from [0.f,1.f]",
                                        0.5f, hecl::CVar::EFlags::System | hecl::CVar::EFlags::Archive);
}

void Console::registerCommand(std::string_view name, std::string_view helpText, std::string_view usage,
                              std::function<void(Console*, const std::vector<std::string>&)>&& func,
                              SConsoleCommand::ECommandFlags cmdFlags) {
  std::string lowName{name};
  athena::utility::tolower(lowName);

  if (m_commands.find(lowName) != m_commands.end()) {
    return;
  }

  m_commands.emplace(std::move(lowName), SConsoleCommand{std::string{name}, std::string{helpText}, std::string{usage},
                                                         std::move(func), cmdFlags});
}

void Console::unregisterCommand(std::string_view name) {
  std::string lowName{name};
  athena::utility::tolower(lowName);

  const auto iter = m_commands.find(lowName);
  if (iter == m_commands.end()) {
    return;
  }

  m_commands.erase(iter);
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
      } else if (isInLiteral) {
        curLiteral += arg;
      } else {
        args.push_back(std::move(arg));
      }
    }

    if (isInLiteral) {
      if ((curLiteral.back() != '\'' && curLiteral.back() != '"') || depth > 1) {
        report(Level::Warning, fmt("Unterminated string literal"));
        return;
      }
      args.push_back(std::move(curLiteral));
    }

    std::string commandName = args[0];
    args.erase(args.begin());

    std::string lowComName = commandName;
    athena::utility::tolower(lowComName);
    if (const auto iter = m_commands.find(lowComName); iter != m_commands.end()) {
      const SConsoleCommand& cmd = iter->second;
      if (bool(cmd.m_flags & SConsoleCommand::ECommandFlags::Developer) && !com_developer->toBoolean()) {
        report(Level::Error, fmt("This command can only be executed in developer mode"), commandName);
        return;
      }

      if (bool(cmd.m_flags & SConsoleCommand::ECommandFlags::Cheat) && !com_enableCheats->toBoolean()) {
        report(Level::Error, fmt("This command can only be executed with cheats enabled"), commandName);
        return;
      }
      cmd.m_func(this, args);
    } else if (const CVar* cv = m_cvarMgr->findCVar(commandName)) {
      args.insert(args.begin(), std::move(commandName));
      if (args.size() > 1)
        m_cvarMgr->setCVar(this, args);
      else
        m_cvarMgr->getCVar(this, args);
    } else {
      report(Level::Error, fmt("'{}' is not a valid command or variable!"), commandName);
    }
}

void Console::help(Console* /*con*/, const std::vector<std::string>& args) {
  if (args.empty()) {
    report(Level::Info, fmt("Expected usage: help <command>"));
    return;
  }
  std::string cmd = args.front();
  athena::utility::tolower(cmd);
  auto it = m_commands.find(cmd);
  if (it == m_commands.end()) {
    report(Level::Error, fmt("No such command '{}'"), args.front());
    return;
  }

  report(Level::Info, fmt("{}: {}"), it->second.m_displayName, it->second.m_helpString);
  if (!it->second.m_usage.empty())
    report(Level::Info, fmt("Usage: {} {}"), it->second.m_displayName, it->second.m_usage);
}

void Console::listCommands(Console* /*con*/, const std::vector<std::string>& /*args*/) {
  for (const auto& comPair : m_commands)
    report(Level::Info, fmt("'{}': {}"), comPair.second.m_displayName, comPair.second.m_helpString);
}

bool Console::commandExists(std::string_view cmd) const {
  std::string cmdName{cmd};
  athena::utility::tolower(cmdName);

  return m_commands.find(cmdName) != m_commands.end();
}

void Console::vreport(Level level, fmt::string_view fmt, fmt::format_args args) {
  std::string tmp = fmt::vformat(fmt, args);
  std::vector<std::string> lines = athena::utility::split(tmp, '\n');
  for (std::string& line : lines) {
    m_log.emplace_back(std::move(line), level);
  }
  fmt::print(fmt("{}\n"), tmp);
}

void Console::init(boo::IWindow* window) {
  m_window = window;
}

void Console::proc() {
  if (m_conHeight->isModified()) {
    m_cachedConHeight = float(m_conHeight->toReal());
  }

  if (m_conSpeed->isModified()) {
    m_cachedConSpeed = float(m_conSpeed->toReal());
  }

  if (m_state == State::Opened) {
    fmt::print(fmt("\r{}                                   "), m_commandString);
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
  if (m_state != State::Opened) {
    return;
  }

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
    fmt::print(fmt("\n"));
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

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity,
                                      fmt::string_view format, fmt::format_args args) {
  auto tmp = fmt::internal::vformat(format, args);
  std::vector<std::string> lines = athena::utility::split(tmp, '\n');
  for (const std::string& line : lines) {
    auto v = fmt::format(fmt("[{}] {}"), modName, line);
    m_con->m_log.emplace_back(std::move(v), Console::Level(severity));
  }
}

void Console::LogVisorAdapter::report(const char* modName, logvisor::Level severity,
                                      fmt::wstring_view format, fmt::wformat_args args) {
  auto tmp = fmt::internal::vformat(format, args);
  std::vector<std::string> lines = athena::utility::split(athena::utility::wideToUtf8(tmp), '\n');
  for (const std::string& line : lines) {
    auto v = fmt::format(fmt("[{}] {}"), modName, line);
    m_con->m_log.emplace_back(std::move(v), Console::Level(severity));
  }
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file,
                                            unsigned linenum, fmt::string_view format, fmt::format_args args) {
  auto tmp = fmt::internal::vformat(format, args);
  auto v = fmt::format(fmt("[{}] {} {}:{}"), modName, tmp, file, linenum);
  m_con->m_log.emplace_back(std::move(v), Console::Level(severity));
}

void Console::LogVisorAdapter::reportSource(const char* modName, logvisor::Level severity, const char* file,
                                            unsigned linenum, fmt::wstring_view format, fmt::wformat_args args) {
  auto tmp = fmt::internal::vformat(format, args);
  std::vector<std::string> lines = athena::utility::split(athena::utility::wideToUtf8(tmp), '\n');
  for (const std::string& line : lines) {
    auto v = fmt::format(fmt("[{}] {} {}:{}"), modName, line, file, linenum);
    m_con->m_log.emplace_back(std::move(v), Console::Level(severity));
  }
}

void Console::dumpLog() {
  for (const auto& l : m_log) {
    switch (l.second) {
    case Level::Info:
      fmt::print(fmt("{}\n"), l.first);
      break;
    case Level::Warning:
      fmt::print(fmt("[Warning] {}\n"), l.first);
      break;
    case Level::Error:
      fmt::print(fmt("[ Error ] {}\n"), l.first);
      break;
    case Level::Fatal:
      fmt::print(fmt("[ Fatal ] {}\n"), l.first);
      break;
    }
  }
}

void Console::RegisterLogger(Console* con) { logvisor::MainLoggers.emplace_back(new LogVisorAdapter(con)); }

Console* Console::instance() { return m_instance; }
} // namespace hecl
