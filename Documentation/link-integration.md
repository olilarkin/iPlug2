# Ableton Link Integration for iOS Apps

iPlug2 supports optional [Ableton Link](https://www.ableton.com/link/) integration for iOS standalone apps. When enabled, the standalone AUv3 host publishes Link tempo, beat position, and transport state to the plugin through the standard `musicalContextBlock` and `transportStateBlock` callbacks.

The integration uses the open-source Ableton Link C++ library from `Dependencies/IPlug/Link`. It does not require `LinkKit.xcframework`.

## What Link Provides

- Tempo synchronization with other Link-enabled apps
- Beat and phase alignment on the shared Link timeline
- Optional start/stop synchronization
- A host-side settings sheet launched from plugin UI code

## Requirements

- iOS standalone app target
- `Dependencies/IPlug/Link` submodule initialized
- iOS 14+ local network privacy description
- Multicast networking entitlement for device peer discovery on iOS 14+

Ableton Link is GPL-licensed unless you have a separate commercial license from Ableton. Keep `APP_ENABLE_LINK` optional for projects that cannot accept that dependency.

## Setup

Initialize the Link submodule:

```sh
git submodule update --init Dependencies/IPlug/Link
```

Enable Link in your project's `config.h`:

```cpp
#define APP_ENABLE_LINK 1
```

Add the reusable Link settings in your iOS `.xcconfig`:

```xcconfig
EXTRA_INC_PATHS = $(IGRAPHICS_INC_PATHS) $(ABLETON_LINK_INC_PATHS)
EXTRA_ALL_DEFS = OBJC_PREFIX=vMyPlug IGRAPHICS_NANOVG IGRAPHICS_METAL SAMPLE_TYPE_FLOAT $(ABLETON_LINK_DEFS)
```

For peer discovery on iOS 14+, add a local network usage string to the standalone app `Info.plist`:

```xml
<key>NSLocalNetworkUsageDescription</key>
<string>Ableton Link uses the local network to discover and synchronize with nearby music apps.</string>
```

For device builds, add the multicast entitlement to the app entitlements file after Apple enables it for your team:

```xml
<key>com.apple.developer.networking.multicast</key>
<true/>
```

## Settings Button

Plugins can open the host-side Link sheet from iOS graphics code:

```cpp
#if defined(OS_IOS) && defined(APP_ENABLE_LINK) && APP_ENABLE_LINK
#include "IGraphicsIOS.h"
#endif

// In your UI setup:
auto* linkBtn = new IVButtonControl(linkButtonBounds, SplashClickActionFunc, "Link", style);
linkBtn->SetAnimationEndActionFunction([pGraphics](IControl* pCaller) {
  if (auto* pIOSGraphics = dynamic_cast<IGraphicsIOS*>(pGraphics))
  {
    IRECT r = pCaller->GetRECT();
    pIOSGraphics->LaunchLinkSettingsDialog(r.MW(), r.T);
  }
});
pGraphics->AttachControl(linkBtn);
```

The settings sheet includes Link enable/disable, peer count, local tempo control, start/stop sync, and local transport start/stop controls. Link enable, start/stop sync, tempo, and quantum are persisted in `NSUserDefaults`.

## Plugin Timing Data

When Link is enabled, plugins read synchronized timing through the normal `ITimeInfo` values in `ProcessBlock()`:

```cpp
void MyPlugin::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double tempo = GetTempo();
  const double beatPosition = GetPPQPos();
  const double samplePosition = GetSamplePos();
  const bool transportRunning = GetTransportIsRunning();

  // Tempo-synced processing...
}
```

`IPlugAUPlayer` maps the Link timeline to:

- `mTempo`: Link session tempo
- `mPPQPos`: Link beat position at the estimated output time
- `mSamplePos`: beat-derived sample position using the current Link tempo
- `mTransportIsRunning`: Link start/stop state

## Chunks Demo

`Examples/IPlugChunks` enables Link for the iOS standalone app. The iOS UI has a `Link` button in the top bar that launches the settings sheet. The step display follows the Link tempo and beat phase.

## Troubleshooting

If peers are not discovered:

- Confirm all apps are on the same local network
- Confirm Link is enabled in the settings sheet
- Confirm `NSLocalNetworkUsageDescription` is present
- Confirm the app build is signed with `com.apple.developer.networking.multicast` for device testing

If the project does not compile:

- Confirm `Dependencies/IPlug/Link` is initialized
- Confirm `$(ABLETON_LINK_INC_PATHS)` is included in `EXTRA_INC_PATHS`
- Confirm `$(ABLETON_LINK_DEFS)` is included in `EXTRA_ALL_DEFS`

## Current Scope

- iOS standalone AUv3 host support
- Fixed 4/4 time signature and default quantum of 4 beats
- No desktop standalone Link host yet
