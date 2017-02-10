#include <AppKit/AppKit.h>
#include "CFrontEndUITouchBar.hpp"
#include "GameGlobalObjects.hpp"
#include "MP1/MP1.hpp"

#if !__has_feature(objc_arc)
#error ARC Required
#endif

extern "C" uint8_t START_BUTTON_2X[];
extern "C" size_t START_BUTTON_2X_SZ;

static NSColor* BlueConfirm()
{
    return [NSColor colorWithSRGBRed:0/255.f green:130/255.f blue:215/255.f alpha:1.f];
}

static NSColor* NormalModeColor()
{
    return [NSColor colorWithSRGBRed:0/255.f green:130/255.f blue:0/255.f alpha:1.f];
}

static NSColor* HardModeColor()
{
    return [NSColor redColor];
}

static NSColor* FileColor(const urde::CFrontEndUITouchBar::SFileSelectDetail& detail)
{
    switch (detail.state)
    {
    case urde::CFrontEndUITouchBar::EFileState::New:
        return [NSColor darkGrayColor];
    case urde::CFrontEndUITouchBar::EFileState::Normal:
        return NormalModeColor();
    case urde::CFrontEndUITouchBar::EFileState::Hard:
        return HardModeColor();
    }
}

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
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"pressStart"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"pressStart"])
    {
        NSData* imgData = [NSData dataWithBytesNoCopy:START_BUTTON_2X length:START_BUTTON_2X_SZ freeWhenDone:NO];
        NSImage* img = [[NSImage alloc] initWithData:imgData];
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Start" image:img target:self action:@selector(onPressStart:)];
        button.imageHugsTitle = YES;
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onPressStart:(id)sender
{
    _startPressed = YES;
}
@end

@interface FrontEndUITouchBarProceedBack : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::EAction _action;
}
-(IBAction)onBack:(id)sender;
-(IBAction)onProceed:(id)sender;
@end

@implementation FrontEndUITouchBarProceedBack
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"proceedBackGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"proceedBackGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"proceedBackGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"back", @"proceed"];
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
    else if ([identifier isEqualToString:@"proceed"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarGoForwardTemplate]
                                              target:self action:@selector(onProceed:)];
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onBack:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Back;
}
-(IBAction)onProceed:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Confirm;
}
@end

@interface FrontEndUITouchBarStartOptions : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::EAction _action;
    BOOL _normalBeat;
}
-(IBAction)onStart:(id)sender;
-(IBAction)onNormal:(id)sender;
-(IBAction)onHard:(id)sender;
-(IBAction)onOptions:(id)sender;
-(IBAction)onCancel:(id)sender;
@end

@implementation FrontEndUITouchBarStartOptions
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"startOptionsGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"startOptionsGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"startOptionsGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = _normalBeat ? @[@"cancel", @"normal", @"hard", @"options"] :
                                 @[@"cancel", @"start", @"options"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"start"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Start" target:self action:@selector(onStart:)];
        button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"normal"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Normal Mode" target:self action:@selector(onNormal:)];
        button.bezelColor = NormalModeColor();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"hard"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Hard Mode" target:self action:@selector(onHard:)];
        button.bezelColor = HardModeColor();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"options"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Options" target:self action:@selector(onOptions:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"cancel"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarGoBackTemplate]
                                              target:self action:@selector(onCancel:)];
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onStart:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Start;
}
-(IBAction)onNormal:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Normal;
}
-(IBAction)onHard:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Hard;
}
-(IBAction)onOptions:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Options;
}
-(IBAction)onCancel:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Back;
}
@end

@interface FrontEndUITouchBarEraseBack : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::EAction _action;
}
-(IBAction)onCancel:(id)sender;
-(IBAction)onErase:(id)sender;
@end

