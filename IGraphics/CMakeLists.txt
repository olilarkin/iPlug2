if (APPLE)
    enable_language(OBJC OBJCXX)
endif()

set(Flavours
  igraphics
  igraphics_vst3
)

if (APPLE)
  list(APPEND Flavours igraphics_auv2 igraphics_auv3)
endif()

if (IPLUG_AAX)
  list(APPEND Flavours igraphics_AAX)
endif()

# TODO: can we provide a single target and forward obj-c class definions to downstream targets?
foreach (Target ${Flavours})
  add_library(${Target} STATIC)

  # TODO: iOS / web
  set(Sources
    Drawing/IGraphicsSkia.h
    Drawing/IGraphicsNanoVG.h
    Drawing/IGraphicsCanvas.cpp
    # Drawing/IGraphicsNanoVG_src.m
    Drawing/IGraphicsSkia.cpp
    Drawing/IGraphicsNanoVG.cpp
    Drawing/IGraphicsCanvas.h
    igraphics_include_in_plug_hdr.h
    # Platforms/IGraphicsIOS_view.h
    # Platforms/IGraphicsWeb.cpp
    # Platforms/IGraphicsWeb.h
    Platforms/IGraphicsWin.h
    Platforms/IGraphicsLinux.h
    Platforms/IGraphicsMac.h
    Platforms/IGraphicsMac.mm
    Platforms/IGraphicsMac_view.h
    Platforms/IGraphicsMac_view.mm
    Platforms/IGraphicsWin_dnd.h
    Platforms/IGraphicsLinux.cpp
    Platforms/IGraphicsCoreText.h
    # Platforms/IGraphicsIOS.mm
    Platforms/IGraphicsCoreText.mm
    Platforms/IGraphicsWin.cpp
    # Platforms/IGraphicsIOS.h
    # Platforms/IGraphicsIOS_view.mm
    IGraphicsPopupMenu.h
    IControl.h
    IGraphicsPrivate.h
    IGraphicsUtilities.h
    IGraphicsEditorDelegate.cpp
    IGraphics.cpp
    Controls/IVPresetManagerControls.h
    Controls/IVDisplayControl.h
    Controls/IControls.h
    # Controls/Test/Skia/ISkParagraphControl.h
    # Controls/Test/Skia/ISkLottieControl.h
    # Controls/Test/TestCursorControl.h
    # Controls/Test/TestSizeControl.h
    # Controls/Test/TestAnimationControl.h
    # Controls/Test/TestFontControl.h
    # Controls/Test/TestCustomShaderControl.h
    # Controls/Test/TestMPSControl.h
    # Controls/Test/TestTextControl.h
    # Controls/Test/TestDrawContextControl.h
    # Controls/Test/TestGesturesControl.h
    # Controls/Test/TestSVGControl.h
    # Controls/Test/TestShadowGradientControl.h
    # Controls/Test/TestBezierControl.h
    # Controls/Test/TestImageControl.h
    # Controls/Test/TestDirBrowseControl.h
    # Controls/Test/TestLayerControl.h
    # Controls/Test/TestFlexBoxControl.h
    # Controls/Test/TestControls.h
    # Controls/Test/TestTextOrientationControl.h
    # Controls/Test/TestTextSizeControl.h
    # Controls/Test/TestKeyboardControl.h
    # Controls/Test/TestColorControl.h
    # Controls/Test/TestBlendControl.h
    # Controls/Test/TestGradientControl.h
    # Controls/Test/TestDragNDropControl.h
    # Controls/Test/TestDropShadowControl.h
    # Controls/Test/TestGFXLabel.h
    # Controls/Test/TestCustomShaderControl.mm
    # Controls/Test/TestMultiPathControl.h
    # Controls/Test/TestMultiTouchControl.h
    # Controls/Test/TestMPSControl.mm
    # Controls/Test/TestArcControl.h
    # Controls/Test/TestMaskControl.h
    # Controls/Test/TestPolyControl.h
    Controls/IVMultiSliderControl.h
    Controls/IVTabbedPagesControl.h
    Controls/ITextEntryControl.h
    Controls/IVScopeControl.h
    Controls/IWebViewControl.h
    Controls/IVMeterControl.h
    Controls/IVNumberBoxControl.h
    Controls/ILEDControl.h
    Controls/IBubbleControl.h
    Controls/IRTTextControl.h
    Controls/IPlatformViewControl.h
    Controls/IColorPickerControl.h
    Controls/IAboutBoxControl.h
    Controls/ITextEntryControl.cpp
    Controls/IVKeyboardControl.h
    Controls/IFPSDisplayControl.h
    Controls/IPopupMenuControl.cpp
    Controls/IPopupMenuControl.h
    Controls/IControls.cpp
    Controls/IVSpectrumAnalyzerControl.h
    Controls/ICornerResizerControl.h
    Controls/IShaderControl.h
    IGraphicsStructs.h
    IGraphicsConstants.h
    IGraphicsLiveEdit.h
    igraphics_select.h
    igraphics_include_in_plug_src.h
    IGraphicsEditorDelegate.h
    IControl.cpp
    IGraphics.h

    Extras/IGraphicsFlexBox.cpp
    Extras/IGraphicsFlexBox.h
  )

    target_sources(${Target} PRIVATE ${Sources})

    target_include_directories(${Target} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

    # FIXME: can we have proper dependencies?
    target_include_directories(${Target}
        PRIVATE
        ../WDL
        ../IPlug
        ../IPlug/Extras
    )

    target_link_libraries(${Target} PUBLIC yoga stb)

    if (NOT WIN32)
        set_source_files_properties(
            Platforms/IGraphicsWin.cpp
            PROPERTIES HEADER_FILE_ONLY TRUE
        )
    endif()

    if (NOT LINUX)
        set_source_files_properties(
            Platforms/IGraphicsLinux.cpp
            PROPERTIES HEADER_FILE_ONLY TRUE
        )
    endif()

    if (APPLE)
        target_compile_options(${Target} PRIVATE
            $<$<COMPILE_LANGUAGE:OBJC>:-fobjc-arc>
            $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>)
        set_source_files_properties(
            Platforms/IGraphicsMac.mm
            Platforms/IGraphicsMac_view.mm
            PROPERTIES
            COMPILE_FLAGS  $<$<COMPILE_LANGUAGE:OBJCXX>:-fno-objc-arc>
        )
        target_link_libraries(${Target} PUBLIC
            "-framework Cocoa"
            "-framework QuartzCore"
        )
    set_source_files_properties(Drawing/IGraphicsNanoVG.cpp PROPERTIES LANGUAGE OBJCXX)
    else()
        set_source_files_properties(
            Platforms/IGraphicsMac.mm
            Platforms/IGraphicsMac_view.mm
            Platforms/IGraphicsCoreText.mm
            PROPERTIES HEADER_FILE_ONLY TRUE
        )
    endif()

    target_include_directories(${Target}
        PUBLIC Controls Drawing Platforms
    )

    set(IPLUG_NANOVG TRUE)

    # if (IPLUG_NANOVG)
    if(TRUE)
        # nanovg
        target_compile_definitions(${Target} PUBLIC IGRAPHICS_NANOVG)
        target_link_libraries(${Target} PUBLIC nanosvg)

        set_source_files_properties(
            Drawing/IGraphicsCanvas.cpp
            Drawing/IGraphicsSkia.cpp
            PROPERTIES HEADER_FILE_ONLY TRUE
        )
        if (APPLE)
            target_compile_definitions(${Target} PUBLIC IGRAPHICS_METAL)
            target_link_libraries(${Target} PUBLIC metal_nanovg)
        else()
            target_link_libraries(${Target} PUBLIC nanovg)
        endif()
    endif()
endforeach()

if (TARGET igraphics_vst3)
  target_link_libraries(igraphics_vst3 PUBLIC iplug_vst3)
  target_compile_definitions(igraphics_vst3 PUBLIC VST3_API)
endif()

if (TARGET igraphics_auv2)
  target_link_libraries(igraphics_auv2 PUBLIC iplug_auv2)
  target_compile_definitions(igraphics_auv2 PUBLIC AU_API)
endif()
if (TARGET igraphics_auv3)
  target_compile_definitions(igraphics_auv3 PUBLIC AUv3_API)
endif()
if (TARGET igraphics_AAX)
  # FIXME: embarassingly untested
  target_link_libraries(igraphics_vst3 PUBLIC AAX::Sdk)
  target_compile_definitions(igraphics_AAX PUBLIC AAX_API)
endif()
