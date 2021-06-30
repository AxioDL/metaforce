#pragma once

#include <string>
#include <vector>
#include <list>
#include <cstdio>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <termios.h>
#else
#include <conio.h>
#endif

#include "hecl/Database.hpp"
#include "logvisor/logvisor.hpp"

extern logvisor::Module LogModule;

struct ToolPassInfo {
  std::string pname;
  std::string cwd;
  std::vector<std::string> args;
  std::vector<char> flags;
  std::string output;
  hecl::Database::Project* project = nullptr;
  unsigned verbosityLevel = 0;
  bool force = false;
  bool yes = false;
  bool gui = false;
};

#define RED "\033[0;31m"
#define GREEN "\033[0;32m"
#define YELLOW "\033[0;33m"
#define BLUE "\033[0;34m"
#define MAGENTA "\033[0;35m"
#define CYAN "\033[0;36m"
#define BOLD "\033[1m"
#define NORMAL "\033[0m"

#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#define WRAP_INDENT 4

extern bool XTERM_COLOR;

class ToolBase {
protected:
  const ToolPassInfo& m_info;
  bool m_good = false;

  bool continuePrompt() {
    if (!m_info.yes) {
      if (XTERM_COLOR)
        fmt::print(FMT_STRING("\n" BLUE BOLD "Continue?" NORMAL " (Y/n) "));
      else
        fmt::print(FMT_STRING("\nContinue? (Y/n) "));
      fflush(stdout);

      int ch;
#ifndef _WIN32
      struct termios tioOld, tioNew;
      tcgetattr(0, &tioOld);
      tioNew = tioOld;
      tioNew.c_lflag &= ~ICANON;
      tcsetattr(0, TCSANOW, &tioNew);
      while ((ch = getchar()))
#else
      while ((ch = getch()))
#endif
      {
        if (ch == 'n' || ch == 'N') {
          fmt::print(FMT_STRING("\n"));
          return false;
        }
        if (ch == 'y' || ch == 'Y' || ch == '\r' || ch == '\n')
          break;
      }
#ifndef _WIN32
      tcsetattr(0, TCSANOW, &tioOld);
#endif
    }
    fmt::print(FMT_STRING("\n"));
    return true;
  }

public:
  explicit ToolBase(const ToolPassInfo& info) : m_info(info) {
    hecl::VerbosityLevel = info.verbosityLevel;
    hecl::GuiMode = info.gui;
  }
  virtual ~ToolBase() = default;
  virtual std::string_view toolName() const = 0;
  virtual int run() = 0;
  virtual void cancel() {}
  explicit operator bool() const { return m_good; }
};

class HelpOutput {
public:
  using HelpFunc = void (*)(HelpOutput&);

private:
  FILE* m_sout = nullptr;
  HelpFunc m_helpFunc;
  int m_lineWidth;
  std::string m_wrapBuffer;

  void _wrapBuf(std::string& string) {
    int counter;
    std::string::iterator it = string.begin();

    while (it != string.end()) {
      std::ptrdiff_t v = it - string.begin();

      /* copy string until the end of the line is reached */
      for (counter = WRAP_INDENT; counter < m_lineWidth; ++counter) {
        if (it >= string.end())
          return;
        if (*it == '\n') {
          counter = WRAP_INDENT;
          ++it;
        }
        if (counter == WRAP_INDENT) {
          for (int i = 0; i < WRAP_INDENT; ++i)
            it = string.insert(it, ' ') + 1;
        }
        if (it >= string.end())
          return;
        if (*it != '\n')
          ++it;
      }
      /* check for whitespace */
      if (isspace(*it)) {
        *it = '\n';
        counter = WRAP_INDENT;
        ++it;
      } else {
        /* check for nearest whitespace back in string */
        for (std::string::iterator k = it; k != string.begin(); --k) {
          if (isspace(*k)) {
            counter = WRAP_INDENT;
            if (k - string.begin() < v)
              k = string.insert(it, '\n');
            else
              *k = '\n';
            it = k + 1;
            break;
          }
        }
      }
    }
  }

public:
  explicit HelpOutput(HelpFunc helpFunc)
  : m_helpFunc(helpFunc), m_lineWidth(hecl::GuiMode ? 120 : hecl::ConsoleWidth()) {}

  void go() {
#if _WIN32
    m_sout = stdout;
    m_helpFunc(*this);
#else
    m_sout = popen("less -R", "w");
    if (m_sout) {
      m_helpFunc(*this);
      pclose(m_sout);
    } else {
      m_sout = stdout;
      m_helpFunc(*this);
    }
#endif
  }

  void print(const char* str) { fmt::print(m_sout, FMT_STRING("{}"), str); }

  void printBold(const char* str) {
    if (XTERM_COLOR)
      fmt::print(m_sout, FMT_STRING("" BOLD "{}" NORMAL ""), str);
    else
      fmt::print(m_sout, FMT_STRING("{}"), str);
  }

  void secHead(const char* headName) {
    if (XTERM_COLOR)
      fmt::print(m_sout, FMT_STRING("" BOLD "{}" NORMAL "\n"), headName);
    else
      fmt::print(m_sout, FMT_STRING("{}\n"), headName);
  }

  void optionHead(const char* flag, const char* synopsis) {
    if (XTERM_COLOR)
      fmt::print(m_sout, FMT_STRING("" BOLD "{}" NORMAL " ({})\n"), flag, synopsis);
    else
      fmt::print(m_sout, FMT_STRING("{} ({})\n"), flag, synopsis);
  }

  void beginWrap() { m_wrapBuffer.clear(); }

  void wrap(const char* str) { m_wrapBuffer += str; }

  void wrapBold(const char* str) {
    if (XTERM_COLOR)
      m_wrapBuffer += "" BOLD "";
    m_wrapBuffer += str;
    if (XTERM_COLOR)
      m_wrapBuffer += "" NORMAL "";
  }

  void endWrap() {
    _wrapBuf(m_wrapBuffer);
    m_wrapBuffer += '\n';
    fmt::print(m_sout, FMT_STRING("{}"), m_wrapBuffer);
    m_wrapBuffer.clear();
  }
};

static std::string MakePathArgAbsolute(const std::string& arg, const std::string& cwd) {
#if _WIN32
  if (arg.size() >= 2 && iswalpha(arg[0]) && arg[1] == ':')
    return arg;
  if (arg[0] == '\\' || arg[0] == '/')
    return arg;
  return cwd + '\\' + arg;
#else
  if (arg[0] == '/' || arg[0] == '\\')
    return arg;
  if (cwd.back() == '/' || cwd.back() == '\\')
    return cwd + arg;
  return cwd + '/' + arg;
#endif
}
