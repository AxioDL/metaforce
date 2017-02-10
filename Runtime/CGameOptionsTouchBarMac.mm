#include <AppKit/AppKit.h>
#include "CGameOptionsTouchBar.hpp"
#include "CGameOptions.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"
#include "GuiSys/CStringTable.hpp"

#if !__has_feature(objc_arc)
#error ARC Required
#endif

static NSColor* BlueConfirm()
{
    return [NSColor colorWithSRGBRed:0/255.f green:130/255.f blue:215/255.f alpha:1.f];
}

@interface GameOptionsTouchBar : NSObject <NSTouchBarDelegate>
{
@public
    urde::CStringTable* _pauseScreenStrg;
    urde::CGameOptionsTouchBar::EAction _action;
    std::pair<int, int> _selection;
    int _value, _pendingValue;
}
-(IBAction)onBack:(id)sender;
-(IBAction)onSlide:(id)sender;
-(IBAction)onSet0:(id)sender;
-(IBAction)onSet1:(id)sender;
-(IBAction)onSet2:(id)sender;
-(IBAction)onLeft:(id)sender;
-(IBAction)onRight:(id)sender;
@end

@implementation GameOptionsTouchBar
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"optionsGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"optionsGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"optionsGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items;
        if (_selection.first == -1)
        {
            items = [NSMutableArray arrayWithCapacity:5];
            [items addObject:@"back"];
            for (int i=0 ; i<4 ; ++i)
                [items addObject:[NSString stringWithFormat:@"left/%d", i]];
        }
        else if (_selection.second == -1)
        {
            const std::pair<int, const urde::SGameOption*>& opt = urde::GameOptionsRegistry[_selection.first];
            items = [NSMutableArray arrayWithCapacity:opt.first+1];
            [items addObject:@"back"];
            for (int i=0 ; i<opt.first ; ++i)
                [items addObject:[NSString stringWithFormat:@"right/%d", i]];
        }
        else
        {
            const std::pair<int, const urde::SGameOption*>& opt = urde::GameOptionsRegistry[_selection.first];
            const urde::SGameOption& subopt = opt.second[_selection.second];
            if (subopt.type == urde::EOptionType::Float)
                items = @[@"back", @"value"];
            else if (subopt.type == urde::EOptionType::DoubleEnum)
                items = @[@"back", @"label", @"double0", @"double1"];
            else if (subopt.type == urde::EOptionType::TripleEnum)
                items = @[@"back", @"label", @"triple0", @"triple1", @"triple2"];
        }
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"back"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarGoBackTemplate]
                                              target:self action:@selector(onBack:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"label"])
    {
        const std::pair<int, const urde::SGameOption*>& opt = urde::GameOptionsRegistry[_selection.first];
        const urde::SGameOption& subopt = opt.second[_selection.second];

        const char16_t* cStr = _pauseScreenStrg->GetString(subopt.stringId);
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSTextField* label = [NSTextField labelWithString:[str stringByAppendingString:@":"]];
        item.view = label;
        return item;
    }
    else if ([identifier isEqualToString:@"value"])
    {
        const std::pair<int, const urde::SGameOption*>& opt = urde::GameOptionsRegistry[_selection.first];
        const urde::SGameOption& subopt = opt.second[_selection.second];

        const char16_t* cStr = _pauseScreenStrg->GetString(subopt.stringId);
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSSliderTouchBarItem* item = [[NSSliderTouchBarItem alloc] initWithIdentifier:identifier];
        NSSlider* slider = [NSSlider sliderWithValue:_value minValue:subopt.minVal maxValue:subopt.maxVal
                                              target:nil action:nil];
        item.target = self;
        item.action = @selector(onSlide:);
        item.slider = slider;
        item.label = str;
        return item;
    }
    else if ([identifier isEqualToString:@"double0"])
    {
        const char16_t* cStr = _pauseScreenStrg->GetString(95); // Off
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onSet0:)];
        if (_value == 0)
            button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"double1"])
    {
        const char16_t* cStr = _pauseScreenStrg->GetString(94); // On
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onSet1:)];
        if (_value == 1)
            button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"triple0"])
    {
        const char16_t* cStr = _pauseScreenStrg->GetString(96); // Mono
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onSet0:)];
        if (_value == 0)
            button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"triple1"])
    {
        const char16_t* cStr = _pauseScreenStrg->GetString(97); // Stereo
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onSet1:)];
        if (_value == 1)
            button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"triple2"])
    {
        const char16_t* cStr = _pauseScreenStrg->GetString(98); // Dolby
        NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onSet2:)];
        if (_value == 2)
            button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else
    {
        NSArray<NSString*>* pc = [identifier pathComponents];
        if ([pc count] == 2)
        {
            NSString* first = [pc objectAtIndex:0];
            if ([first isEqualToString:@"left"])
            {
                auto idx = strtoul([[pc objectAtIndex:1] UTF8String], nullptr, 10);
                const char16_t* cStr = _pauseScreenStrg->GetString(16+idx);
                NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

                NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
                NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onLeft:)];
                button.tag = idx;
                item.view = button;
                return item;
            }
            else if ([first isEqualToString:@"right"])
            {
                const std::pair<int, const urde::SGameOption*>& opt = urde::GameOptionsRegistry[_selection.first];
                auto idx = strtoul([[pc objectAtIndex:1] UTF8String], nullptr, 10);
                const urde::SGameOption& subopt = opt.second[idx];
                const char16_t* cStr = _pauseScreenStrg->GetString(subopt.stringId);
                NSString* str = [NSString stringWithUTF8String:hecl::Char16ToUTF8(cStr).c_str()];

                NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
                NSButton* button = [NSButton buttonWithTitle:str target:self action:@selector(onRight:)];
                button.tag = idx;
                item.view = button;
                return item;
            }
        }
    }
    return nil;
}
-(IBAction)onBack:(id)sender
{
    _action = urde::CGameOptionsTouchBar::EAction::Back;
}
-(IBAction)onSlide:(id)sender
{
    _pendingValue = [((NSSliderTouchBarItem*)sender).slider intValue];
    _action = urde::CGameOptionsTouchBar::EAction::ValueChange;
}
-(IBAction)onSet0:(id)sender
{
    _pendingValue = 0;
    _action = urde::CGameOptionsTouchBar::EAction::ValueChange;
}
-(IBAction)onSet1:(id)sender
{
    _pendingValue = 1;
    _action = urde::CGameOptionsTouchBar::EAction::ValueChange;
}
-(IBAction)onSet2:(id)sender
{
    _pendingValue = 2;
    _action = urde::CGameOptionsTouchBar::EAction::ValueChange;
}
-(IBAction)onLeft:(id)sender
{
    _selection.first = ((NSButton*)sender).tag;
    _action = urde::CGameOptionsTouchBar::EAction::Advance;
}
-(IBAction)onRight:(id)sender
{
    _selection.second = ((NSButton*)sender).tag;
    _action = urde::CGameOptionsTouchBar::EAction::Advance;
}
@end

