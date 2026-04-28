cmake_minimum_required(VERSION 3.27)

project(TranscendenceMacOS LANGUAGES C CXX)

if(NOT APPLE)
    message(FATAL_ERROR "This scaffold is currently intended for macOS only.")
endif()

find_package(SDL2 REQUIRED)

include_directories(${SDL2_INCLUDE_DIRS})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_compile_options(-Wno-non-pod-varargs)
add_compile_definitions(TARGET_PLATFORM_MACOS)

option(TRANSCENDENCE_M1_MENU_ONLY "Limit the build graph to the milestone-1 menu path." ON)
option(TRANSCENDENCE_ENABLE_PLATFORM_MACOS "Enable macOS-specific platform target placeholder." OFF)
option(TRANSCENDENCE_ENABLE_AUDIO_BACKEND "Enable audio backend placeholder target." OFF)

set(ALCHEMY_KERNEL_SOURCES
    "Alchemy/Kernel/CalcConvexHull.cpp"
    "Alchemy/Kernel/CArrayBase.cpp"
    "Alchemy/Kernel/CAtomizer.cpp"
    "Alchemy/Kernel/CAtomTable.cpp"
    "Alchemy/Kernel/CConsoleFormat.cpp"
    "Alchemy/Kernel/CDictionary.cpp"
    "Alchemy/Kernel/CException.cpp"
    "Alchemy/Kernel/CFileDirectory.cpp"
    "Alchemy/Kernel/CGeometry.cpp"
    "Alchemy/Kernel/CINTDynamicArray.cpp"
    "Alchemy/Kernel/CIntArray.cpp"
    "Alchemy/Kernel/CIntGraph.cpp"
    "Alchemy/Kernel/CLabelArranger.cpp"
    "Alchemy/Kernel/CMapBase.cpp"
    "Alchemy/Kernel/CPolygon.cpp"
    "Alchemy/Kernel/CProjection3D.cpp"
    "Alchemy/Kernel/CSphericalCoordinates.cpp"
    "Alchemy/Kernel/CStepIncrementor.cpp"
    "Alchemy/Kernel/CVector.cpp"
    "Alchemy/Kernel/CWordParser.cpp"
    "Alchemy/Kernel/CXForm3D.cpp"
    "Alchemy/Kernel/Euclid.cpp"
    "Alchemy/Kernel/RunLengthCompression.cpp"
    "Alchemy/Kernel/SecureHashAlgorithm.cpp"
    "Alchemy/Kernel/Shapes.cpp"
    "Alchemy/Kernel/XForm.cpp"
)

set(ALCHEMY_CODECHAIN_SOURCES
    "Alchemy/CodeChain/CCDouble.cpp"
    "Alchemy/CodeChain/CCInteger.cpp"
    "Alchemy/CodeChain/CCItemPool.cpp"
    "Alchemy/CodeChain/CCNil.cpp"
    "Alchemy/CodeChain/CCNumeral.cpp"
    "Alchemy/CodeChain/CCTrue.cpp"
    "Alchemy/CodeChain/ICCItem.cpp"
    "Alchemy/CodeChain/ICCItemPtr.cpp"
    "Alchemy/CodeChain/PreComp.cpp"
)

set(ALCHEMY_XMLUTIL_SOURCES
    "Alchemy/XMLUtil/CEntityResolverList.cpp"
    "Alchemy/XMLUtil/CExternalEntityTable.cpp"
    "Alchemy/XMLUtil/Misc.cpp"
)

set(ALCHEMY_JPEG_SOURCES
    "Alchemy/IntelJPEGUtil/Load.cpp"
    "Alchemy/IntelJPEGUtil/Save.cpp"
)

set(ALCHEMY_GRAPHICS_SOURCES
    "Alchemy/Graphics/Misc.cpp"
    "Alchemy/DirectXUtil/CScreenMgrSDL.cpp"
)

set(MAMMOTH_TSE_SOURCES
    "Mammoth/TSE/CDesignCollection.cpp"
    "Mammoth/TSE/CSystem.cpp"
    "Mammoth/TSE/CUniverse.cpp"
    "Mammoth/TSE/CreateSystem.cpp"
)

set(MAMMOTH_TSUI_SOURCES
    "Mammoth/TSUI/CVisualPalette.cpp"
    "Mammoth/TSUI/IHISession.cpp"
    "Mammoth/TSUI/IHIController.cpp"
    "Mammoth/TSUI/TSUICodeChain.cpp"
    "Mammoth/TSUI/CBackgroundProcessor.cpp"
    "Mammoth/TSUI/HardCrash.cpp"
    "Mammoth/TSUI/CHexarcServiceStub.cpp"
    "Mammoth/TSUI/CCloudService.cpp"
    "Mammoth/TSUI/CMapLegendPainter.cpp"
    "Mammoth/TSUI/CMapScaleCounter.cpp"
    "Mammoth/TSUI/CSoundtrackManager.cpp"
    "Mammoth/TSUI/CTilePainter.cpp"
    "Mammoth/TSUI/CTimersHUD.cpp"
    "Mammoth/TSUI/IHUDPainter.cpp"
    "Mammoth/TSUI/CWeaponHUDDefault.cpp"
    "Mammoth/TSUI/CReactorHUDDefault.cpp"
    "Mammoth/TSUI/CUIHelper.cpp"
    "Mammoth/TSUI/CArmorHUDImages.cpp"
    "Mammoth/TSUI/CArmorHUDRingSegments.cpp"
    "Mammoth/TSUI/CArmorHUDShaped.cpp"
    "Mammoth/TSUI/CArmorHUDRectangular.cpp"
    "Mammoth/TSUI/Dialogs.cpp"
    "Mammoth/TSUI/ClassInfoHelpers.cpp"
    "Mammoth/TSUI/CItemPainter.cpp"
    "Mammoth/TSUI/CHoverDescriptionPainter.cpp"
    "Mammoth/TSUI/CStargateEffectPainter.cpp"
    "Mammoth/TSUI/CTextAreaAnimator.cpp"
    "Mammoth/TSUI/CSmallOptionButtonAnimator.cpp"
    "Mammoth/TSUI/CAccelerateHUD.cpp"
    "Mammoth/TSUI/CAutomataEffectPainter.cpp"
    "Mammoth/TSUI/CLRSHUD.cpp"
    "Mammoth/TSUI/CItemDataAnimatron.cpp"
    "Mammoth/TSUI/CGalacticMapSystemDetails.cpp"
    "Mammoth/TSUI/CReadProfileTask.cpp"
    "Mammoth/TSUI/CListCollectionTask.cpp"
    "Mammoth/TSUI/CListSaveFilesTask.cpp"
    "Mammoth/TSUI/CHumanInterface.cpp"
    "Transcendence/Transcendence/CIntroSession.cpp"
)

