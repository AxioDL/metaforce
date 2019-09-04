#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <boo/System.hpp>
#include <logvisor/logvisor.hpp>

namespace boo {
class IWindow;

enum class EModifierKey;
enum class ESpecialKey;

struct IGraphicsCommandQueue;
} // namespace boo

namespace hecl {
class CVarManager;
class CVar;
struct SConsoleCommand {
  enum class ECommandFlags { Normal = 0, Cheat = (1 << 0), Developer = (1 << 1) };
  std::string m_displayName;
  std::string m_helpString;
  std::string m_usage;
  std::function<void(class Console*, const std::vector<std::string>&)> m_func;
  ECommandFlags m_flags;
};
ENABLE_BITWISE_ENUM(SConsoleCommand::ECommandFlags)

class Console {
  friend class LogVisorAdapter;
  struct LogVisorAdapter : logvisor::ILogger {
    Console* m_con;
    LogVisorAdapter(Console* con) : m_con(con) {}

    ~LogVisorAdapter() override = default;
    void report(const char* modName, logvisor::Level severity, fmt::string_view format, fmt::format_args args) override;
    void report(const char* modName, logvisor::Level severity, fmt::wstring_view format,
                fmt::wformat_args args) override;
    void reportSource(const char* modName, logvisor::Level severity, const char* file, unsigned linenum,
                      fmt::string_view format, fmt::format_args args) override;
    void reportSource(const char* modName, logvisor::Level severity, const char* file, unsigned linenum,
                      fmt::wstring_view format, fmt::wformat_args args) override;
  };

public:
  static Console* m_instance;
  enum class Level {
    Info,    /**< Non-error informative message */
    Warning, /**< Non-error warning message */
    Error,   /**< Recoverable error message */
    Fatal    /**< Non-recoverable error message (Kept for compatibility with logvisor) */
  };

  enum State { Closed, Closing, Opened, Opening };

private:
  CVarManager* m_cvarMgr = nullptr;
  boo::IWindow* m_window = nullptr;
  std::unordered_map<std::string, SConsoleCommand> m_commands;
  std::vector<std::pair<std::string, Level>> m_log;
  int m_logOffset = 0;
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
  void registerCommand(std::string_view name, std::string_view helpText, std::string_view usage,
                       const std::function<void(Console*, const std::vector<std::string>&)>&& func,
                       SConsoleCommand::ECommandFlags cmdFlags = SConsoleCommand::ECommandFlags::Normal);
  void unregisterCommand(std::string_view name);

  void executeString(const std::string& strToExec);

  void help(Console* con, const std::vector<std::string>& args);
  void listCommands(Console* con, const std::vector<std::string>& args);
  bool commandExists(std::string_view cmd);

  void vreport(Level level, fmt::string_view format, fmt::format_args args);
  template <typename S, typename... Args, typename Char = fmt::char_t<S>>
  void report(Level level, const S& format, Args&&... args) {
    vreport(level, fmt::to_string_view<Char>(format),
            fmt::basic_format_args<fmt::buffer_context<Char>>(
                fmt::internal::make_args_checked<Args...>(format, args...)));
  }

  void init(boo::IWindow* ctx);
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
} // namespace hecl
