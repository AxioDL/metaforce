#include "VISIRenderer.hpp"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <GL/glx.h>
#include "athena/Global.hpp"
#include "logvisor/logvisor.hpp"
#include <thread>
#include <condition_variable>
#include <cstdint>
#include <limits.h>
#include <signal.h>

#define MWM_HINTS_FUNCTIONS (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)

#define MWM_DECOR_BORDER (1L << 1)
#define MWM_DECOR_RESIZEH (1L << 2)
#define MWM_DECOR_TITLE (1L << 3)
#define MWM_DECOR_MENU (1L << 4)
#define MWM_DECOR_MINIMIZE (1L << 5)
#define MWM_DECOR_MAXIMIZE (1L << 6)

#define MWM_FUNC_RESIZE (1L << 1)
#define MWM_FUNC_MOVE (1L << 2)
#define MWM_FUNC_MINIMIZE (1L << 3)
#define MWM_FUNC_MAXIMIZE (1L << 4)
#define MWM_FUNC_CLOSE (1L << 5)

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;

static const int ContextAttribList[7][7] = {
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 5, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 4, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 3, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 2, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 1, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 4, GLX_CONTEXT_MINOR_VERSION_ARB, 0, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
    {GLX_CONTEXT_MAJOR_VERSION_ARB, 3, GLX_CONTEXT_MINOR_VERSION_ARB, 3, GLX_CONTEXT_FLAGS_ARB,
     GLX_CONTEXT_CORE_PROFILE_BIT_ARB, 0},
};

static bool s_glxError;
static int ctxErrorHandler(Display* /*dpy*/, XErrorEvent* /*ev*/) {
  s_glxError = true;
  return 0;
}

static logvisor::Module Log("visigen-xlib");
static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* /*file*/, const char*, int /*line*/,
                      fmt::string_view fmt, fmt::format_args args) {
  AthenaLog.vreport(logvisor::Level(level), fmt, args);
}

static Display* xDisp;
static Window windowId;

static void UpdatePercent(float percent) {
  XLockDisplay(xDisp);
  std::string title = fmt::format(fmt("VISIGen [{:g}%]"), double(percent * 100.f));
  XChangeProperty(xDisp, windowId, XA_WM_NAME, XA_STRING, 8, PropModeReplace,
                  reinterpret_cast<const unsigned char*>(title.c_str()), int(title.size()));
  XUnlockDisplay(xDisp);
}

/* Empty handler for SIGINT */
static void _sigint(int) {}

