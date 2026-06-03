 /*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#import "AppViewController.h"
#import "IPlugAUPlayer.h"
#import "IPlugAUAudioUnit.h"

#include "config.h"

#import "IPlugAUViewController.h"
#import <CoreAudioKit/CoreAudioKit.h>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

@interface AppViewController ()
{
  IPlugAUPlayer* player;
  IPLUG_AUVIEWCONTROLLER* pluginVC;
  IBOutlet UIView* auView;
}

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
- (void) presentLinkTempoDialogWithTempo:(double)tempo;
#endif
@end

@implementation AppViewController

- (BOOL) prefersStatusBarHidden
{
  return YES;
}

- (void) viewDidLoad
{
  [super viewDidLoad];

#if PLUG_HAS_UI
  NSString* storyBoardName = [NSString stringWithFormat:@"%s-iOS-MainInterface", PLUG_NAME];
  UIStoryboard* storyboard = [UIStoryboard storyboardWithName:storyBoardName bundle: nil];
  pluginVC = [storyboard instantiateViewControllerWithIdentifier:@"main"];
  [self addChildViewController:pluginVC];
#endif
  
  AudioComponentDescription desc;

#if PLUG_TYPE==0
#if PLUG_DOES_MIDI_IN
  desc.componentType = kAudioUnitType_MusicEffect;
#else
  desc.componentType = kAudioUnitType_Effect;
#endif
#elif PLUG_TYPE==1
  desc.componentType = kAudioUnitType_MusicDevice;
#elif PLUG_TYPE==2
  desc.componentType = 'aumi';
#endif

  desc.componentSubType = PLUG_UNIQUE_ID;
  desc.componentManufacturer = PLUG_MFR_ID;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;

  [AUAudioUnit registerSubclass: IPLUG_AUAUDIOUNIT.class asComponentDescription:desc name:@"Local AUv3" version: UINT32_MAX];

  player = [[IPlugAUPlayer alloc] initWithComponentType:desc.componentType];

  [player loadAudioUnitWithComponentDescription:desc completion:^{
    self->pluginVC.audioUnit = (IPLUG_AUAUDIOUNIT*) self->player.currentAudioUnit;

    [self embedPlugInView];
  }];
  
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotification:) name:@"LaunchBTMidiDialog" object:nil];
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(receiveNotification:) name:@"LaunchLinkSettingsDialog" object:nil];
#endif
}

- (void) receiveNotification:(NSNotification*) notification
{
  if ([notification.name isEqualToString:@"LaunchBTMidiDialog"])
  {
    NSDictionary* dict = notification.userInfo;
    NSNumber* x = (NSNumber*) dict[@"x"];
    NSNumber* y = (NSNumber*) dict[@"y"];
   
    CABTMIDICentralViewController* vc = [[CABTMIDICentralViewController alloc] init];
    UINavigationController* nc = [[UINavigationController alloc] initWithRootViewController:vc];
    nc.modalPresentationStyle = UIModalPresentationPopover;
    
    UIPopoverPresentationController* ppc = nc.popoverPresentationController;
    ppc.permittedArrowDirections = UIPopoverArrowDirectionAny;
    ppc.sourceView = self.view;
    ppc.sourceRect = CGRectMake([x floatValue], [y floatValue], 1., 1.);
    
    [self presentViewController:nc animated:YES completion:nil];
  }
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  else if ([notification.name isEqualToString:@"LaunchLinkSettingsDialog"])
  {
    if (self.presentedViewController != nil)
      return;

    NSDictionary* dict = notification.userInfo;
    NSNumber* x = (NSNumber*) dict[@"x"];
    NSNumber* y = (NSNumber*) dict[@"y"];

    const BOOL linkEnabled = player.isLinkEnabled;
    const BOOL startStopEnabled = player.isLinkStartStopSyncEnabled;
    const BOOL linkPlaying = player.isLinkPlaying;
    const double linkTempo = player.linkTempo;
    NSString* message = [NSString stringWithFormat:@"Peers: %lu\nTempo: %.1f BPM\nStart/Stop Sync: %@\nTransport: %@",
                         (unsigned long) player.linkPeerCount,
                         linkTempo,
                         startStopEnabled ? @"On" : @"Off",
                         linkPlaying ? @"Playing" : @"Stopped"];

    UIAlertController* ac = [UIAlertController alertControllerWithTitle:@"Ableton Link"
                                                                 message:message
                                                          preferredStyle:UIAlertControllerStyleActionSheet];

    [ac addAction:[UIAlertAction actionWithTitle:linkEnabled ? @"Disable Link" : @"Enable Link"
                                           style:UIAlertActionStyleDefault
                                         handler:^(UIAlertAction* action) {
                                           self->player.linkEnabled = !linkEnabled;
                                         }]];

    [ac addAction:[UIAlertAction actionWithTitle:startStopEnabled ? @"Disable Start/Stop Sync" : @"Enable Start/Stop Sync"
                                           style:UIAlertActionStyleDefault
                                         handler:^(UIAlertAction* action) {
                                           self->player.linkStartStopSyncEnabled = !startStopEnabled;
                                         }]];

    [ac addAction:[UIAlertAction actionWithTitle:@"Set Tempo..."
                                           style:UIAlertActionStyleDefault
                                         handler:^(UIAlertAction* action) {
                                           [self presentLinkTempoDialogWithTempo:linkTempo];
                                         }]];

    [ac addAction:[UIAlertAction actionWithTitle:linkPlaying ? @"Stop Transport" : @"Start Transport"
                                           style:UIAlertActionStyleDefault
                                         handler:^(UIAlertAction* action) {
                                           self->player.linkPlaying = !linkPlaying;
                                         }]];

    [ac addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];

    UIPopoverPresentationController* ppc = ac.popoverPresentationController;
    ppc.permittedArrowDirections = UIPopoverArrowDirectionAny;
    ppc.sourceView = self.view;
    ppc.sourceRect = CGRectMake([x floatValue], [y floatValue], 1., 1.);

    [self presentViewController:ac animated:YES completion:nil];
  }
#endif
}

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
- (void) presentLinkTempoDialogWithTempo:(double)tempo
{
  UIAlertController* ac = [UIAlertController alertControllerWithTitle:@"Set Link Tempo"
                                                               message:nil
                                                        preferredStyle:UIAlertControllerStyleAlert];

  [ac addTextFieldWithConfigurationHandler:^(UITextField* textField) {
    textField.keyboardType = UIKeyboardTypeDecimalPad;
    textField.text = [NSString stringWithFormat:@"%.1f", tempo];
    textField.clearButtonMode = UITextFieldViewModeWhileEditing;
  }];

  [ac addAction:[UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleCancel handler:nil]];

  [ac addAction:[UIAlertAction actionWithTitle:@"Set"
                                         style:UIAlertActionStyleDefault
                                       handler:^(UIAlertAction* action) {
                                         UITextField* textField = ac.textFields.firstObject;
                                         const double tempo = textField.text.doubleValue;
                                         if (tempo > 0.0)
                                           self->player.linkTempo = tempo;
                                       }]];

  [self presentViewController:ac animated:YES completion:nil];
}
#endif

- (void) embedPlugInView
{
#if PLUG_HAS_UI
  UIView* view = pluginVC.view;
  view.frame = auView.bounds;
  [auView addSubview: view];
#if TARGET_OS_VISION && defined(VISIONOS_TRANSPARENT_VC)
  self.view.opaque = false;
  self.view.backgroundColor = UIColor.clearColor;
#endif

  view.translatesAutoresizingMaskIntoConstraints = NO;

  NSArray* constraints = [NSLayoutConstraint constraintsWithVisualFormat: @"H:|[view]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(view)];
  [auView addConstraints: constraints];

  constraints = [NSLayoutConstraint constraintsWithVisualFormat: @"V:|[view]|" options:0 metrics:nil views:NSDictionaryOfVariableBindings(view)];
  [auView addConstraints: constraints];
#endif
}

- (UIRectEdge) preferredScreenEdgesDeferringSystemGestures
{
  return UIRectEdgeAll;
}

#if TARGET_OS_VISION && defined(VISIONOS_TRANSPARENT_VC)
- (UIContainerBackgroundStyle) preferredContainerBackgroundStyle
{
  return UIContainerBackgroundStyleHidden;
}
#endif
@end
