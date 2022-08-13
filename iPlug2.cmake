#  ==============================================================================
#  
#  This file is part of the iPlug 2 library. Copyright (C) the iPlug 2 developers. 
#
#  See LICENSE.txt for  more info.
#
#  ==============================================================================

# This file should be included in your main CMakeLists.txt file.

include_guard(DIRECTORY)

if (APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)

  add_compile_options(-Wno-elaborated-enum-base)    # help clangd
  add_compile_options(-Wno-deprecated-declarations) # WAYYYY to many warnings
endif()

# Make sure MSVC uses static linking for compatibility with Skia libraries and easier distribution.
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# We generate folders for targets that support it (Visual Studio, Xcode, etc.)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] TRUE)
set(CMAKE_XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym")
set(CMAKE_XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")

function(iplug_add_app PlugName)
  cmake_policy(SET CMP0076 NEW)

  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "SOURCES;RESOURCES")

  set(TargetName ${PlugName}App)

  add_executable(${TargetName} WIN32 MACOSX_BUNDLE ${ARG_SOURCES})
  target_link_libraries(${TargetName} PUBLIC igraphics iplug_core)
  target_link_libraries(${TargetName} PUBLIC rtmidi rtaudio iplug_core igraphics)

  set(SdkRoot ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(ResourceDir ${CMAKE_CURRENT_SOURCE_DIR}/resources)

  target_sources(${TargetName} PRIVATE
    ${SdkRoot}/IPlug/APP/IPlugAPP.cpp
    ${SdkRoot}/IPlug/APP/IPlugAPP_dialog.cpp
    ${SdkRoot}/IPlug/APP/IPlugAPP_host.cpp
    ${SdkRoot}/IPlug/APP/IPlugAPP_main.cpp
    ${SdkRoot}/IPlug/APP/IPlugAPP_main.mm
    ${SdkRoot}/IPlug/APP/IPlugAPP.h
  )

  target_compile_definitions(${TargetName} PUBLIC APP_API)
  target_include_directories(${TargetName} PUBLIC
    ${SdkRoot}/IPlug/APP
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
  )

  target_compile_definitions(${TargetName} PUBLIC IPLUG_DSP=1 IPLUG_EDITOR=1)

  if (APPLE)
    enable_language(OBJCXX)
    # Set language for files combining C++ and Objective-C
    set_source_files_properties(${sdk}/IPlugAPP_main.cpp DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTIES LANGUAGE "OBJCXX")
    target_link_libraries(${TargetName} PUBLIC Swell "-framework CoreMIDI")

    target_link_options(${TargetName} BEFORE PUBLIC -ObjC)

    set(Resources
      "${ResourceDir}/${PlugName}.icns"
      "${CMAKE_CURRENT_BINARY_DIR}/${PlugName}-macOS-MainMenu.nib"
      ${ARG_RESOURCES}
    )

    find_program( IBTOOL ibtool HINTS "/usr/bin" "${OSX_DEVELOPER_ROOT}/usr/bin" )
    add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PlugName}-macOS-MainMenu.nib
      COMMAND ${IBTOOL} --errors --warnings --notices --output-format human-readable-text
             --compile
             ${CMAKE_CURRENT_BINARY_DIR}/${PlugName}-macOS-MainMenu.nib
             ${ResourceDir}/${PlugName}-macOS-MainMenu.xib
    )

    set_source_files_properties(${Resources}
      PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(${TargetName} PUBLIC ${Resources})
    source_group(Resources FILES ${Resources} ${ResourceDir}/${PlugName}-macOS-MainMenu.xib)

    set_target_properties(${TargetName} PROPERTIES
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST
        ${ResourceDir}/${PlugName}-macOS-Info.plist
      OUTPUT_NAME_DEBUG ${PlugName}
      OUTPUT_NAME_RELEASE ${PlugName}
    )
  endif()
endfunction()

function(iplug_add_vst3 PlugName)
  cmake_policy(SET CMP0076 NEW)

  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "SOURCES;RESOURCES")

  set(TargetName ${PlugName}VST3)

  add_library(${TargetName} MODULE ${ARG_SOURCES})
  target_link_libraries(${TargetName} PUBLIC igraphics_vst3 iplug_core)

  set(SdkRoot ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(ResourceDir ${CMAKE_CURRENT_SOURCE_DIR}/resources)

  target_compile_definitions(${TargetName} PUBLIC VST3_API)
  target_include_directories(${TargetName} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
  )

  target_compile_definitions(${TargetName} PUBLIC IPLUG_DSP=1 IPLUG_EDITOR=1)

  if (APPLE)
    set(Resources
      "${ResourceDir}/${PlugName}.icns"
      ${ARG_RESOURCES}
    )

    set_source_files_properties(${Resources}
      PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
    )

    target_sources(${TargetName} PUBLIC ${Resources})
    source_group(Resources FILES ${Resources})

    set_target_properties(${TargetName} PROPERTIES
      BUNDLE TRUE
      MACOSX_BUNDLE TRUE
      MACOSX_BUNDLE_INFO_PLIST
        ${ResourceDir}/${PlugName}-VST3-Info.plist
      BUNDLE_EXTENSION "vst3"
      PREFIX ""
      SUFFIX ""
      OUTPUT_NAME_DEBUG ${PlugName}
      OUTPUT_NAME_RELEASE ${PlugName}    
    )

    # Create PkgInfo file to make it appear as a bundle in Finder
    set(PKGINFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PlugName}.vst3/Contents/PkgInfo")
    file(WRITE ${PKGINFO_FILE} "BNDL????")
    add_custom_command(TARGET ${TargetName} POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E touch ${PKGINFO_FILE})
  endif()
endfunction()

function(iplug_add_auv2 PlugName)
  cmake_policy(SET CMP0076 NEW)

  cmake_parse_arguments(PARSE_ARGV 0 ARG "" "" "SOURCES;RESOURCES")

  set(TargetName ${PlugName}AUv2)

  add_library(${TargetName} MODULE ${ARG_SOURCES})
  target_link_libraries(${TargetName} PUBLIC igraphics_auv2 iplug_core)

  set(SdkRoot ${CMAKE_CURRENT_FUNCTION_LIST_DIR})
  set(ResourceDir ${CMAKE_CURRENT_SOURCE_DIR}/resources)

  target_compile_definitions(${TargetName} PUBLIC AU_API)
  target_include_directories(${TargetName} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/resources
  )

  target_compile_definitions(${TargetName} PUBLIC IPLUG_DSP=1 IPLUG_EDITOR=1)

  set(Resources
    "${ResourceDir}/${PlugName}.icns"
    ${ARG_RESOURCES}
  )

  set_source_files_properties(${Resources}
    PROPERTIES MACOSX_PACKAGE_LOCATION "Resources"
  )

  enable_language(OBJCXX)
  # Set language for files combining C++ and Objective-C
  target_link_libraries(${TargetName} PUBLIC "-framework AudioUnit -framework AudioToolbox -framework CoreAudio -framework CoreMIDI")

  target_sources(${TargetName} PUBLIC ${Resources})
  source_group(Resources FILES ${Resources})

  set_target_properties(${TargetName} PROPERTIES
    BUNDLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST
      ${ResourceDir}/${PlugName}-AU-Info.plist
    BUNDLE_EXTENSION "component"
    PREFIX ""
    SUFFIX ""
    OUTPUT_NAME_DEBUG ${PlugName}
    OUTPUT_NAME_RELEASE ${PlugName}  
  )

  # Create PkgInfo file to make it appear as a bundle in Finder
  set(PKGINFO_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PlugName}.component/Contents/PkgInfo")
  file(WRITE ${PKGINFO_FILE} "BNDL????")
  add_custom_command(TARGET ${TargetName} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E touch ${PKGINFO_FILE})
endfunction()
