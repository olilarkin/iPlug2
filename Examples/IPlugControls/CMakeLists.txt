cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
cmake_policy(SET CMP0091 NEW)

project(IPlugControls VERSION 1.0.0 LANGUAGES C CXX)


set(SRC_FILES
  config.h
  IPlugControls.cpp
  IPlugControls.h
  IconsFontaudio.h
)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${SRC_FILES})

set(Resources
  resources/fonts/fontaudio.ttf
  resources/fonts/forkawesome-webfont.ttf
  resources/fonts/Roboto-Regular.ttf

  resources/img/button.png
  resources/img/button@2x.png
  resources/img/font.png
  resources/img/font@2x.png
  resources/img/hslider-handle.svg
  resources/img/hslider-track.svg
  resources/img/knob-rotate.png
  resources/img/knob-rotate@2x.png
  resources/img/knob.png
  resources/img/knob@2x.png
  resources/img/slider-handle.png
  resources/img/slider-handle.svg
  resources/img/slider-handle@2x.png
  resources/img/slider-track.png
  resources/img/slider-track.svg
  resources/img/slider-track@2x.png
  resources/img/switch.png
  resources/img/switch@2x.png
  resources/img/vector-knob.svg
)

iplug_add_app(IPlugControls
    SOURCES ${SRC_FILES}
    RESOURCES ${Resources}
)

iplug_add_vst3(IPlugControls
    SOURCES ${SRC_FILES}
    RESOURCES ${Resources}
)

set(AUv2_SRC_FILES
  "../../IPlug/AUv2/IPlugAU_view_factory.mm"
)

iplug_add_auv2(IPlugControls
    SOURCES ${SRC_FILES} ${AUv2_SRC_FILES}
    RESOURCES ${Resources}
)

if (APPLE)
    foreach (Target IPlugControlsApp IPlugControlsVST3 IPlugControlsAUv2)
        target_link_libraries(${Target} PUBLIC "-framework Accelerate")
    endforeach()
endif()
