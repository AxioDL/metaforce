#include "VISIRenderer.hpp"
#include <AppKit/AppKit.h>
#include "athena/Global.hpp"
#include "logvisor/logvisor.hpp"
#include <thread>

#if !__has_feature(objc_arc)
#error ARC Required
#endif

static std::thread s_task;

static const NSOpenGLPixelFormatAttribute PF_RGBA8_Z24_ATTRS[] =
{
    NSOpenGLPFAAccelerated,
    NSOpenGLPFAOpenGLProfile, NSOpenGLProfileVersion3_2Core,
    //NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAColorSize, 24,
    NSOpenGLPFAAlphaSize, 8,
    NSOpenGLPFADepthSize, 24,
    0, 0
};

@interface OpenGLView : NSOpenGLView
{
    VISIRenderer* m_renderer;
}
- (id)initWithFrame:(NSRect)frame renderer:(VISIRenderer*)renderer;
@end

static NSWindow* s_Window;
static void UpdatePercent(float percent)
{
    dispatch_async(dispatch_get_main_queue(), ^{
        s_Window.title = [NSString stringWithFormat:@"VISIGen [%g%%]", percent * 100.f];
    });
}

@implementation OpenGLView
- (id)initWithFrame:(NSRect)frame renderer:(VISIRenderer*)renderer;
{
    NSOpenGLPixelFormat* pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:PF_RGBA8_Z24_ATTRS];
    self = [super initWithFrame:frame pixelFormat:pf];
    m_renderer = renderer;
    return self;
}
- (void)prepareOpenGL
{
    s_task = std::thread([self](){
        [[self openGLContext] makeCurrentContext];
        m_renderer->Run(UpdatePercent);
        [NSApp terminate:nil];
    });
}
@end

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    VISIRenderer* m_renderer;
    NSWindow* m_window;
    NSOpenGLView* m_glView;
}
- (id)initWithRenderer:(VISIRenderer*)renderer;
@end

@implementation AppDelegate
- (id)initWithRenderer:(VISIRenderer*)renderer
{
    self = [super init];
    m_renderer = renderer;
    return self;
}
- (void)applicationDidFinishLaunching:(NSNotification*)notification
{
    NSRect cRect = NSMakeRect(100, 100, 768, 512);
    m_window = [[NSWindow alloc] initWithContentRect:cRect
                                           styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskMiniaturizable
                                             backing:NSBackingStoreBuffered
                                               defer:NO];
    m_window.releasedWhenClosed = NO;
    m_window.title = @"VISIGen";
    s_Window = m_window;
    m_glView = [[OpenGLView alloc] initWithFrame:cRect renderer:m_renderer];
    m_window.contentView = m_glView;
    [m_window makeKeyAndOrderFront:nil];
}
- (void)applicationWillTerminate:(NSNotification*)notification
{
    m_renderer->Terminate();
    if (s_task.joinable())
        s_task.join();
    exit(m_renderer->ReturnVal());
}
@end

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.report(logvisor::Level(level), fmt, ap);
    va_end(ap);
}

int main(int argc, const char** argv)
{
    logvisor::RegisterStandardExceptions();
    logvisor::RegisterConsoleLogger();
    atSetExceptionHandler(AthenaExc);
    VISIRenderer renderer(argc, argv);
    @autoreleasepool
    {
        [[NSApplication sharedApplication] setActivationPolicy:NSApplicationActivationPolicyRegular];

        /* Delegate (OS X callbacks) */
        AppDelegate* appDelegate = [[AppDelegate alloc] initWithRenderer:&renderer];
        [[NSApplication sharedApplication] setDelegate:appDelegate];
        [[NSApplication sharedApplication] run];
    }
    return renderer.ReturnVal();
}
