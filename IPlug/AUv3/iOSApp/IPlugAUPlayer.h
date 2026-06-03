 /*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#pragma once

#import <AVFoundation/AVFoundation.h>

#include "config.h"

@interface IPlugAUPlayer : NSObject

@property (assign) AUAudioUnit* currentAudioUnit;

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
/** Enable/disable Ableton Link networking and host timing publication. */
@property (nonatomic, getter=isLinkEnabled) BOOL linkEnabled;
/** Enable/disable sharing Link start/stop state with peers. */
@property (nonatomic, getter=isLinkStartStopSyncEnabled) BOOL linkStartStopSyncEnabled;
/** Current number of connected Link peers. */
@property (nonatomic, readonly) NSUInteger linkPeerCount;
/** Current Link tempo in BPM. */
@property (nonatomic) double linkTempo;
/** Current Link transport state. */
@property (nonatomic, getter=isLinkPlaying) BOOL linkPlaying;
/** Quantum (beats per bar) used when mapping the Link phase. Default: 4.0. */
@property (nonatomic) double quantum;
#endif

- (instancetype) initWithComponentType:(UInt32) unitComponentType;

- (void) loadAudioUnitWithComponentDescription:(AudioComponentDescription) desc completion:(void (^) (void)) completionBlock;
@end
