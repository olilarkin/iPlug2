 /*
 ==============================================================================
 
 This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
 
 See LICENSE.txt for  more info.
 
 ==============================================================================
*/

#import "IPlugAUPlayer.h"
#include "IPlugConstants.h"
#include "config.h"
#include <algorithm>
#include <cmath>

#if !__has_feature(objc_arc)
#error This file must be compiled with Arc. Use -fobjc-arc flag
#endif

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
#include <ableton/Link.hpp>
#include <atomic>
#include <chrono>
#include <memory>

static std::chrono::microseconds LinkOutputHostTime(const ableton::Link& link, int64_t outputLatencyMicros)
{
  using namespace std::chrono;
  return link.clock().micros() + microseconds{outputLatencyMicros > 0 ? outputLatencyMicros : 0};
}

static NSInteger LinkSampleOffsetToNextBeat(double beat, double tempo, double sampleRate)
{
  if (tempo <= 0.0 || sampleRate <= 0.0)
    return 0;

  const double phase = beat - std::floor(beat);
  const double beatsToNext = phase <= 1.0e-9 ? 0.0 : 1.0 - phase;
  return (NSInteger) std::llround(beatsToNext * 60.0 * sampleRate / tempo);
}

static double LinkSamplePosition(double beat, double tempo, double sampleRate)
{
  if (tempo <= 0.0 || sampleRate <= 0.0)
    return 0.0;

  return beat * 60.0 * sampleRate / tempo;
}

static NSString* const kIPlugLinkEnabledKey = @"IPlugAUPlayer.LinkEnabled";
static NSString* const kIPlugLinkStartStopSyncEnabledKey = @"IPlugAUPlayer.LinkStartStopSyncEnabled";
static NSString* const kIPlugLinkTempoKey = @"IPlugAUPlayer.LinkTempo";
static NSString* const kIPlugLinkQuantumKey = @"IPlugAUPlayer.LinkQuantum";

static double LinkClampedTempo(double tempo)
{
  if (!std::isfinite(tempo))
    return iplug::DEFAULT_TEMPO;

  return std::min(std::max(tempo, 20.0), 999.0);
}

static double LinkStoredDouble(NSString* key, double defaultValue)
{
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  if ([defaults objectForKey:key] == nil)
    return defaultValue;

  return [defaults doubleForKey:key];
}

static BOOL LinkStoredBool(NSString* key, BOOL defaultValue)
{
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  if ([defaults objectForKey:key] == nil)
    return defaultValue;

  return [defaults boolForKey:key];
}
#endif

bool isInstrument()
{
#if PLUG_TYPE == 1
  return YES;
#else
  return NO;
#endif
}

@implementation IPlugAUPlayer
{
  AVAudioEngine* engine;
  AVAudioUnit* avAudioUnit;
  UInt32 componentType;
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  std::unique_ptr<ableton::Link> _link;
  std::atomic<std::size_t> _linkPeerCount;
  std::atomic<double> _linkTempo;
  std::atomic<bool> _linkIsPlaying;
  std::atomic<double> _linkQuantum;
  std::atomic<int64_t> _linkOutputLatencyMicros;
  std::atomic<double> _linkSampleRate;
#endif
}

- (instancetype) initWithComponentType: (UInt32) unitComponentType
{
  self = [super init];
  
  if (self)
  {
    engine = [[AVAudioEngine alloc] init];
    componentType = unitComponentType;
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
    const double initialTempo = LinkClampedTempo(LinkStoredDouble(kIPlugLinkTempoKey, iplug::DEFAULT_TEMPO));
    const double initialQuantum = LinkStoredDouble(kIPlugLinkQuantumKey, 4.0);
    const BOOL linkEnabled = LinkStoredBool(kIPlugLinkEnabledKey, YES);
    const BOOL startStopSyncEnabled = LinkStoredBool(kIPlugLinkStartStopSyncEnabledKey, NO);

    _linkPeerCount.store(0, std::memory_order_relaxed);
    _linkTempo.store(initialTempo, std::memory_order_relaxed);
    _linkIsPlaying.store(false, std::memory_order_relaxed);
    _linkQuantum.store(std::isfinite(initialQuantum) && initialQuantum > 0.0 ? initialQuantum : 4.0, std::memory_order_relaxed);
    _linkOutputLatencyMicros.store(0, std::memory_order_relaxed);
    _linkSampleRate.store(iplug::DEFAULT_SAMPLE_RATE, std::memory_order_relaxed);

    _link = std::make_unique<ableton::Link>(initialTempo);
    IPlugAUPlayer* owner = self;
    _link->setNumPeersCallback([owner](std::size_t peers) {
      owner->_linkPeerCount.store(peers, std::memory_order_relaxed);
    });
    _link->setTempoCallback([owner](double tempo) {
      owner->_linkTempo.store(tempo, std::memory_order_relaxed);
    });
    _link->setStartStopCallback([owner](bool isPlaying) {
      owner->_linkIsPlaying.store(isPlaying, std::memory_order_relaxed);
    });
    _link->enableStartStopSync(startStopSyncEnabled);
    _link->enable(linkEnabled);
#endif
  }

  return self;
}

