cmake_minimum_required(VERSION 3.27)

project(TranscendenceMacOS LANGUAGES C CXX)

if(NOT APPLE)
    message(FATAL_ERROR "This scaffold is currently intended for macOS only.")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(TRANSCENDENCE_M1_MENU_ONLY "Limit the build graph to the milestone-1 menu path." ON)
option(TRANSCENDENCE_ENABLE_PLATFORM_MACOS "Enable macOS-specific platform target placeholder." OFF)
option(TRANSCENDENCE_ENABLE_AUDIO_BACKEND "Enable audio backend placeholder target." OFF)

set(ALCHEMY_KERNEL_SOURCES
    "Alchemy/Kernel/Base64.cpp"
    "Alchemy/Kernel/CalcConvexHull.cpp"
    "Alchemy/Kernel/CArchiver.cpp"
    "Alchemy/Kernel/CArrayBase.cpp"
    "Alchemy/Kernel/CAtomizer.cpp"
    "Alchemy/Kernel/CAtomTable.cpp"
    "Alchemy/Kernel/CConsoleFormat.cpp"
    "Alchemy/Kernel/CDataFile.cpp"
    "Alchemy/Kernel/CDictionary.cpp"
    "Alchemy/Kernel/CException.cpp"
    "Alchemy/Kernel/CFileDirectory.cpp"
    "Alchemy/Kernel/CFileReadBlock.cpp"
    "Alchemy/Kernel/CFileReadStream.cpp"
    "Alchemy/Kernel/CFileWriteStream.cpp"
    "Alchemy/Kernel/CGeometry.cpp"
    "Alchemy/Kernel/CHTML.cpp"
    "Alchemy/Kernel/CHTMLForm.cpp"
    "Alchemy/Kernel/CHTTPClientSession.cpp"
    "Alchemy/Kernel/CHTTPMessage.cpp"
    "Alchemy/Kernel/CIDTable.cpp"
    "Alchemy/Kernel/CINTDynamicArray.cpp"
    "Alchemy/Kernel/CIntArray.cpp"
    "Alchemy/Kernel/CIntegerIP.cpp"
    "Alchemy/Kernel/CIntGraph.cpp"
    "Alchemy/Kernel/CLabelArranger.cpp"
    "Alchemy/Kernel/CLargeSet.cpp"
    "Alchemy/Kernel/CManualEvent.cpp"
    "Alchemy/Kernel/CMapBase.cpp"
    "Alchemy/Kernel/CMarkovWordGenerator.cpp"
    "Alchemy/Kernel/CMemoryStream.cpp"
    "Alchemy/Kernel/CObject.cpp"
    "Alchemy/Kernel/CPeriodicWaiter.cpp"
    "Alchemy/Kernel/CPolygon.cpp"
    "Alchemy/Kernel/CProjection3D.cpp"
    "Alchemy/Kernel/CRegKey.cpp"
    "Alchemy/Kernel/CResourceReadBlock.cpp"
    "Alchemy/Kernel/CSphericalCoordinates.cpp"
    "Alchemy/Kernel/CStackBase.cpp"
    "Alchemy/Kernel/CStepIncrementor.cpp"
    "Alchemy/Kernel/CString.cpp"
    "Alchemy/Kernel/CSymbolTable.cpp"
    "Alchemy/Kernel/CTextFileLog.cpp"
    "Alchemy/Kernel/CThreadPool.cpp"
    "Alchemy/Kernel/CVector.cpp"
    "Alchemy/Kernel/CVoronoiTessellation.cpp"
    "Alchemy/Kernel/CWordParser.cpp"
    "Alchemy/Kernel/CXForm3D.cpp"
    "Alchemy/Kernel/CryptoRandom.cpp"
    "Alchemy/Kernel/Euclid.cpp"
    "Alchemy/Kernel/ILog.cpp"
    "Alchemy/Kernel/InternetsUtil.cpp"
    "Alchemy/Kernel/Kernel.cpp"
    "Alchemy/Kernel/Math.cpp"
    "Alchemy/Kernel/Path.cpp"
    "Alchemy/Kernel/Pattern.cpp"
    "Alchemy/Kernel/RegEx.cpp"
    "Alchemy/Kernel/RunLengthCompression.cpp"
    "Alchemy/Kernel/SecureHashAlgorithm.cpp"
    "Alchemy/Kernel/Shapes.cpp"
    "Alchemy/Kernel/Time.cpp"
    "Alchemy/Kernel/UI.cpp"
    "Alchemy/Kernel/Unicode.cpp"
    "Alchemy/Kernel/Utilities.cpp"
    "Alchemy/Kernel/Voronoi.cpp"
    "Alchemy/Kernel/VoronoiGenerator.cpp"
    "Alchemy/Kernel/XForm.cpp"
    "Alchemy/Kernel/Zip.cpp"
    "Alchemy/Kernel/ZipArchive.cpp"
)

