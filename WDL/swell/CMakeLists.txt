add_library(Swell STATIC)

set(Sources
    gtkimcontextsimpleseqs.h
    swell-appstub-generic.cpp
    swell-appstub.mm
    swell-dlg-generic.cpp
    swell-dlg.mm
    swell-dlggen.h
    swell-functions.h
    swell-gdi-generic.cpp
    swell-gdi-internalpool.h
    swell-gdi-lice.cpp
    swell-gdi.mm
    swell-generic-gdk.cpp
    swell-generic-headless.cpp
    swell-ini.cpp
    swell-internal.h
    swell-kb-generic.cpp
    swell-kb.mm
    swell-menu-generic.cpp
    swell-menu.mm
    swell-menugen.h
    swell-misc-generic.cpp
    swell-misc.mm
    swell-miscdlg-generic.cpp
    swell-miscdlg.mm
    swell-modstub-generic.cpp
    swell-modstub.mm
    swell-types.h
    swell-win32.h
    swell-wnd-generic.cpp
    swell-wnd.mm
    swell.cpp
    swell.h
    swellappmain.h
    swellappmain.mm
)

if (NOT APPLE)
    set_source_files_properties(
        swell-appstub.mm
        swell-dlg.mm
        swell-gdi.mm
        swell-kb.mm
        swell-menu.mm
        swell-misc.mm
        swell-miscdlg.mm
        swell-modstub.mm
        swell-wnd.mm
        swellappmain.mm
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()

if (NOT WIN32)
    set_source_files_properties(
        swell-dlg-generic.cpp
        swell-gdi-generic.cpp
        swell-kb-generic.cpp
        swell-menu-generic.cpp
        swell-misc-generic.cpp
        swell-miscdlg-generic.cpp
        swell-modstub-generic.cpp
        swell-wnd-generic.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()

if (NOT LINUX)
    set_source_files_properties(
        swell-gdi-lice.cpp
        swell-generic-gdk.cpp
        swell-generic-headless.cpp
        PROPERTIES HEADER_FILE_ONLY TRUE
    )
endif()

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${Sources})
target_sources(Swell PRIVATE ${Sources})

target_include_directories(Swell PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if (APPLE)
    enable_language(OBJCXX)

    target_compile_definitions(Swell PUBLIC SWELL_COMPILED SWELL_NO_METAL)
    target_link_libraries(Swell PUBLIC
        "-framework AudioUnit"
        "-framework Carbon"
        "-framework CoreFoundation"
        "-framework Cocoa"
        "-framework QuartzCore"
        "-framework Metal")

    target_link_options(Swell PUBLIC -ObjC)
endif()

target_compile_definitions(Swell PUBLIC SWELL_COMPILED WDL_NO_DEFINE_MINMAX)