- (void) loadAudioUnitWithComponentDescription:(AudioComponentDescription)desc
                                   completion:(void (^) (void))completionBlock
{
  [AVAudioUnit instantiateWithComponentDescription:desc options:0
                                 completionHandler:^(AVAudioUnit* __nullable audioUnit, NSError* __nullable error) {
                                   [self onAudioUnitInstantiated:audioUnit error:error completion:completionBlock];
                                 }];
}

- (void) onAudioUnitInstantiated:(AVAudioUnit* __nullable) audioUnit error:(NSError* __nullable) error completion:(void (^) (void))completionBlock
{
  if (audioUnit == nil)
    return;
  
  avAudioUnit = audioUnit;
  
  [engine attachNode:avAudioUnit];

  self.currentAudioUnit = avAudioUnit.AUAudioUnit;
  

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  [self setupLinkContextBlocks];
#endif

  [self setupSession];
    
#ifdef _DEBUG
  [self printEngineInfo];
  [self printSessionInfo];
#endif
  
  [self makeEngineConnections];
  [self addNotifications];
  
  AVAudioSession* session = [AVAudioSession sharedInstance];

  if (![session setActive:TRUE error: &error])
  {
    NSLog(@"Error setting session active: %@", [error localizedDescription]);
  }

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  [self updateLinkTimingCache];
#endif
  
  if (![engine startAndReturnError: &error])
  {
    NSLog(@"engine failed to start: %@", error);
  }

  completionBlock();
}

- (void) dealloc
{
  [[NSNotificationCenter defaultCenter] removeObserver: self];
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
  if (_link)
  {
    _link->enable(false);
    _link->setNumPeersCallback([](std::size_t) {});
    _link->setTempoCallback([](double) {});
    _link->setStartStopCallback([](bool) {});
    _link.reset();
  }
#endif
}

#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
- (BOOL) isLinkEnabled
{
  return _link ? _link->isEnabled() : NO;
}

- (void) setLinkEnabled:(BOOL)enabled
{
  if (_link)
    _link->enable(enabled);

  [[NSUserDefaults standardUserDefaults] setBool:enabled forKey:kIPlugLinkEnabledKey];
}

- (BOOL) isLinkStartStopSyncEnabled
{
  return _link ? _link->isStartStopSyncEnabled() : NO;
}

- (void) setLinkStartStopSyncEnabled:(BOOL)enabled
{
  if (_link)
    _link->enableStartStopSync(enabled);

  [[NSUserDefaults standardUserDefaults] setBool:enabled forKey:kIPlugLinkStartStopSyncEnabledKey];
}

- (NSUInteger) linkPeerCount
{
  return (NSUInteger) _linkPeerCount.load(std::memory_order_relaxed);
}

- (double) linkTempo
{
  if (_link)
    return _link->captureAppSessionState().tempo();

  return _linkTempo.load(std::memory_order_relaxed);
}

- (void) setLinkTempo:(double)tempo
{
  const double clampedTempo = LinkClampedTempo(tempo);

  if (_link)
  {
    const auto time = _link->clock().micros();
    auto sessionState = _link->captureAppSessionState();
    sessionState.setTempo(clampedTempo, time);
    _link->commitAppSessionState(sessionState);
  }

  _linkTempo.store(clampedTempo, std::memory_order_relaxed);
  [[NSUserDefaults standardUserDefaults] setDouble:clampedTempo forKey:kIPlugLinkTempoKey];
}

- (BOOL) isLinkPlaying
{
  if (_link)
    return _link->captureAppSessionState().isPlaying() ? YES : NO;

  return _linkIsPlaying.load(std::memory_order_relaxed) ? YES : NO;
}

- (void) setLinkPlaying:(BOOL)playing
{
  if (!_link)
    return;

  const auto time = _link->clock().micros();
  auto sessionState = _link->captureAppSessionState();
  sessionState.setIsPlayingAndRequestBeatAtTime(playing, time, 0.0, self.quantum);
  _link->commitAppSessionState(sessionState);
  _linkIsPlaying.store(playing, std::memory_order_relaxed);
}

- (double) quantum
{
  return _linkQuantum.load(std::memory_order_relaxed);
}