set(ALCHEMY_CODECHAIN_SOURCES
    "Alchemy/CodeChain/CCAtomTable.cpp"
    "Alchemy/CodeChain/CCDouble.cpp"
    "Alchemy/CodeChain/CCInteger.cpp"
    "Alchemy/CodeChain/CCItemPool.cpp"
    "Alchemy/CodeChain/CCLambda.cpp"
    "Alchemy/CodeChain/CCLinkedList.cpp"
    "Alchemy/CodeChain/CCNil.cpp"
    "Alchemy/CodeChain/CCNumeral.cpp"
    "Alchemy/CodeChain/CCPrimitive.cpp"
    "Alchemy/CodeChain/CCString.cpp"
    "Alchemy/CodeChain/CCSymbolTable.cpp"
    "Alchemy/CodeChain/CCTrue.cpp"
    "Alchemy/CodeChain/CCVector.cpp"
    "Alchemy/CodeChain/CConsPool.cpp"
    "Alchemy/CodeChain/CPageManager.cpp"
    "Alchemy/CodeChain/CTextFilePage.cpp"
    "Alchemy/CodeChain/CodeChain.cpp"
    "Alchemy/CodeChain/Functions.cpp"
    "Alchemy/CodeChain/ICCItem.cpp"
    "Alchemy/CodeChain/ICCItemPtr.cpp"
    "Alchemy/CodeChain/Link.cpp"
    "Alchemy/CodeChain/LoadApp.cpp"
    "Alchemy/CodeChain/pageLibrary.cpp"
    "Alchemy/CodeChain/PreComp.cpp"
)

set(ALCHEMY_XMLUTIL_SOURCES
    "Alchemy/XMLUtil/CEntityResolverList.cpp"
    "Alchemy/XMLUtil/CExternalEntityTable.cpp"
    "Alchemy/XMLUtil/CJSONMessage.cpp"
    "Alchemy/XMLUtil/CJSONValue.cpp"
    "Alchemy/XMLUtil/CXMLElement.cpp"
    "Alchemy/XMLUtil/JSONParser.cpp"
    "Alchemy/XMLUtil/Misc.cpp"
    "Alchemy/XMLUtil/Parser.cpp"
)

set(ALCHEMY_JPEG_SOURCES
    "Alchemy/IntelJPEGUtil/Load.cpp"
    "Alchemy/IntelJPEGUtil/Save.cpp"
)

set(ALCHEMY_GRAPHICS_SOURCES
    "Alchemy/Graphics/DIB.cpp"
    "Alchemy/Graphics/Misc.cpp"
    "Alchemy/Graphics/Raw.cpp"
)

set(MAMMOTH_TSE_SOURCES
    "Mammoth/TSE/CDesignCollection.cpp"
    "Mammoth/TSE/CSystem.cpp"
    "Mammoth/TSE/CUniverse.cpp"
    "Mammoth/TSE/CreateSystem.cpp"
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

add_library(mammoth_tse STATIC ${MAMMOTH_TSE_SOURCES})
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

add_library(mammoth_tsui_core INTERFACE)
target_link_libraries(mammoth_tsui_core INTERFACE
    mammoth_tse
    alchemy_kernel
    alchemy_graphics
)

add_library(platform_common INTERFACE)

add_library(platform_sdl INTERFACE)
target_link_libraries(platform_sdl INTERFACE platform_common)

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

add_library(transcendence_menu_path INTERFACE)
target_link_libraries(transcendence_menu_path INTERFACE
    mammoth_tsui_core
    mammoth_tse
    alchemy_kernel
    alchemy_graphics
    alchemy_jpeg
)

add_executable(transcendence_app EXCLUDE_FROM_ALL
    "Transcendence/Transcendence/Main.cpp"
)

target_link_libraries(transcendence_app PRIVATE
    transcendence_menu_path
    platform_common
    platform_sdl
    render_common
    render_metal
)

if(TARGET platform_macos)
    target_link_libraries(transcendence_app PRIVATE platform_macos)
endif()

set_target_properties(transcendence_app PROPERTIES
    OUTPUT_NAME "Transcendence"
)

message(STATUS "Configured bounded milestone-1 macOS scaffold.")
message(STATUS "Targets are placeholders for the audited menu-path graph and are not yet full source manifests.")