@implementation FrontEndUITouchBarEraseBack
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"eraseBackGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"eraseBackGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"eraseBackGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"cancel", @"erase"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"cancel"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Cancel" target:self action:@selector(onCancel:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"erase"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Erase" target:self action:@selector(onErase:)];
        button.bezelColor = [NSColor redColor];
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onCancel:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Back;
}
-(IBAction)onErase:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Confirm;
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
-(IBAction)onErase:(id)sender;
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
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"fileA", @"fileB", @"fileC", @"erase", @"fusionBonus", @"imageGallery"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"fileA"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[0], 'A') target:self action:@selector(onFileA:)];
        button.bezelColor = FileColor(_details[0]);
        button.enabled = !_eraseGame || _details[0].state != urde::CFrontEndUITouchBar::EFileState::New;
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"fileB"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[1], 'B') target:self action:@selector(onFileB:)];
        button.bezelColor = FileColor(_details[1]);
        button.enabled = !_eraseGame || _details[1].state != urde::CFrontEndUITouchBar::EFileState::New;
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"fileC"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:GetFileSelectTitle(_details[2], 'C') target:self action:@selector(onFileC:)];
        button.bezelColor = FileColor(_details[2]);
        button.enabled = !_eraseGame || _details[2].state != urde::CFrontEndUITouchBar::EFileState::New;
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"erase"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        if (!_eraseGame)
        {
            NSButton* button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarDeleteTemplate]
                                                  target:self action:@selector(onErase:)];
            bool hasSave = false;
            for (int i=0 ; i<3 ; ++i)
                if (_details[i].state != urde::CFrontEndUITouchBar::EFileState::New)
                {
                    hasSave = true;
                    break;
                }
            button.enabled = hasSave;
            item.view = button;
        }
        else
        {
            NSButton* button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarGoBackTemplate]
                                                  target:self action:@selector(onErase:)];
            item.view = button;
        }
        return item;
    }
    else if ([identifier isEqualToString:@"fusionBonus"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Bonuses" target:self action:@selector(onFusionBonus:)];
        button.enabled = !_eraseGame;
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"imageGallery"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Gallery" target:self action:@selector(onImageGallery:)];
        button.enabled = !_eraseGame && _galleryActive;
        item.view = button;
        return item;
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
-(IBAction)onErase:(id)sender
{
    if (!_eraseGame)
        _action = urde::CFrontEndUITouchBar::EAction::Erase;
    else
        _action = urde::CFrontEndUITouchBar::EAction::Back;
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

@interface FrontEndUITouchBarNoCardSelect : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::EAction _action;
    BOOL _galleryActive;
}
-(IBAction)onStart:(id)sender;
-(IBAction)onOptions:(id)sender;
-(IBAction)onFusionBonus:(id)sender;
-(IBAction)onImageGallery:(id)sender;
@end

@implementation FrontEndUITouchBarNoCardSelect
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"noCardSelectGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"noCardSelectGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"noCardSelectGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"start", @"fusionBonus", @"options", @"imageGallery"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"start"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Start" target:self action:@selector(onStart:)];
        button.bezelColor = BlueConfirm();
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"options"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Options" target:self action:@selector(onOptions:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"fusionBonus"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Fusion Bonuses" target:self action:@selector(onFusionBonus:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"imageGallery"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Image Gallery" target:self action:@selector(onImageGallery:)];
        button.enabled = _galleryActive;
        item.view = button;
        return item;
    }
    return nil;
}
-(IBAction)onStart:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Start;
}
-(IBAction)onOptions:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Options;
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

@interface FrontEndUITouchBarFusionBonus : NSObject <NSTouchBarDelegate>
{
@public
    urde::CFrontEndUITouchBar::EAction _action;
    BOOL _fusionSuitActive;
}
-(IBAction)onNESMetroid:(id)sender;
-(IBAction)onFusionSuit:(id)sender;
-(IBAction)onBack:(id)sender;
@end

@implementation FrontEndUITouchBarFusionBonus
- (NSTouchBar*)makeTouchBar
{
    NSTouchBar* touchBar = [NSTouchBar new];
    touchBar.delegate = self;
    id items = @[@"fusionBonusGroup"];
    touchBar.customizationRequiredItemIdentifiers = items;
    touchBar.defaultItemIdentifiers = items;
    touchBar.principalItemIdentifier = @"fusionBonusGroup";
    return touchBar;
}
-(NSTouchBarItem*)touchBar:(NSTouchBar*)touchBar
     makeItemForIdentifier:(NSTouchBarItemIdentifier)identifier
{
    if ([identifier isEqualToString:@"fusionBonusGroup"])
    {
        NSGroupTouchBarItem* item = [[NSGroupTouchBarItem alloc] initWithIdentifier:identifier];
        NSTouchBar* touchBar = [NSTouchBar new];
        touchBar.delegate = self;
        id items = @[@"back", @"NESMetroid", @"fusionSuit"];
        touchBar.customizationRequiredItemIdentifiers = items;
        touchBar.defaultItemIdentifiers = items;
        item.groupTouchBar = touchBar;
        return item;
    }
    else if ([identifier isEqualToString:@"NESMetroid"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Play NES Metroid" target:self action:@selector(onNESMetroid:)];
        item.view = button;
        return item;
    }
    else if ([identifier isEqualToString:@"fusionSuit"])
    {
        NSCustomTouchBarItem* item = [[NSCustomTouchBarItem alloc] initWithIdentifier:identifier];
        NSButton* button = [NSButton buttonWithTitle:@"Fusion Suit" target:self action:@selector(onFusionSuit:)];
        if (_fusionSuitActive)
            button.bezelColor = BlueConfirm();
        item.view = button;
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
    return nil;
}
-(IBAction)onNESMetroid:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::NESMetroid;
}
-(IBAction)onFusionSuit:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::FusionSuit;
}
-(IBAction)onBack:(id)sender
{
    _action = urde::CFrontEndUITouchBar::EAction::Back;
}
@end

