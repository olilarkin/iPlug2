if (APPLE)
  enable_language(OBJCXX)
endif()

add_library(iplug_core STATIC ${SourcesCore})

set(SourcesCore
    IPlugAPIBase.h
    IPlugAPIBase.cpp
    IPlugConstants.h
    IPlugEditorDelegate.h
    IPlugLogger.h
    IPlugMidi.h
    IPlugParameter.h
    IPlugParameter.cpp
    IPlugPaths.h
    IPlugPaths.cpp
    IPlugPaths.mm
    IPlugPlatform.h
    IPlugPluginBase.h
    IPlugPluginBase.cpp
    IPlugProcessor.h
    IPlugProcessor.cpp
    IPlugQueue.h
    IPlugStructs.h
    IPlugTimer.h
    IPlugTimer.cpp
    IPlugUtilities.h

    # Extras/Easing.h
    # Extras/DCBlocker.h
    # Extras/Smoothers.h
    # Extras/NChanDelay.h
    # # Extras/OSC/IPlugOSC_msg.h
    # # Extras/OSC/IPlugOSC.cpp
    # # Extras/OSC/IPlugOSC.h
    # # Extras/OSC/IPlugOSC_msg.cpp
    # Extras/LanczosResampler.h
    # Extras/ADSREnvelope.h
    # Extras/Oscillator.h
    # Extras/NoiseGate.h
    # Extras/WebView/IPlugWKWebViewUIDelegate.mm
    # Extras/WebView/IPlugWebView.cpp
    # Extras/WebView/IPlugWebView_mac.mm
    # Extras/WebView/IPlugWKWebViewDelegate.mm
    # Extras/WebView/IPlugWKWebViewScriptMessageHandler.mm
    # Extras/WebView/IPlugWKWebViewScriptMessageHandler.h
    # Extras/WebView/IPlugWebViewEditorDelegate.mm
    # Extras/WebView/IPlugWKWebViewDelegate.h
    # Extras/WebView/IPlugWebView.h
    # Extras/WebView/IPlugWKWebView.h
    # Extras/WebView/IPlugWKWebViewUIDelegate.h
    # Extras/WebView/IPlugWebViewEditorDelegate.h
    # Extras/WebView/IPlugWebViewEditorDelegate.cpp
    # Extras/WebView/LICENSE.txt
    # Extras/WebView/IPlugWKWebView.mm
    # Extras/HIIR/PolyphaseIIR2Designer.h
    # Extras/HIIR/FPUUpsampler2x.h
    # Extras/HIIR/PolyphaseIIR2Designer.cpp
    # Extras/HIIR/FPUStageProc.h
    # Extras/HIIR/readme.txt
    # Extras/HIIR/FPUDownsampler2x.h
    # Extras/HIIR/license.txt
    # Extras/README.md
    # Extras/PlatformView/IPlugPlatformView.mm
    # Extras/PlatformView/IPlugPlatformView.h
    # Extras/RealtimeResampler.h
    # Extras/Synth/VoiceAllocator.h
    # Extras/Synth/SynthVoice.h
    # Extras/Synth/VoiceAllocator.cpp
    # Extras/Synth/MidiSynth.h
    # Extras/Synth/ControlRamp.h
    # Extras/Synth/MidiSynth.cpp
    # Extras/Cocoa/IPlugCocoaEditorDelegate.mm
    # Extras/Cocoa/IPlugCocoaEditorDelegate.h
    # Extras/Cocoa/IPlugCocoaViewController.h
    # Extras/Cocoa/IPlugCocoaViewController.mm
    # Extras/SVF.h
    # Extras/LFO.h
    # Extras/Oscillator_table.h
    # Extras/Oversampler.h
)