- (void) setQuantum:(double)quantum
{
  if (std::isfinite(quantum) && quantum > 0.0)
  {
    _linkQuantum.store(quantum, std::memory_order_relaxed);
    [[NSUserDefaults standardUserDefaults] setDouble:quantum forKey:kIPlugLinkQuantumKey];
  }
}

- (void) updateLinkTimingCache
{
  AVAudioSession* session = [AVAudioSession sharedInstance];
  const double sampleRate = session.sampleRate > 0.0 ? session.sampleRate : iplug::DEFAULT_SAMPLE_RATE;
  const int64_t outputLatencyMicros = (int64_t) std::llround(session.outputLatency * 1.0e6);

  _linkSampleRate.store(sampleRate, std::memory_order_relaxed);
  _linkOutputLatencyMicros.store(outputLatencyMicros, std::memory_order_relaxed);
}

- (void) setupLinkContextBlocks
{
  __weak IPlugAUPlayer* weakSelf = self;

  self.currentAudioUnit.musicalContextBlock = ^BOOL(double* tempo,
                                                     double* timeSignatureNumerator,
                                                     NSInteger* timeSignatureDenominator,
                                                     double* currentBeatPosition,
                                                     NSInteger* sampleOffsetToNextBeat,
                                                     double* currentMeasureDownbeatPosition) {
    IPlugAUPlayer* strongSelf = weakSelf;
    if (!strongSelf) return NO;
    ableton::Link* link = strongSelf->_link.get();
    if (!link || !link->isEnabled()) return NO;

    auto sessionState = link->captureAudioSessionState();
    auto hostTimeAtOutput = LinkOutputHostTime(*link, strongSelf->_linkOutputLatencyMicros.load(std::memory_order_relaxed));
    double q = strongSelf.quantum;
    double linkTempo = sessionState.tempo();
    double beat = sessionState.beatAtTime(hostTimeAtOutput, q);
    double sampleRate = strongSelf->_linkSampleRate.load(std::memory_order_relaxed);

    if (tempo) *tempo = linkTempo;
    if (timeSignatureNumerator) *timeSignatureNumerator = 4;
    if (timeSignatureDenominator) *timeSignatureDenominator = 4;
    if (currentBeatPosition) *currentBeatPosition = beat;
    if (sampleOffsetToNextBeat) *sampleOffsetToNextBeat = LinkSampleOffsetToNextBeat(beat, linkTempo, sampleRate);
    if (currentMeasureDownbeatPosition) *currentMeasureDownbeatPosition = std::floor(beat / q) * q;

    return YES;
  };

  self.currentAudioUnit.transportStateBlock = ^BOOL(AUHostTransportStateFlags* transportStateFlags,
                                                     double* currentSamplePosition,
                                                     double* cycleStartBeatPosition,
                                                     double* cycleEndBeatPosition) {
    IPlugAUPlayer* strongSelf = weakSelf;
    if (!strongSelf) return NO;
    ableton::Link* link = strongSelf->_link.get();
    if (!link || !link->isEnabled()) return NO;

    auto sessionState = link->captureAudioSessionState();
    auto hostTimeAtOutput = LinkOutputHostTime(*link, strongSelf->_linkOutputLatencyMicros.load(std::memory_order_relaxed));
    double q = strongSelf.quantum;
    double linkTempo = sessionState.tempo();
    double beat = sessionState.beatAtTime(hostTimeAtOutput, q);
    bool isPlaying = sessionState.isPlaying();
    double sampleRate = strongSelf->_linkSampleRate.load(std::memory_order_relaxed);

    if (transportStateFlags) {
      *transportStateFlags = 0;
      if (isPlaying) {
        *transportStateFlags |= AUHostTransportStateMoving;
      }
    }

    if (currentSamplePosition) *currentSamplePosition = LinkSamplePosition(beat, linkTempo, sampleRate);

    if (cycleStartBeatPosition) *cycleStartBeatPosition = 0;
    if (cycleEndBeatPosition) *cycleEndBeatPosition = 0;

    return YES;
  };
}
#endif

- (void) restartAudioEngine
{
  [engine stop];

  NSError *error = nil;
  
  if (![engine startAndReturnError:&error])
  {
    NSLog(@"Error re-starting audio engine: %@", error);
  }
  else
  {
#if defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
    [self updateLinkTimingCache];
#endif
    [self printSessionInfo];
  }
}

