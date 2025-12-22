# Ableton Link Integration for iOS Apps

iPlug2 supports optional [Ableton Link](https://www.ableton.com/link/) integration for iOS standalone apps, enabling tempo and transport synchronization with other Link-enabled applications.

## Overview

When enabled, Link provides:
- **Tempo sync** - All connected apps share the same tempo
- **Beat/phase sync** - Apps stay aligned on the beat grid
- **Transport sync** - Play/stop state synchronized across apps (optional)

The integration works by having `IPlugAUPlayer` (the iOS standalone app host) provide Link-derived tempo and transport data to your AUv3 plugin via the standard `musicalContextBlock` and `transportStateBlock` callbacks.

## Requirements

- iOS 14.0+ (requires multicast entitlement for peer discovery)
- LinkKit.xcframework (included in `Dependencies/Build/xcframeworks/`)

## Setup

### 1. Enable in config.h

Add to your project's `config.h`:

```cpp
#define APP_ENABLE_LINK 1
```

### 2. Add LinkKit Framework

In your Xcode project's iOS App target:

1. Go to **General** > **Frameworks, Libraries, and Embedded Content**
2. Click **+** and add `LinkKit.xcframework` from `Dependencies/Build/xcframeworks/`
3. Ensure it's set to **Embed & Sign**

Add the framework search path in **Build Settings**:
```
$(IPLUG2_ROOT)/Dependencies/Build/xcframeworks
```

### 3. Configure Info.plist

For start/stop sync support, add to your iOS App's Info.plist:

```xml
<key>ABLLinkStartStopSyncSupported</key>
<true/>
```

For iOS 14+, you may need the multicast entitlement. Add to your entitlements file:
```xml
<key>com.apple.developer.networking.multicast</key>
<true/>
```

## Adding a Link Settings Button

To let users access Link settings, add a button in your plugin UI that calls `LaunchLinkSettingsDialog()`:

```cpp
// In your plugin's UI setup (e.g., in the constructor lambda)
pGraphics->AttachControl(new IVButtonControl(linkButtonBounds,
  [pGraphics](IControl* pCaller) {
    SplashClickActionFunc(pCaller);
    // Launch Link settings popover at button position
    IRECT bounds = pCaller->GetRECT();
    pGraphics->LaunchLinkSettingsDialog(bounds.MW(), bounds.MH());
  },
  "Link", DEFAULT_STYLE));
```

This presents the standard Ableton Link settings view controller showing:
- Link enable/disable toggle
- Connection status (number of connected peers)
- Start/Stop Sync toggle

## Accessing Link Data in Your Plugin

When Link is active, your plugin receives tempo and transport info through the standard `ITimeInfo` structure in `ProcessBlock()`:

```cpp
void MyPlugin::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  // Get current time info (populated from Link when enabled)
  ITimeInfo timeInfo;
  GetTimeInfo(timeInfo);

  // Use Link-synced tempo
  double tempo = timeInfo.mTempo;           // BPM from Link session
  double beatPos = timeInfo.mPPQPos;        // Current beat position
  bool isPlaying = timeInfo.mTransportIsRunning;  // Link play state

  // Your tempo-synced audio processing...
}
```

## Default Values

| Setting | Default | Description |
|---------|---------|-------------|
| Initial Tempo | 120 BPM | Set via `DEFAULT_TEMPO` constant |
| Quantum | 4.0 | Beats per bar (4/4 time signature) |
| Link Active | true | Link networking enabled on app launch |

## Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     iOS Standalone App                       │
├─────────────────────────────────────────────────────────────┤
│  IPlugAUPlayer                                               │
│  ├── ABLLinkRef (Link session)                              │
│  ├── Sets musicalContextBlock → tempo, beat position        │
│  └── Sets transportStateBlock → play/stop, sample position  │
├─────────────────────────────────────────────────────────────┤
│  IPlugAUAudioUnit (your plugin as AUv3)                     │
│  └── Queries context blocks in render → populates ITimeInfo │
├─────────────────────────────────────────────────────────────┤
│  Your Plugin ProcessBlock()                                  │
│  └── GetTimeInfo() returns Link-synced values               │
└─────────────────────────────────────────────────────────────┘
```

## Troubleshooting

### Link not discovering peers
- Ensure all devices are on the same WiFi network
- Check that multicast entitlement is configured (iOS 14+)
- Verify Link is enabled in the settings UI

### Tempo not updating
- Confirm `APP_ENABLE_LINK` is defined in config.h
- Ensure LinkKit.xcframework is linked to the App target (not just the AUv3 extension)

### Build errors about ABLLink.h
- Verify framework search path includes `$(IPLUG2_ROOT)/Dependencies/Build/xcframeworks`
- Ensure `#include "config.h"` appears before the Link imports

## Limitations

- iOS only (uses LinkKit xcframework)
- Desktop standalone support requires separate integration with the cross-platform Link library
- Time signature is currently hardcoded to 4/4
- Quantum (beats per bar) is fixed at 4.0

## Further Reading

- [Ableton Link Documentation](https://ableton.github.io/link/)
- [LinkKit Integration Guide](https://github.com/Ableton/LinkKit)