add_library(alchemy_kernel STATIC ${ALCHEMY_KERNEL_SOURCES})
target_include_directories(alchemy_kernel PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
)

add_library(alchemy_codechain STATIC ${ALCHEMY_CODECHAIN_SOURCES})
target_include_directories(alchemy_codechain PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
)
target_link_libraries(alchemy_codechain PUBLIC alchemy_kernel)

add_library(alchemy_xmlutil STATIC ${ALCHEMY_XMLUTIL_SOURCES})
target_include_directories(alchemy_xmlutil PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
)
target_link_libraries(alchemy_xmlutil PUBLIC alchemy_kernel)

add_library(alchemy_zlib INTERFACE)

add_library(alchemy_jpeg STATIC ${ALCHEMY_JPEG_SOURCES})
target_include_directories(alchemy_jpeg PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
)
target_link_libraries(alchemy_jpeg PUBLIC
    alchemy_kernel
)

add_library(alchemy_graphics STATIC ${ALCHEMY_GRAPHICS_SOURCES})
target_include_directories(alchemy_graphics PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
)
target_link_libraries(alchemy_graphics PUBLIC
    alchemy_kernel
)

# mammoth_tse - Transcendence Space Engine
# Heavy DirectX dependencies require significant porting work
# Building with minimal source subset for initial porting
add_library(mammoth_tse STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/TSE/CDesignCollection.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/TSE/CSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/TSE/CUniverse.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/TSE/CreateSystem.cpp"
)
target_include_directories(mammoth_tse PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/Include"
)
target_link_libraries(mammoth_tse PUBLIC
    alchemy_kernel
    alchemy_codechain
    alchemy_xmlutil
    alchemy_graphics
    alchemy_zlib
    alchemy_jpeg
)

# mammoth_tsui - Transcendence UI Engine
add_library(mammoth_tsui STATIC ${MAMMOTH_TSUI_SOURCES})
target_include_directories(mammoth_tsui PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/Include"
)
target_link_libraries(mammoth_tsui PUBLIC
    mammoth_tse
    alchemy_kernel
    alchemy_graphics
    platform_sdl
)

# mammoth_tsui_core requires mammoth_tse
add_library(mammoth_tsui_core INTERFACE)
target_link_libraries(mammoth_tsui_core INTERFACE
    mammoth_tse
    alchemy_kernel
    alchemy_graphics
)

add_library(platform_common INTERFACE)

add_library(platform_sdl STATIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence/Platform/AppCore.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence/Platform/CPlatformTimerSystem.cpp"
)
target_include_directories(platform_sdl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence/Platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence"
)
target_link_libraries(platform_sdl PUBLIC
    SDL2::SDL2main
    SDL2::SDL2
    "-framework CoreFoundation"
    platform_common
)

add_library(render_common INTERFACE)

add_library(render_metal INTERFACE)
target_link_libraries(render_metal INTERFACE
    render_common
    platform_common
    platform_sdl
)

if(TRANSCENDENCE_ENABLE_PLATFORM_MACOS)
    add_library(platform_macos INTERFACE)
    target_link_libraries(platform_macos INTERFACE platform_common)
endif()

if(TRANSCENDENCE_ENABLE_AUDIO_BACKEND)
    add_library(audio_backend INTERFACE)
    target_link_libraries(audio_backend INTERFACE platform_common)
endif()

add_executable(transcendence_app
    "Transcendence/Transcendence/main.cpp"
)

target_include_directories(transcendence_app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Alchemy/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Mammoth/Include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence/Platform"
    "${CMAKE_CURRENT_SOURCE_DIR}/Transcendence/Transcendence"
)

target_link_libraries(transcendence_app PRIVATE
    alchemy_kernel
    alchemy_codechain
    alchemy_xmlutil
    alchemy_graphics
    alchemy_jpeg
    mammoth_tse
    mammoth_tsui
    platform_common
    platform_sdl
    render_common
    render_metal
)

set_target_properties(transcendence_app PROPERTIES
    OUTPUT_NAME "Transcendence"
)

message(STATUS "Configured bounded milestone-1 macOS scaffold.")
message(STATUS "Targets are placeholders for the audited menu-path graph and are not yet full source manifests.")
