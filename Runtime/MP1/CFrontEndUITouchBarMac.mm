#include <AppKit/AppKit.h>
#include "CFrontEndUITouchBar.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#if !__has_feature(objc_arc)
#error ARC Required
#endif

extern "C" uint8_t START_BUTTON_2X[];
extern "C" size_t START_BUTTON_2X_SZ;

@interface FrontEndUITouchBarPressStart : NSObject <NSTouchBarDelegate>
{
@public
    BOOL _startPressed;
}
-(IBAction)onPressStart:(id)sender;
@end

@implementation FrontEndUITouchBarPressStart
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"pressStartGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"pressStartGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"pressStartGroup"])
    {
        NSGroupTouchBarItem* group = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"pressStart"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        group.groupTouchBar = touchBar;
        return group;
    }
    else if ([identifier isEqualToString:@"pressStart"])
    {
        NSData* imgData = [NSData dataWithBytesNoCopy:START_BUTTON_2X length:START_BUTTON_2X_SZ freeWhenDone:NO];
        NSImage* img = [[NSImage alloc] initWithData:imgData];
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Start" image:img target:self action:@selector(onPressStart:)];
        button.imageHugsTitle = YES;
        pressStart.view = button;
        return pressStart;
    }
    return nil;
}
-(IBAction)onPressStart:(id)sender
{
    _startPressed = YES;
}
@end

static NSString* GetFileSelectTitle(const urde::CFrontEndUITouchBar::SFileSelectDetail& detail, char letter)
{
    switch (detail.state)
    {
    case urde::CFrontEndUITouchBar::EFileState::New:
        return [NSString stringWithFormat:@"[New Game %c]", letter];
    case urde::CFrontEndUITouchBar::EFileState::Normal:
        return [NSString stringWithFormat:@"[Samus %c] %d%%", letter, detail.percent];
    case urde::CFrontEndUITouchBar::EFileState::Hard:
        return [NSString stringWithFormat:@"[Hard %c] %d%%", letter, detail.percent];
    }
    return @"";
}

@interface FrontEndUITouchBarFileSelect : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::SFileSelectDetail _details[3];
    urde::CFrontEndUITouchBar::EAction _action;
    BOOL _eraseGame;
    BOOL _galleryActive;
}
-(IBAction)onFileA:(id)sender;
-(IBAction)onFileB:(id)sender;
-(IBAction)onFileC:(id)sender;
-(IBAction)onFusionBonus:(id)sender;
-(IBAction)onImageGallery:(id)sender;
@end

@implementation FrontEndUITouchBarFileSelect
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"fileSelectGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"fileSelectGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"fileSelectGroup"])
    {
        NSGroupTouchBarItem* group = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"fileA", @"fileB", @"fileC", @"fusionBonus", @"imageGallery"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        group.groupTouchBar = touchBar;
        return group;
    }
    else if ([identifier isEqualToString:@"fileA"])
    {
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[0], 'A') target:self action:@selector(onFileA:)];
        button.enabled = !_eraseGame || _details[0].state != urde::CFrontEndUITouchBar::EFileState::New;
        pressStart.view = button;
        return pressStart;
    }
    else if ([identifier isEqualToString:@"fileB"])
    {
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[1], 'B') target:self action:@selector(onFileB:)];
        button.enabled = !_eraseGame || _details[1].state != urde::CFrontEndUITouchBar::EFileState::New;
        pressStart.view = button;
        return pressStart;
    }
    else if ([identifier isEqualToString:@"fileC"])
    {
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[2], 'C') target:self action:@selector(onFileC:)];
        button.enabled = !_eraseGame || _details[2].state != urde::CFrontEndUITouchBar::EFileState::New;
        pressStart.view = button;
        return pressStart;
    }
    else if ([identifier isEqualToString:@"fusionBonus"])
    {
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Fusion Bonuses" target:self action:@selector(onFusionBonus:)];
        button.enabled = !_eraseGame;
        pressStart.view = button;
        return pressStart;
    }
    else if ([identifier isEqualToString:@"imageGallery"])
    {
        NSCustomTouchBarItem* pressStart = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Image Gallery" target:self action:@selector(onImageGallery:)];
        button.enabled = !_eraseGame && _galleryActive;
        pressStart.view = button;
        return pressStart;
    }
    return nil;
}
-(IBAction)onFileA:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::FileA;
}
-(IBAction)onFileB:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::FileB;
}
-(IBAction)onFileC:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::FileC;
}
-(IBAction)onFusionBonus:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::FusionBonus;
}
-(IBAction)onImageGallery:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::ImageGallery;
}
@end

namespace urde
{

class CFrontEndUITouchBarMac : public CFrontEndUITouchBar
{
    EPhase m_phase = EPhase::None;
    FrontEndUITouchBarPressStart* m_pressStartBar;
    FrontEndUITouchBarFileSelect* m_fileSelectBar;

    void Activate()
    {
        id provider = nil;
        switch (m_phase)
        {
        case EPhase::PressStart:
            provider = m_pressStartBar;
            break;
        case EPhase::FileSelect:
            provider = m_fileSelectBar;
            break;
        default: break;
        }
        g_Main->GetMainWindow()->setTouchBarProvider((__bridge_retained void*)provider);
    }

public:
    CFrontEndUITouchBarMac()
    {
        m_pressStartBar = [FrontEndUITouchBarPressStart new];
        m_fileSelectBar = [FrontEndUITouchBarFileSelect new];
    }
    void SetPhase(EPhase ph)
    {
        m_phase = ph;
        Activate();
    }
    void SetFileSelectPhase(const SFileSelectDetail details[3], bool eraseGame, bool galleryActive)
    {
        m_fileSelectBar->_details[0] = details[0];
        m_fileSelectBar->_details[1] = details[1];
        m_fileSelectBar->_details[2] = details[2];
        m_fileSelectBar->_eraseGame = eraseGame;
        m_fileSelectBar->_galleryActive = galleryActive;
        m_phase = EPhase::FileSelect;
        Activate();
    }
    EAction PopAction()
    {
        switch (m_phase)
        {
        case EPhase::PressStart:
            if (m_pressStartBar->_startPressed)
            {
                m_pressStartBar->_startPressed = NO;
                return EAction::Start;
            }
            break;
        case EPhase::FileSelect:
            if (m_fileSelectBar->_action != EAction::None)
            {
                EAction action = m_fileSelectBar->_action;
                m_fileSelectBar->_action = EAction::None;
                return action;
            }
            break;
        default: break;
        }
        return EAction::None;
    }
};

std::unique_ptr<CFrontEndUITouchBar> NewFrontEndUITouchBar()
{
    return std::make_unique<CFrontEndUITouchBarMac>();
}

}