int main(int argc, const char** argv) {
  if (argc > 1 && !strcmp(argv[1], "--dlpackage")) {
    fmt::print(fmt("{}\n"), URDE_DLPACKAGE);
    return 100;
  }

  /* Program is portable to all locales */
  setlocale(LC_ALL, "");

  logvisor::RegisterStandardExceptions();
  logvisor::RegisterConsoleLogger();
  atSetExceptionHandler(AthenaExc);
  VISIRenderer renderer(argc, argv);

  if (!XInitThreads()) {
    Log.report(logvisor::Error, fmt("X doesn't support multithreading"));
    return 1;
  }

  /* Open Xlib Display */
  xDisp = XOpenDisplay(nullptr);
  if (!xDisp) {
    Log.report(logvisor::Error, fmt("Can't open X display"));
    return 1;
  }

  /* Default screen */
  int xDefaultScreen = DefaultScreen(xDisp);
  Screen* screen = ScreenOfDisplay(xDisp, xDefaultScreen);

  /* Query framebuffer configurations */
  GLXFBConfig* fbConfigs = nullptr;
  int numFBConfigs = 0;
  fbConfigs = glXGetFBConfigs(xDisp, xDefaultScreen, &numFBConfigs);
  if (!fbConfigs || numFBConfigs == 0) {
    Log.report(logvisor::Error, fmt("glXGetFBConfigs failed"));
    return 1;
  }

  VisualID selVisualId = 0;
  GLXFBConfig selFBConfig = nullptr;
  for (int i = 0; i < numFBConfigs; ++i) {
    GLXFBConfig config = fbConfigs[i];
    int visualId, depthSize, colorSize, doubleBuffer;
    glXGetFBConfigAttrib(xDisp, config, GLX_VISUAL_ID, &visualId);
    glXGetFBConfigAttrib(xDisp, config, GLX_DEPTH_SIZE, &depthSize);
    glXGetFBConfigAttrib(xDisp, config, GLX_BUFFER_SIZE, &colorSize);
    glXGetFBConfigAttrib(xDisp, config, GLX_DOUBLEBUFFER, &doubleBuffer);

    /* Single-buffer only */
    if (doubleBuffer)
      continue;

    if (colorSize >= 32 && depthSize >= 24 && visualId != 0) {
      selFBConfig = config;
      selVisualId = VisualID(visualId);
      break;
    }
  }
  XFree(fbConfigs);

  if (!selFBConfig) {
    Log.report(logvisor::Error, fmt("unable to find suitable pixel format"));
    return 1;
  }

  XVisualInfo visTemplate = {};
  visTemplate.screen = xDefaultScreen;
  int numVisuals;
  XVisualInfo* visualList = XGetVisualInfo(xDisp, VisualScreenMask, &visTemplate, &numVisuals);
  Visual* selectedVisual = nullptr;
  for (int i = 0; i < numVisuals; ++i) {
    if (visualList[i].visualid == selVisualId) {
      selectedVisual = visualList[i].visual;
      break;
    }
  }
  XFree(visualList);

  /* Create colormap */
  Colormap colormapId = XCreateColormap(xDisp, screen->root, selectedVisual, AllocNone);

  /* Create window */
  XSetWindowAttributes swa;
  swa.colormap = colormapId;
  swa.border_pixmap = 0;
  swa.event_mask = 0;

  int instIdx = -1;
  if (argc > 3)
    instIdx = atoi(argv[3]);

  int x = 0;
  int y = 0;
  if (instIdx != -1) {
    x = (instIdx & 1) != 0;
    y = (instIdx & 2) != 0;
  }

  windowId = XCreateWindow(xDisp, screen->root, x, y, 768, 512, 10, CopyFromParent, CopyFromParent, selectedVisual,
                           CWBorderPixel | CWEventMask | CWColormap, &swa);

  if (!glXCreateContextAttribsARB) {
    glXCreateContextAttribsARB = reinterpret_cast<glXCreateContextAttribsARBProc>(
        glXGetProcAddressARB(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
    if (!glXCreateContextAttribsARB) {
      Log.report(logvisor::Error, fmt("unable to resolve glXCreateContextAttribsARB"));
      return 1;
    }
  }

  s_glxError = false;
  XErrorHandler oldHandler = XSetErrorHandler(ctxErrorHandler);
  GLXContext glxCtx = nullptr;
  for (uint32_t attribIdx = 0; attribIdx < std::extent<decltype(ContextAttribList)>::value; ++attribIdx) {
    glxCtx = glXCreateContextAttribsARB(xDisp, selFBConfig, nullptr, True, ContextAttribList[attribIdx]);
    if (glxCtx)
      break;
  }
  XSetErrorHandler(oldHandler);
  if (!glxCtx) {
    Log.report(logvisor::Fatal, fmt("unable to make new GLX context"));
    return 1;
  }
  GLXWindow glxWindow = glXCreateWindow(xDisp, selFBConfig, windowId, nullptr);
  if (!glxWindow) {
    Log.report(logvisor::Fatal, fmt("unable to make new GLX window"));
    return 1;
  }

  XMapWindow(xDisp, windowId);

  struct {
    unsigned long flags = 0;
    unsigned long functions = 0;
    unsigned long decorations = 0;
    long inputMode = 0;
    unsigned long status = 0;
  } wmHints;

  Atom motifWmHints = XInternAtom(xDisp, "_MOTIF_WM_HINTS", True);
  if (motifWmHints) {
    wmHints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
    wmHints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
    wmHints.functions |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
    XChangeProperty(xDisp, windowId, motifWmHints, motifWmHints, 32, PropModeReplace,
                    reinterpret_cast<unsigned char*>(&wmHints), 5);
  }

  /* SIGINT will be used to cancel main thread when client thread ends
   * (also enables graceful quitting via ctrl-c) */
  pthread_t mainThread = pthread_self();
  struct sigaction s;
  s.sa_handler = _sigint;
  sigemptyset(&s.sa_mask);
  s.sa_flags = 0;
  sigaction(SIGINT, &s, nullptr);
  sigaction(SIGUSR2, &s, nullptr);

  sigset_t waitmask, origmask;
  sigemptyset(&waitmask);
  sigaddset(&waitmask, SIGINT);
  sigaddset(&waitmask, SIGUSR2);
  pthread_sigmask(SIG_BLOCK, &waitmask, &origmask);

  int x11Fd = ConnectionNumber(xDisp);

  /* Spawn client thread */
  std::mutex initmt;
  std::condition_variable initcv;
  std::unique_lock<std::mutex> outerLk(initmt);
  std::thread clientThread([&]() {
    std::unique_lock<std::mutex> innerLk(initmt);
    innerLk.unlock();
    initcv.notify_one();

    XLockDisplay(xDisp);
    if (!glXMakeContextCurrent(xDisp, glxWindow, glxWindow, glxCtx))
      Log.report(logvisor::Fatal, fmt("unable to make GLX context current"));
    XUnlockDisplay(xDisp);

    renderer.Run(UpdatePercent);

    XLockDisplay(xDisp);
    XClientMessageEvent exitEvent = {};
    exitEvent.type = ClientMessage;
    exitEvent.window = windowId;
    exitEvent.format = 32;
    XSendEvent(xDisp, windowId, 0, 0, reinterpret_cast<XEvent*>(&exitEvent));
    XFlush(xDisp);
    XUnlockDisplay(xDisp);
    pthread_kill(mainThread, SIGUSR2);
  });
  initcv.wait(outerLk);

  /* Begin application event loop */
  bool clientRunning = true;
  while (clientRunning) {
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(x11Fd, &fds);
    if (pselect(x11Fd + 1, &fds, nullptr, nullptr, nullptr, &origmask) < 0) {
      /* SIGINT/SIGUSR2 handled here */
      if (errno == EINTR || errno == SIGUSR2)
        break;
    }

    if (FD_ISSET(x11Fd, &fds)) {
      XLockDisplay(xDisp);
      while (XPending(xDisp)) {
        XEvent event;
        XNextEvent(xDisp, &event);
        if (XFilterEvent(&event, None))
          continue;
        if (event.type == ClientMessage)
          clientRunning = false;
      }
      XUnlockDisplay(xDisp);
    }
  }

  renderer.Terminate();
  pthread_cancel(clientThread.native_handle());
  if (clientThread.joinable())
    clientThread.join();

  return renderer.ReturnVal();
}