namespace urde
{

class CGameOptionsTouchBarMac : public CGameOptionsTouchBar
{
    TLockedToken<CStringTable> m_pauseScreen;
    GameOptionsTouchBar* m_touchBar;
    bool m_initialized = false;
public:
    CGameOptionsTouchBarMac()
    {
        m_pauseScreen = g_SimplePool->GetObj("STRG_PauseScreen");
        m_touchBar = [GameOptionsTouchBar new];
        m_touchBar->_pauseScreenStrg = m_pauseScreen.GetObj();
        m_touchBar->_selection = std::make_pair(-1, -1);
        m_touchBar->_value = -1;
    }
    EAction PopAction()
    {
        if (m_touchBar->_action != EAction::None)
        {
            EAction action = m_touchBar->_action;
            m_touchBar->_action = EAction::None;
            return action;
        }
        return EAction::None;
    }
    void GetSelection(int& left, int& right, int& value)
    {
        left = m_touchBar->_selection.first;
        right = m_touchBar->_selection.second;
        value = m_touchBar->_pendingValue;
    }
    void SetSelection(int left, int right, int value)
    {
        if (m_initialized &&
            left == m_touchBar->_selection.first &&
            right == m_touchBar->_selection.second &&
            value == m_touchBar->_value)
            return;
        m_initialized = true;
        m_touchBar->_selection = std::make_pair(left, right);
        m_touchBar->_value = value;
        g_Main->GetMainWindow()->setTouchBarProvider((__bridge_retained void*)m_touchBar);
    }
};

std::unique_ptr<CGameOptionsTouchBar> NewGameOptionsTouchBar()
{
    return std::make_unique<CGameOptionsTouchBarMac>();
}

}
