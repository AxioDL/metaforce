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

#define MWM_HINTS_FUNCTIONS   (1L << 0)
#define MWM_HINTS_DECORATIONS (1L << 1)

#define MWM_DECOR_BORDER      (1L<<1)
#define MWM_DECOR_RESIZEH     (1L<<2)
#define MWM_DECOR_TITLE       (1L<<3)
#define MWM_DECOR_MENU        (1L<<4)
#define MWM_DECOR_MINIMIZE    (1L<<5)
#define MWM_DECOR_MAXIMIZE    (1L<<6)

#define MWM_FUNC_RESIZE       (1L<<1)
#define MWM_FUNC_MOVE         (1L<<2)
#define MWM_FUNC_MINIMIZE     (1L<<3)
#define MWM_FUNC_MAXIMIZE     (1L<<4)
#define MWM_FUNC_CLOSE        (1L<<5)

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
static glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;

static const int ContextAttribList[7][7] =
{
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 5,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 4,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 1,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
    GLX_CONTEXT_MINOR_VERSION_ARB, 0,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
{   GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 3,
    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
},
};

static bool s_glxError;
static int ctxErrorHandler(Display *dpy, XErrorEvent *ev)
{
    s_glxError = true;
    return 0;
}

static logvisor::Module Log("visigen-xlib");
static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.report(logvisor::Level(level), fmt, ap);
    va_end(ap);
}

static Display* xDisp;
static Window windowId;

static void UpdatePercent(float percent)
{
    XLockDisplay(xDisp);
    char title[256];
    snprintf(title, 256, "VISIGen [%g%%]", percent * 100.f);
    XChangeProperty(xDisp, windowId, XA_WM_NAME, XA_STRING, 8,
                    PropModeReplace, (unsigned char*)title, strlen(title));
    XUnlockDisplay(xDisp);
}

/* Empty handler for SIGINT */
static void _sigint(int) {}