- (void) setupSession
{
  AVAudioSession* session = [AVAudioSession sharedInstance];
  NSError* error = nil;

  AVAudioSessionCategoryOptions options = AVAudioSessionCategoryOptionDefaultToSpeaker | AVAudioSessionCategoryOptionAllowBluetooth;
  [session setCategory: isInstrument() ? AVAudioSessionCategoryPlayback
                                       : AVAudioSessionCategoryPlayAndRecord
                  withOptions:options error: &error];
  
  if (error)
  {
    NSLog(@"Error setting category: %@", error);
  }
  
  [session setPreferredSampleRate:iplug::DEFAULT_SAMPLE_RATE error: &error];
  
  if (error)
  {
    NSLog(@"Error setting samplerate: %@", error);
  }
  
  [session setPreferredIOBufferDuration:128.0/iplug::DEFAULT_SAMPLE_RATE error: &error];
  
  if (error)
  {
    NSLog(@"Error setting io buffer duration: %@", error);
  }
}

- (void) makeEngineConnections
{
  if (!isInstrument())
  {
    AVAudioNode* inputNode = [engine inputNode];
    AVAudioFormat* inputNodeFormat = [inputNode inputFormatForBus:0];
    
    @autoreleasepool {
      @try {
        [engine connect:inputNode to:avAudioUnit format: inputNodeFormat];
      }
      @catch (NSException *exception) {
        NSLog(@"NSException when trying to connect input node: %@, Reason: %@", exception.name, exception.reason);
      }
    }
  }
  
  auto numOutputBuses = [avAudioUnit numberOfOutputs];
  AVAudioMixerNode* mainMixer = [engine mainMixerNode];
  AVAudioFormat* pluginOutputFormat = [avAudioUnit outputFormatForBus:0];
  AVAudioNode* outputNode = [engine outputNode];

  if (numOutputBuses > 1)
  {
    // Assume all output buses are the same format
    for (int busIdx=0; busIdx<numOutputBuses; busIdx++)
    {
      [engine connect:avAudioUnit to:mainMixer fromBus: busIdx toBus:[mainMixer nextAvailableInputBus] format: pluginOutputFormat];
    }
  }
  else
  {
    [engine connect:avAudioUnit to:outputNode format: pluginOutputFormat];
  }
}

- (void) printEngineInfo
{
  if (!isInstrument())
  {
    AVAudioFormat* inputNodeFormat = [[engine inputNode] inputFormatForBus:0];
    AVAudioFormat* pluginInputFormat = [avAudioUnit inputFormatForBus:0];
    NSLog(@"Input Node SR: %i", int(inputNodeFormat.sampleRate));
    NSLog(@"Input Node Chans: %i", inputNodeFormat.channelCount);
    NSLog(@"Plugin Input SR: %i", int(pluginInputFormat.sampleRate));
    NSLog(@"Plugin Input Chans: %i", pluginInputFormat.channelCount);
  }
  
  AVAudioFormat* pluginOutputFormat = [avAudioUnit outputFormatForBus:0];
  AVAudioFormat* outputNodeFormat = [[engine outputNode] outputFormatForBus:0];
  
  NSLog(@"Plugin Output SR: %i", int(pluginOutputFormat.sampleRate));
  NSLog(@"Plugin Output Chans: %i", pluginOutputFormat.channelCount);
  NSLog(@"Output Node SR: %i", int(outputNodeFormat.sampleRate));
  NSLog(@"Output Node Chans: %i", outputNodeFormat.channelCount);
}

- (void) printSessionInfo
{
  AVAudioSession* session = [AVAudioSession sharedInstance];
  NSLog(@"Session SR: %i", int(session.sampleRate));
  NSLog(@"Session IO Buffer: %i", int((session.IOBufferDuration * session.sampleRate)+0.5));
  if (!isInstrument()) NSLog(@"Session Input Chans: %i", int(session.inputNumberOfChannels));
  NSLog(@"Session Output Chans: %i", int(session.outputNumberOfChannels));
  if (!isInstrument()) NSLog(@"Session Input Latency: %f ms", session.inputLatency * 1000.0f);
  NSLog(@"Session Output Latency: %f ms", session.outputLatency * 1000.0f);
  AVAudioSessionRouteDescription *currentRoute = [session currentRoute];
  for (AVAudioSessionPortDescription* input in currentRoute.inputs)
  {
    NSLog(@"Input Port Name: %@", input.portName);
  }
  
  for (AVAudioSessionPortDescription* output in currentRoute.outputs)
  {
    NSLog(@"Output Port Name: %@", output.portName);
  }
}

- (void) addNotifications
{
  NSNotificationCenter* notifCtr = [NSNotificationCenter defaultCenter];

  [notifCtr addObserver: self selector: @selector (onEngineConfigurationChange:) name:AVAudioEngineConfigurationChangeNotification object: engine];
}

#pragma mark Notifications
- (void) onEngineConfigurationChange: (NSNotification*) notification
{
  [self restartAudioEngine];
}

@end
