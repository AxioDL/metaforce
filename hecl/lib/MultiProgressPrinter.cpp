#include "hecl/MultiProgressPrinter.hpp"

#define BOLD "\033[1m"
#define NORMAL "\033[0m"
#define PREV_LINE "\033[%dF"
#define HIDE_CURSOR "\033[?25l"
#define SHOW_CURSOR "\033[?25h"

#if _WIN32
#define FOREGROUND_WHITE FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE
#endif

namespace hecl
{

void MultiProgressPrinter::ThreadStat::print(const TermInfo& tinfo) const
{
    bool blocks = m_factor >= 0.f;
    float factor = std::max(0.f, std::min(1.f, m_factor));
    int iFactor = factor * 100.f;

    int half;
    if (blocks)
        half = (tinfo.width + 1) / 2 - 2;
    else
        half = tinfo.width - 4;
    int rightHalf = tinfo.width - half - 4;

    int messageLen = m_message.size();
    int submessageLen = m_submessage.size();
    if (half - messageLen < submessageLen-2)
        submessageLen = 0;

    if (submessageLen)
    {
        if (messageLen > half-submessageLen-1)
            hecl::Printf(_S("  %.*s... %s "), half-submessageLen-4, m_message.c_str(), m_submessage.c_str());
        else
        {
            hecl::Printf(_S("  %s"), m_message.c_str());
            for (int i=half-messageLen-submessageLen-1 ; i>=0 ; --i)
                hecl::Printf(_S(" "));
            hecl::Printf(_S("%s "), m_submessage.c_str());
        }
    }
    else
    {
        if (messageLen > half)
            hecl::Printf(_S("  %.*s... "), half-3, m_message.c_str());
        else
        {
            hecl::Printf(_S("  %s"), m_message.c_str());
            for (int i=half-messageLen ; i>=0 ; --i)
                hecl::Printf(_S(" "));
        }
    }

    if (blocks)
    {
        int blocks = rightHalf - 7;
        int filled = blocks * factor;
        int rem = blocks - filled;

        if (tinfo.xtermColor)
        {
            hecl::Printf(_S("" BOLD "%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                hecl::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                hecl::Printf(_S("-"));
            hecl::Printf(_S("]" NORMAL ""));
        }
        else
        {
#if _WIN32
            SetConsoleTextAttribute(tinfo.console, FOREGROUND_INTENSITY | FOREGROUND_WHITE);
#endif
            hecl::Printf(_S("%3d%% ["), iFactor);
            for (int b=0 ; b<filled ; ++b)
                hecl::Printf(_S("#"));
            for (int b=0 ; b<rem ; ++b)
                hecl::Printf(_S("-"));
            hecl::Printf(_S("]"));
#if _WIN32
            SetConsoleTextAttribute(tinfo.console, FOREGROUND_WHITE);
#endif
        }
    }
}

void MultiProgressPrinter::DrawIndeterminateBar()
{
    int half = m_termInfo.width - 2;
    int blocks = half - 2;

    ++m_indeterminateCounter;
    if (m_indeterminateCounter <= -blocks)
        m_indeterminateCounter = -blocks + 1;
    else if (m_indeterminateCounter >= blocks)
        m_indeterminateCounter = -blocks + 2;
    int absCounter = std::abs(m_indeterminateCounter);

    int pre = absCounter;
    int rem = blocks - pre - 1;

    if (m_termInfo.xtermColor)
    {
        hecl::Printf(_S("" BOLD " ["));
        for (int b=0 ; b<pre ; ++b)
            hecl::Printf(_S("-"));
        hecl::Printf(_S("#"));
        for (int b=0 ; b<rem ; ++b)
            hecl::Printf(_S("-"));
        hecl::Printf(_S("]" NORMAL ""));
    }
    else
    {
#if _WIN32
        SetConsoleTextAttribute(m_termInfo.console, FOREGROUND_INTENSITY | FOREGROUND_WHITE);
#endif
        hecl::Printf(_S(" ["));
        for (int b=0 ; b<pre ; ++b)
            hecl::Printf(_S("-"));
        hecl::Printf(_S("#"));
        for (int b=0 ; b<rem ; ++b)
            hecl::Printf(_S("-"));
        hecl::Printf(_S("]"));
#if _WIN32
        SetConsoleTextAttribute(m_termInfo.console, FOREGROUND_WHITE);
#endif
    }
}

void MultiProgressPrinter::MoveCursorUp(int n)
{
    if (n)
    {
        if (m_termInfo.xtermColor)
        {
            hecl::Printf(_S("" PREV_LINE ""), n);
        }
#if _WIN32
        else
        {
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            GetConsoleScreenBufferInfo(m_termInfo.console, &consoleInfo);
            consoleInfo.dwCursorPosition.X = 0;
            consoleInfo.dwCursorPosition.Y -= n;
            SetConsoleCursorPosition(m_termInfo.console, consoleInfo.dwCursorPosition);
        }
#endif
    }
    else
    {
        hecl::Printf(_S("\r"));
    }
}

void MultiProgressPrinter::DoPrint()
{
    auto logLk = logvisor::LockLog();
    uint64_t logCounter = logvisor::GetLogCounter();
    if (logCounter != m_lastLogCounter)
    {
        m_curThreadLines = 0;
        m_lastLogCounter = logCounter;
    }
#if _WIN32
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(m_termInfo.console, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(m_termInfo.console, &cursorInfo);
#endif
    if (m_termInfo.xtermColor)
        hecl::Printf(_S("" HIDE_CURSOR ""));

    if (m_dirty)
    {
        m_termInfo.width = (hecl::GuiMode ? 120 : std::max(80, hecl::ConsoleWidth()));
        MoveCursorUp(m_curThreadLines + m_curProgLines);
        m_curThreadLines = m_curProgLines = 0;

        if (m_newLineAfter)
        {
            for (const ThreadStat& stat : m_threadStats)
            {
                if (stat.m_active)
                {
                    stat.print(m_termInfo);
                    hecl::Printf(_S("\n"));
                    ++m_curThreadLines;
                }
            }

            if (m_mainIndeterminate)
            {
                DrawIndeterminateBar();
                hecl::Printf(_S("\n"));
                ++m_curProgLines;
            }
            else if (m_mainFactor >= 0.f)
            {
                float factor = std::max(0.0f, std::min(1.0f, m_mainFactor));
                int iFactor = factor * 100.0;
                int half = m_termInfo.width - 2;

                int blocks = half - 8;
                int filled = blocks * factor;
                int rem = blocks - filled;

                if (m_termInfo.xtermColor)
                {
                    hecl::Printf(_S("" BOLD "  %3d%% ["), iFactor);
                    for (int b=0 ; b<filled ; ++b)
                        hecl::Printf(_S("#"));
                    for (int b=0 ; b<rem ; ++b)
                        hecl::Printf(_S("-"));
                    hecl::Printf(_S("]" NORMAL ""));
                }
                else
                {
#if _WIN32
                    SetConsoleTextAttribute(m_termInfo.console, FOREGROUND_INTENSITY | FOREGROUND_WHITE);
#endif
                    hecl::Printf(_S("  %3d%% ["), iFactor);
                    for (int b=0 ; b<filled ; ++b)
                        hecl::Printf(_S("#"));
                    for (int b=0 ; b<rem ; ++b)
                        hecl::Printf(_S("-"));
                    hecl::Printf(_S("]"));
#if _WIN32
                    SetConsoleTextAttribute(m_termInfo.console, FOREGROUND_WHITE);
#endif
                }

                hecl::Printf(_S("\n"));
                ++m_curProgLines;
            }
        }
        else
        {
            const ThreadStat& stat = m_threadStats[m_latestThread];
            stat.print(m_termInfo);
            hecl::Printf(_S("\r"));
        }
        m_dirty = false;
    }
    else if (m_mainIndeterminate)
    {
        m_termInfo.width = (hecl::GuiMode ? 120 : std::max(80, hecl::ConsoleWidth()));
        MoveCursorUp(m_curProgLines);
        m_curProgLines = 0;
        DrawIndeterminateBar();
        hecl::Printf(_S("\n"));
        ++m_curProgLines;
    }

    if (m_termInfo.xtermColor)
        hecl::Printf(_S("" SHOW_CURSOR ""));
    fflush(stdout);

#if _WIN32
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(m_termInfo.console, &cursorInfo);
#endif
}

void MultiProgressPrinter::LogProc()
{
    while (m_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (!m_dirty && !m_mainIndeterminate)
            continue;
        std::lock_guard<std::mutex> lk(m_logLock);
        DoPrint();
    }
}

MultiProgressPrinter::MultiProgressPrinter(bool activate)
{
    if (activate)
    {
        /* Xterm check */
#if _WIN32
        m_newLineAfter = true;
        m_termInfo.console = GetStdHandle(STD_OUTPUT_HANDLE);
        const char* conemuANSI = getenv("ConEmuANSI");
        if (conemuANSI && !strcmp(conemuANSI, "ON"))
            m_termInfo.xtermColor = true;
#else
        m_newLineAfter = false;
        const char* term = getenv("TERM");
        if (term && !strncmp(term, "xterm", 5))
        {
            m_termInfo.xtermColor = true;
            m_newLineAfter = true;
        }
#endif

        m_running = true;
        m_logThread = std::thread(std::bind(&MultiProgressPrinter::LogProc, this));
    }
}

MultiProgressPrinter::~MultiProgressPrinter()
{
    m_running = false;
    if (m_logThread.joinable())
        m_logThread.join();
}

void MultiProgressPrinter::print(const hecl::SystemChar* message,
                                 const hecl::SystemChar* submessage,
                                 float factor, int threadIdx) const
{
    if (!m_running)
        return;
    std::lock_guard<std::mutex> lk(m_logLock);
    if (threadIdx < 0)
        threadIdx = 0;
    if (threadIdx >= m_threadStats.size())
        m_threadStats.resize(threadIdx + 1);
    ThreadStat& stat = m_threadStats[threadIdx];
    if (message)
        stat.m_message = message;
    else
        stat.m_message.clear();
    if (submessage)
        stat.m_submessage = submessage;
    else
        stat.m_submessage.clear();
    stat.m_factor = factor;
    stat.m_active = true;
    m_latestThread = threadIdx;
    m_dirty = true;
}

void MultiProgressPrinter::setMainFactor(float factor) const
{
    if (!m_running)
        return;
    std::lock_guard<std::mutex> lk(m_logLock);
    if (!m_mainIndeterminate)
        m_dirty = true;
    m_mainFactor = factor;
}

void MultiProgressPrinter::setMainIndeterminate(bool indeterminate) const
{
    if (!m_running)
        return;
    std::lock_guard<std::mutex> lk(m_logLock);
    if (m_mainIndeterminate != indeterminate)
    {
        m_mainIndeterminate = indeterminate;
        m_dirty = true;
    }
}

void MultiProgressPrinter::startNewLine() const
{
    if (!m_running)
        return;
    std::lock_guard<std::mutex> lk(m_logLock);
    const_cast<MultiProgressPrinter&>(*this).DoPrint();
    m_threadStats.clear();
    m_curThreadLines = 0;
    m_mainFactor = -1.f;
    auto logLk = logvisor::LockLog();
    hecl::Printf(_S("\n"));
}

void MultiProgressPrinter::flush() const
{
    std::lock_guard<std::mutex> lk(m_logLock);
    const_cast<MultiProgressPrinter&>(*this).DoPrint();
}

}