if (NOT WIN32)
    set_source_files_properties(
        IPlugPaths.cpp
        Extras/WebView/IPlugWebView.cpp
        Extras/WebView/IPlugWebView_win.cpp
        Extras/WebView/IPlugWebViewEditorDelegate.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()

if (NOT APPLE)
    set_source_files_properties(
        IPlugPaths.mm
        Extras/WebView/IPlugWebView_mac.mm
        Extras/WebView/IPlugWebViewEditorDelegate.mm
        Extras/PlatformView/IPlugPlatformView.mm
        Extras/Cocoa/IPlugCocoaEditorDelegate.mm
        Extras/Cocoa/IPlugCocoaViewController.mm
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()
if (NOT IOS)
  set_source_files_properties(
    Extras/WebView/IPlugWKWebViewDelegate.mm
    Extras/WebView/IPlugWKWebView.mm
    Extras/WebView/IPlugWKWebViewUIDelegate.mm
    Extras/WebView/IPlugWKWebViewScriptMessageHandler.mm
    PROPERTIES HEADER_FILE_ONLY TRUE
  )
endif()


target_sources(iplug_core PRIVATE ${SourcesCore})

target_include_directories(iplug_core PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/Extras"
)

# FIXME: dependencies
target_include_directories(iplug_core PUBLIC
    "../WDL"
    "../IGraphics"
    "../Dependencies/Extras/nlohmann"
)

if (APPLE)
    target_compile_options(iplug_core PRIVATE $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
    set_source_files_properties(
      Extras/Cocoa/IPlugCocoaEditorDelegate.mm
      Extras/Cocoa/IPlugCocoaViewController.mm
      Extras/WebView/IPlugWebViewEditorDelegate.mm
      PROPERTIES
      COMPILE_FLAGS  $<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-arc>
    )
  target_link_libraries(iplug_core PUBLIC "-framework AppKit" "-framework WebKit")
  # target_lin
endif()

###################################################################################################
# VST3

add_library(iplug_vst3 STATIC)
set(SourcesVst3
  "VST3/IPlugVST3.h"
  "VST3/IPlugVST3.cpp"
  "VST3/IPlugVST3_Common.h"
  "VST3/IPlugVST3_Controller.h"
  "VST3/IPlugVST3_Controller.cpp"
  "VST3/IPlugVST3_ControllerBase.h"
  "VST3/IPlugVST3_Defs.h"
  "VST3/IPlugVST3_Parameter.h"
  "VST3/IPlugVST3_Processor.h"
  "VST3/IPlugVST3_ProcessorBase.h"
  "VST3/IPlugVST3_ProcessorBase.cpp"
  "VST3/IPlugVST3_View.h"
)

target_sources(iplug_vst3 PRIVATE ${SourcesVst3})
target_compile_definitions(iplug_vst3 PUBLIC VST3_API)
target_link_libraries(iplug_vst3 PUBLIC iplug_core)

target_link_libraries(iplug_vst3
  PUBLIC pluginterfaces sdk
)

target_sources(iplug_vst3 PRIVATE
  ../Dependencies/IPlug/VST3_SDK/public.sdk/source/vst/vstsinglecomponenteffect.cpp
  ../Dependencies/IPlug/VST3_SDK/public.sdk/source/vst/vstsinglecomponenteffect.h
)

target_include_directories(iplug_vst3 INTERFACE VST3)

target_compile_definitions(iplug_vst3 PUBLIC "$<IF:$<CONFIG:Debug>,DEVELOPMENT,RELEASE>")

if (CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_sources(iplug_vst3 INTERFACE "IPlugVST3_RunLoop.cpp")
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX IPlug/VST3 FILES ${SourcesVst3})

###################################################################################################
# AUv2

add_library(iplug_auv2 STATIC)
set(SourcesAuv2
  "AUv2/IPlugAU.h"
  "AUv2/IPlugAU.cpp"
  "AUv2/IPlugAU_ioconfig.h"
  # "AUv2/IPlugAU_view_factory.mm"
  "AUv2/dfx-au-utilities.h"
  "AUv2/dfx-au-utilities.c"
)

target_sources(iplug_auv2 PRIVATE ${SourcesAuv2})
target_compile_definitions(iplug_auv2 PUBLIC AU_API)
target_link_libraries(iplug_auv2 PUBLIC iplug_core)

target_include_directories(iplug_auv2 INTERFACE AUv2)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX IPlug/AUv2 FILES ${SourcesAuv2})
