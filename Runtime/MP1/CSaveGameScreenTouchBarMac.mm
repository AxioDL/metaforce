#include <AppKit/AppKit.h>
#include "CSaveGameScreenTouchBar.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#if !__has_feature(objc_arc)
#error ARC Required
#endif

@interface SaveGameScreenTouchBar : NSObject <NSTouchBarDelegate>
{
@public
    NSString* _opts[3];
    int _opt;
}
-(IBAction)onOpt0:(id)sender;
-(IBAction)onOpt1:(id)sender;
-(IBAction)onOpt2:(id)sender;
@end

@implementation SaveGameScreenTouchBar
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"saveUIGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"saveUIGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"saveUIGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items;
        items = [NSMutableArray arrayWithCapacity:3];
        [items addObject:@"back"];
        for (int i=0 ; i<3 && _opts[i] ; ++i)
            [items addObject:[NSString stringWithFormat:@"opt%d", i]];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"opt0"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:_opts[0] target:self action:@selector(onOpt0:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"opt1"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:_opts[1] target:self action:@selector(onOpt1:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"opt2"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:_opts[2] target:self action:@selector(onOpt2:)];
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onOpt0:(id)sender
{
    _opt = 0;
}
-(IBAction)onOpt1:(id)sender
{
    _opt = 1;
}
-(IBAction)onOpt2:(id)sender
{
    _opt = 2;
}
@end

namespace urde::MP1
{

class CSaveGameScreenTouchBarMac : public CSaveGameScreenTouchBar
{
    SaveGameScreenTouchBar* m_touchBar;
public:
    CSaveGameScreenTouchBarMac()
    {
        m_touchBar = [SaveGameScreenTouchBar new];
        m_touchBar->_opt = -1;
    }
    int PopOption()
    {
        if (m_touchBar->_opt != -1)
        {
            int opt = m_touchBar->_opt;
            m_touchBar->_opt = -1;
            return opt;
        }
        return -1;
    }
    void SetUIOpts(std::u16string_view opt0,
                   std::u16string_view opt1,
                   std::u16string_view opt2)
    {
        m_touchBar->_opts[0] = opt0.size() ? [NSString stringWithUTF8String:hecl::Char16ToUTF8(opt0).c_str()] : nil;
        m_touchBar->_opts[1] = opt1.size() ? [NSString stringWithUTF8String:hecl::Char16ToUTF8(opt1).c_str()] : nil;
        m_touchBar->_opts[2] = opt2.size() ? [NSString stringWithUTF8String:hecl::Char16ToUTF8(opt2).c_str()] : nil;
        g_Main->GetMainWindow()->setTouchBarProvider((__bridge_retained void*)m_touchBar);
    }
};

std::unique_ptr<CSaveGameScreenTouchBar> NewSaveUITouchBar()
{
    return std::make_unique<CSaveGameScreenTouchBarMac>();
}

}