int main(int argc, const char** argv)
{
    if (argc > 1 && !strcmp(argv[1], "--dlpackage"))
    {
        printf("%s\n", URDE_DLPACKAGE);
        return 100;
    }

    /* Program is portable to all locales */
    setlocale(LC_ALL, "");

    logvisor::RegisterStandardExceptions();
    logvisor::RegisterConsoleLogger();
    atSetExceptionHandler(AthenaExc);
    VISIRenderer renderer(argc, argv);

    if (!XInitThreads())
    {
        Log.report(logvisor::Error, "X doesn't support multithreading");
        return 1;
    }

    /* Open Xlib Display */
    xDisp = XOpenDisplay(0);
    if (!xDisp)
    {
        Log.report(logvisor::Error, "Can't open X display");
        return 1;
    }

    /* Default screen */
    int xDefaultScreen = DefaultScreen(xDisp);
    Screen* screen = ScreenOfDisplay(xDisp, xDefaultScreen);

    /* Query framebuffer configurations */
    GLXFBConfig* fbConfigs = nullptr;
    int numFBConfigs = 0;
    fbConfigs = glXGetFBConfigs(xDisp, xDefaultScreen, &numFBConfigs);
    if (!fbConfigs || numFBConfigs == 0)
    {
        Log.report(logvisor::Error, "glXGetFBConfigs failed");
        return 1;
    }

    int selVisualId = -1;
    GLXFBConfig selFBConfig = nullptr;
    for (int i=0 ; i<numFBConfigs ; ++i)
    {
        GLXFBConfig config = fbConfigs[i];
        int visualId, depthSize, colorSize, doubleBuffer;
        glXGetFBConfigAttrib(xDisp, config, GLX_VISUAL_ID, &visualId);
        glXGetFBConfigAttrib(xDisp, config, GLX_DEPTH_SIZE, &depthSize);
        glXGetFBConfigAttrib(xDisp, config, GLX_BUFFER_SIZE, &colorSize);
        glXGetFBConfigAttrib(xDisp, config, GLX_DOUBLEBUFFER, &doubleBuffer);

        /* Single-buffer only */
        if (doubleBuffer)
            continue;

        if (colorSize >= 32 && depthSize >= 24)
        {
            selFBConfig = config;
            selVisualId = visualId;
            break;
        }
    }
    XFree(fbConfigs);

    if (!selFBConfig)
    {
        Log.report(logvisor::Error, "unable to find suitable pixel format");
        return 1;
    }

    XVisualInfo visTemplate = {};
    visTemplate.screen = xDefaultScreen;
    int numVisuals;
    XVisualInfo* visualList = XGetVisualInfo(xDisp, VisualScreenMask, &visTemplate, &numVisuals);
    Visual* selectedVisual = nullptr;
    for (int i=0 ; i<numVisuals ; ++i)
    {
        if (visualList[i].visualid == selVisualId)
        {
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
    if (instIdx != -1)
    {
        x = (instIdx & 1) != 0;
        y = (instIdx & 2) != 0;
    }

    windowId = XCreateWindow(xDisp, screen->root, x, y, 768, 512, 10,
                             CopyFromParent, CopyFromParent, selectedVisual,
                             CWBorderPixel | CWEventMask | CWColormap, &swa);

    if (!glXCreateContextAttribsARB)
    {
        glXCreateContextAttribsARB = (glXCreateContextAttribsARBProc)
                glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
        if (!glXCreateContextAttribsARB)
        {
            Log.report(logvisor::Error, "unable to resolve glXCreateContextAttribsARB");
            return 1;
        }
    }

    s_glxError = false;
    XErrorHandler oldHandler = XSetErrorHandler(ctxErrorHandler);
    GLXContext glxCtx = nullptr;
    for (int attribIdx=0 ; attribIdx<std::extent<decltype(ContextAttribList)>::value ; ++attribIdx)
    {
        glxCtx = glXCreateContextAttribsARB(xDisp, selFBConfig, nullptr, True, ContextAttribList[attribIdx]);
        if (glxCtx)
            break;
    }
    XSetErrorHandler(oldHandler);
    if (!glxCtx)
    {
        Log.report(logvisor::Fatal, "unable to make new GLX context");
        return 1;
    }
    GLXWindow glxWindow = glXCreateWindow(xDisp, selFBConfig, windowId, nullptr);
    if (!glxWindow)
    {
        Log.report(logvisor::Fatal, "unable to make new GLX window");
        return 1;
    }

    XMapWindow(xDisp, windowId);

    struct
    {
        unsigned long flags;
        unsigned long functions;
        unsigned long decorations;
        long inputMode;
        unsigned long status;
    } wmHints = {0};

    Atom motifWmHints = XInternAtom(xDisp, "_MOTIF_WM_HINTS", True);
    if (motifWmHints)
    {
        wmHints.flags = MWM_HINTS_DECORATIONS | MWM_HINTS_FUNCTIONS;
        wmHints.decorations |= MWM_DECOR_BORDER | MWM_DECOR_TITLE | MWM_DECOR_MINIMIZE | MWM_DECOR_MENU;
        wmHints.functions  |= MWM_FUNC_MOVE | MWM_FUNC_MINIMIZE;
        XChangeProperty(xDisp, windowId, motifWmHints, motifWmHints, 32, PropModeReplace, (unsigned char*)&wmHints, 5);
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
    bool clientRunning = true;
    std::mutex initmt;
    std::condition_variable initcv;
    std::unique_lock<std::mutex> outerLk(initmt);
    std::thread clientThread([&]()
    {
        std::unique_lock<std::mutex> innerLk(initmt);
        innerLk.unlock();
        initcv.notify_one();

        XLockDisplay(xDisp);
        if (!glXMakeContextCurrent(xDisp, glxWindow, glxWindow, glxCtx))
            Log.report(logvisor::Fatal, "unable to make GLX context current");
        XUnlockDisplay(xDisp);

        renderer.Run(UpdatePercent);
        clientRunning = false;

        XLockDisplay(xDisp);
        XClientMessageEvent exitEvent = {};
        exitEvent.type = ClientMessage;
        exitEvent.window = windowId;
        exitEvent.format = 32;
        XSendEvent(xDisp, windowId, 0, 0, (XEvent*)&exitEvent);
        XFlush(xDisp);
        XUnlockDisplay(xDisp);
    });
    initcv.wait(outerLk);

    /* Begin application event loop */
    while (clientRunning)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(x11Fd, &fds);
        if (pselect(x11Fd+1, &fds, NULL, NULL, NULL, &origmask) < 0)
        {
            /* SIGINT/SIGUSR2 handled here */
            if (errno == EINTR)
                break;
        }

        if (FD_ISSET(x11Fd, &fds))
        {
            XLockDisplay(xDisp);
            while (XPending(xDisp))
            {
                XEvent event;
                XNextEvent(xDisp, &event);
                if (XFilterEvent(&event, None)) continue;
            }
            XUnlockDisplay(xDisp);
        }
    }

    renderer.Terminate();
    if (clientThread.joinable())
        clientThread.join();

    return renderer.ReturnVal();
}
