 /*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#pragma once

#import <AVFoundation/AVFoundation.h>

#include "config.h"

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
#import <ABLLink.h>
#endif

@interface IPlugAUPlayer : NSObject

@property (assign) AUAudioUnit* currentAudioUnit;

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
/** Reference to the Ableton Link instance for tempo/transport sync */
@property (nonatomic, readonly) ABLLinkRef linkRef;
/** Quantum (beats per bar) for Link sync. Default: 4.0 */
@property (nonatomic) double quantum;
#endif

- (instancetype) initWithComponentType:(UInt32) unitComponentType;

- (void) loadAudioUnitWithComponentDescription:(AudioComponentDescription) desc completion:(void (^) (void)) completionBlock;
@end
