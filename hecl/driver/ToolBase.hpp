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
  hecl::SystemString pname;
  hecl::SystemString cwd;
  std::vector<hecl::SystemString> args;
  std::vector<hecl::SystemChar> flags;
  hecl::SystemString output;
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
        fmt::print(fmt(_SYS_STR("\n" BLUE BOLD "Continue?" NORMAL " (Y/n) ")));
      else
        fmt::print(fmt(_SYS_STR("\nContinue? (Y/n) ")));
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
          fmt::print(fmt(_SYS_STR("\n")));
          return false;
        }
        if (ch == 'y' || ch == 'Y' || ch == '\r' || ch == '\n')
          break;
      }
#ifndef _WIN32
      tcsetattr(0, TCSANOW, &tioOld);
#endif
    }
    fmt::print(fmt(_SYS_STR("\n")));
    return true;
  }

public:
  ToolBase(const ToolPassInfo& info) : m_info(info) {
    hecl::VerbosityLevel = info.verbosityLevel;
    hecl::GuiMode = info.gui;
  }
  virtual ~ToolBase() = default;
  virtual hecl::SystemString toolName() const = 0;
  virtual int run() = 0;
  virtual void cancel() {}
  explicit operator bool() const { return m_good; }
};

class HelpOutput {
public:
  typedef void (*HelpFunc)(HelpOutput&);

private:
  FILE* m_sout;
  HelpFunc m_helpFunc;
  int m_lineWidth;
  hecl::SystemString m_wrapBuffer;

  void _wrapBuf(hecl::SystemString& string) {
    int counter;
    hecl::SystemString::iterator it = string.begin();

    while (it != string.end()) {
      std::ptrdiff_t v = it - string.begin();

      /* copy string until the end of the line is reached */
      for (counter = WRAP_INDENT; counter < m_lineWidth; ++counter) {
        if (it >= string.end())
          return;
        if (*it == _SYS_STR('\n')) {
          counter = WRAP_INDENT;
          ++it;
        }
        if (counter == WRAP_INDENT) {
          for (int i = 0; i < WRAP_INDENT; ++i)
            it = string.insert(it, _SYS_STR(' ')) + 1;
        }
        if (it >= string.end())
          return;
        if (*it != _SYS_STR('\n'))
          ++it;
      }
      /* check for whitespace */
      if (isspace(*it)) {
        *it = _SYS_STR('\n');
        counter = WRAP_INDENT;
        ++it;
      } else {
        /* check for nearest whitespace back in string */
        for (hecl::SystemString::iterator k = it; k != string.begin(); --k) {
          if (isspace(*k)) {
            counter = WRAP_INDENT;
            if (k - string.begin() < v)
              k = string.insert(it, _SYS_STR('\n'));
            else
              *k = _SYS_STR('\n');
            it = k + 1;
            break;
          }
        }
      }
    }
  }

public:
  HelpOutput(HelpFunc helpFunc)
  : m_sout(NULL), m_helpFunc(helpFunc), m_lineWidth(hecl::GuiMode ? 120 : hecl::ConsoleWidth()) {}

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

  void print(const hecl::SystemChar* str) { fmt::print(m_sout, fmt(_SYS_STR("{}")), str); }

  void printBold(const hecl::SystemChar* str) {
    if (XTERM_COLOR)
      fmt::print(m_sout, fmt(_SYS_STR("" BOLD "{}" NORMAL "")), str);
    else
      fmt::print(m_sout, fmt(_SYS_STR("{}")), str);
  }

  void secHead(const hecl::SystemChar* headName) {
    if (XTERM_COLOR)
      fmt::print(m_sout, fmt(_SYS_STR("" BOLD "{}" NORMAL "\n")), headName);
    else
      fmt::print(m_sout, fmt(_SYS_STR("{}\n")), headName);
  }

  void optionHead(const hecl::SystemChar* flag, const hecl::SystemChar* synopsis) {
    if (XTERM_COLOR)
      fmt::print(m_sout, fmt(_SYS_STR("" BOLD "{}" NORMAL " ({})\n")), flag, synopsis);
    else
      fmt::print(m_sout, fmt(_SYS_STR("{} ({})\n")), flag, synopsis);
  }

  void beginWrap() { m_wrapBuffer.clear(); }

  void wrap(const hecl::SystemChar* str) { m_wrapBuffer += str; }

  void wrapBold(const hecl::SystemChar* str) {
    if (XTERM_COLOR)
      m_wrapBuffer += _SYS_STR("" BOLD "");
    m_wrapBuffer += str;
    if (XTERM_COLOR)
      m_wrapBuffer += _SYS_STR("" NORMAL "");
  }

  void endWrap() {
    _wrapBuf(m_wrapBuffer);
    m_wrapBuffer += _SYS_STR('\n');
    fmt::print(m_sout, fmt(_SYS_STR("{}")), m_wrapBuffer);
    m_wrapBuffer.clear();
  }
};

static hecl::SystemString MakePathArgAbsolute(const hecl::SystemString& arg, const hecl::SystemString& cwd) {
#if _WIN32
  if (arg.size() >= 2 && iswalpha(arg[0]) && arg[1] == _SYS_STR(':'))
    return arg;
  if (arg[0] == _SYS_STR('\\') || arg[0] == _SYS_STR('/'))
    return arg;
  return cwd + _SYS_STR('\\') + arg;
#else
  if (arg[0] == _SYS_STR('/') || arg[0] == _SYS_STR('\\'))
    return arg;
  if (cwd.back() == _SYS_STR('/') || cwd.back() == _SYS_STR('\\'))
    return cwd + arg;
  return cwd + _SYS_STR('/') + arg;
#endif
}