namespace urde
{

class CFrontEndUITouchBarMac : public CFrontEndUITouchBar
{
    FrontEndUITouchBarPressStart* m_pressStartBar;
    FrontEndUITouchBarProceedBack* m_proceedBackBar;
    FrontEndUITouchBarStartOptions* m_startOptions;
    FrontEndUITouchBarEraseBack* m_eraseBack;
    FrontEndUITouchBarFileSelect* m_fileSelectBar;
    FrontEndUITouchBarNoCardSelect* m_noCardSelectBar;
    FrontEndUITouchBarFusionBonus* m_fusionBonusBar;

    void Activate()
    {
        id provider = nil;
        switch (m_phase)
        {
        case EPhase::PressStart:
            provider = m_pressStartBar;
            break;
        case EPhase::ProceedBack:
            provider = m_proceedBackBar;
            break;
        case EPhase::StartOptions:
            provider = m_startOptions;
            break;
        case EPhase::EraseBack:
            provider = m_eraseBack;
            break;
        case EPhase::FileSelect:
            provider = m_fileSelectBar;
            break;
        case EPhase::NoCardSelect:
            provider = m_noCardSelectBar;
            break;
        case EPhase::FusionBonus:
            provider = m_fusionBonusBar;
            break;
        default: break;
        }
        g_Main->GetMainWindow()->setTouchBarProvider((__bridge_retained void*)provider);
    }

public:
    CFrontEndUITouchBarMac()
    {
        m_pressStartBar = [FrontEndUITouchBarPressStart new];
        m_proceedBackBar = [FrontEndUITouchBarProceedBack new];
        m_startOptions = [FrontEndUITouchBarStartOptions new];
        m_eraseBack = [FrontEndUITouchBarEraseBack new];
        m_fileSelectBar = [FrontEndUITouchBarFileSelect new];
        m_noCardSelectBar = [FrontEndUITouchBarNoCardSelect new];
        m_fusionBonusBar = [FrontEndUITouchBarFusionBonus new];
    }
    void SetPhase(EPhase ph)
    {
        m_phase = ph;
        Activate();
    }
    EPhase GetPhase()
    {
        return m_phase;
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
    void SetNoCardSelectPhase(bool galleryActive)
    {
        m_noCardSelectBar->_galleryActive = galleryActive;
        m_phase = EPhase::NoCardSelect;
        Activate();
    }
    void SetFusionBonusPhase(bool fusionSuitActive)
    {
        m_fusionBonusBar->_fusionSuitActive = fusionSuitActive;
        m_phase = EPhase::FusionBonus;
        Activate();
    }
    void SetStartOptionsPhase(bool normalBeat)
    {
        m_startOptions->_normalBeat = normalBeat;
        m_phase = EPhase::StartOptions;
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
        case EPhase::ProceedBack:
            if (m_proceedBackBar->_action != EAction::None)
            {
                EAction action = m_proceedBackBar->_action;
                m_proceedBackBar->_action = EAction::None;
                return action;
            }
            break;
        case EPhase::StartOptions:
            if (m_startOptions->_action != EAction::None)
            {
                EAction action = m_startOptions->_action;
                m_startOptions->_action = EAction::None;
                return action;
            }
            break;
        case EPhase::EraseBack:
            if (m_eraseBack->_action != EAction::None)
            {
                EAction action = m_eraseBack->_action;
                m_eraseBack->_action = EAction::None;
                return action;
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
        case EPhase::NoCardSelect:
            if (m_noCardSelectBar->_action != EAction::None)
            {
                EAction action = m_noCardSelectBar->_action;
                m_noCardSelectBar->_action = EAction::None;
                return action;
            }
            break;
        case EPhase::FusionBonus:
            if (m_fusionBonusBar->_action != EAction::None)
            {
                EAction action = m_fusionBonusBar->_action;
                m_fusionBonusBar->_action = EAction::None;
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
