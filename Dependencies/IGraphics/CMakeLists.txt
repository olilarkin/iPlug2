###################################################################################################
# Yoga

add_library(yoga STATIC
    yoga/yoga/YGFloatOptional.h
    yoga/yoga/Utils.h
    yoga/yoga/CompactValue.h
    yoga/yoga/Yoga-internal.h
    yoga/yoga/YGLayout.h
    yoga/yoga/log.cpp
    yoga/yoga/Utils.cpp
    yoga/yoga/YGNodePrint.h
    yoga/yoga/YGEnums.h
    yoga/yoga/YGLayout.cpp
    yoga/yoga/YGNode.h
    yoga/yoga/YGMacros.h
    yoga/yoga/Yoga.cpp
    yoga/yoga/YGStyle.cpp
    yoga/yoga/Yoga.h
    yoga/yoga/YGNodePrint.cpp
    yoga/yoga/YGStyle.h
    yoga/yoga/YGEnums.cpp
    yoga/yoga/log.h
    yoga/yoga/YGValue.cpp
    yoga/yoga/YGConfig.h
    yoga/yoga/YGConfig.cpp
    yoga/yoga/YGNode.cpp
    yoga/yoga/YGValue.h
    yoga/yoga/event/event.h
    yoga/yoga/event/event.cpp
    yoga/yoga/Bitfield.h
)

target_include_directories(yoga
    SYSTEM PUBLIC yoga/yoga
                  yoga
)


###################################################################################################
# NanoVG

add_library(nanovg STATIC
    NanoVG/src/nanovg_gl.h
    NanoVG/src/nanovg.h
    NanoVG/src/stb_image.h
    NanoVG/src/stb_truetype.h
    NanoVG/src/fontstash.h
    NanoVG/src/nanovg_gl_utils.h
    NanoVG/src/nanovg.c
)

target_include_directories(nanovg
    SYSTEM PUBLIC NanoVG/src
)

###################################################################################################
# MetalNanoVG

if(APPLE)
    enable_language(OBJC)
    add_library(metal_nanovg STATIC
        MetalNanoVG/src/nanovg_mtl_shaders.metal
        MetalNanoVG/src/nanovg_mtl.m
        MetalNanoVG/src/mnvg_bitcode/ios.h
        MetalNanoVG/src/mnvg_bitcode/tvos.h
        MetalNanoVG/src/mnvg_bitcode/macos.h
        MetalNanoVG/src/mnvg_bitcode/simulator.h
        MetalNanoVG/src/nanovg_mtl.h
    )

    target_compile_options(metal_nanovg PRIVATE -fobjc-arc)

    target_include_directories(metal_nanovg
       SYSTEM PUBLIC MetalNanoVG/src
    )

    target_link_libraries(metal_nanovg PUBLIC nanovg "-framework Metal")
endif()

###################################################################################################
# NanoSVG

add_library(nanosvg INTERFACE
    NanoSVG/src/nanosvg.h
    NanoSVG/src/nanosvgrast.h
)

target_include_directories(nanosvg
    SYSTEM INTERFACE NanoSVG/src
)

###################################################################################################
# STB

add_library(stb INTERFACE
    STB/stb_textedit.h
)

target_include_directories(stb
    SYSTEM INTERFACE STB
)
