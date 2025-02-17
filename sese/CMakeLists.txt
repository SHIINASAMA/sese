if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug" OR "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebugInfo")
    add_definitions("-DSESE_IS_DEBUG")
endif()

if(SESE_BUILD_TEST)
    add_subdirectory(benchmark)
    add_subdirectory(test)
endif()

if(SESE_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

# ######################################################################################################################
# General Targets
#
# 1. Core
# ######################################################################################################################
file(
    GLOB_RECURSE
    PUBLIC_SRC
    "concurrent/*.cpp"
    "concurrent/*.h"
    "event/*.h"
    "log/*.cpp"
    "log/*.h"
    "convert/*.cpp"
    "convert/*.h"
    "io/*.cpp"
    "io/*.h"
    "thread/*.cpp"
    "thread/*.h"
    "system/*.cpp"
    "system/*.h"
    "net/*.cpp"
    "net/*.h"
    "config/*.cpp"
    "config/*.h"
    "text/*.cpp"
    "text/*.h"
    "util/*.cpp"
    "util/*.h"
    "security/*.cpp"
    "security/*.h"
    "plugin/*.cpp"
    "plugin/*.h"
    "service/*.cpp"
    "service/*.h"
    "res/*.cpp"
    "res/*.h"
    "container/*.cpp"
    "container/*.h"
)

file(GLOB_RECURSE UNIX_SRC "internal/unix/*.cpp" "internal/unix/*.h" "native/unix/*.cpp" "native/unix/*.h")

file(GLOB_RECURSE INTERNAL_SRC "internal/net/*.cpp" "internal/net/*.h" "internal/service/*.cpp" "internal/service/*.h")

add_library(Core ${PUBLIC_SRC} ${INTERNAL_SRC} Config.h Util.h)

if(SESE_BUILD_TEST)
    target_compile_definitions(Core PUBLIC -DSESE_BUILD_TEST)
endif()

target_include_directories(Core PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}> $<INSTALL_INTERFACE:include>)

set_target_properties(
    Core
    PROPERTIES CXX_STANDARD 20
               OUTPUT_NAME "sese.core"
               PREFIX ""
               ENABLE_EXPORTS ON
               WINDOWS_EXPORT_ALL_SYMBOLS ON
)

include(${PROJECT_SOURCE_DIR}/cmake/Manifest.cmake)
target_manifest(Core manifest.json)

if(MSVC AND NOT ${CMAKE_BUILD_TYPE} MATCHES "Debug")
    target_compile_options(Core PRIVATE /Zi)
    set_target_properties(Core PROPERTIES LINK_FLAGS "/DEBUG /OPT:REF /OPT:ICF")
endif()

target_compile_definitions(Core PRIVATE "SESE_MAJOR_VERSION=\"${PROJECT_VERSION_MAJOR}\"")
target_compile_definitions(Core PRIVATE "SESE_MINOR_VERSION=\"${PROJECT_VERSION_MINOR}\"")
target_compile_definitions(Core PRIVATE "SESE_PATCH_VERSION=\"${PROJECT_VERSION_PATCH}\"")
target_compile_definitions(Core PRIVATE "SESE_REPO_HASH=\"${SESE_REPO_HASH}\"")
target_compile_definitions(Core PRIVATE "SESE_REPO_BRANCH=\"${SESE_REPO_BRANCH}\"")

if(SESE_REPLACE_EXECINFO)
    target_link_libraries(Core PRIVATE PkgConfig::libunwind)
endif()
target_link_libraries(Core PRIVATE OpenSSL::Crypto OpenSSL::SSL)
target_link_libraries(Core PRIVATE ZLIB::ZLIB)
target_link_libraries(Core PRIVATE asio::asio)
target_link_libraries(Core PRIVATE simdjson::simdjson)

# ######################################################################################################################
# Internally independent sub-libraries and options
#
# 1. Archive
# 2. DB
# 3. Async Logger
# 4. SESE_RECORD_TIME_PATTERN
# 5. SESE_RECORD_TEXT_PATTERN
# 6. Replace execinfo
# ######################################################################################################################
if(MSVC)
    add_definitions(/utf-8)
endif()
include_directories(${PROJECT_SOURCE_DIR})

if(SESE_USE_ARCHIVE)
    add_subdirectory(archive)
    target_link_libraries(Core PRIVATE Archive)
    set(ARCHIVE_TARGET Archive)
endif()

add_subdirectory(db)
target_link_libraries(Core PRIVATE DB)

if(SESE_USE_ASYNC_LOGGER)
    target_compile_definitions(Core PRIVATE -DUSE_ASYNC_LOGGER)
endif()

set(SESE_RECORD_TIME_PATTERN $ENV{SESE_RECORD_TIME_PATTERN})
if(SESE_RECORD_TIME_PATTERN)
    target_compile_definitions(Core PRIVATE "SESE_RECORD_TIME_PATTERN=\"${SESE_RECORD_TIME_PATTERN}\"")
endif()

set(SESE_RECORD_TEXT_PATTERN $ENV{SESE_RECORD_TEXT_PATTERN})
if(SESE_RECORD_TEXT_PATTERN)
    target_compile_definitions(Core PRIVATE "SESE_RECORD_TEXT_PATTERN=\"${SESE_RECORD_TEXT_PATTERN}\"")
endif()

if(SESE_REPLACE_EXECINFO)
    target_compile_definitions(Core PRIVATE -DUSE_LIBUNWIND)
endif()

# ######################################################################################################################
# Additional task configurations
# ######################################################################################################################
if(SESE_BUILD_TEST AND WIN32)
    target_compile_definitions(Core PUBLIC -DSESE_BUILD_TEST)

    sese_copy_dll_to_binary_path(Core "/sese/test")
    sese_copy_dll_to_binary_path(Core "/sese/benchmark")
endif()

if(SESE_BUILD_EXAMPLE AND WIN32)
    sese_copy_dll_to_binary_path(Core "/demo")
endif()

# ######################################################################################################################
# Windows platform configuration
# ######################################################################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    file(GLOB_RECURSE NATIVE_WIN_SRC "internal/win/*.cpp" "internal/win/*.h")

    target_sources(Core PRIVATE ${NATIVE_WIN_SRC})
    target_compile_definitions(Core PRIVATE -D_WIN32_WINNT=0x0601)

    if(MSVC)
        target_compile_options(Core PRIVATE "/utf-8")
    endif()
    target_link_libraries(
        Core
        PUBLIC crypt32
               ws2_32
               wsock32
               dbghelp
               iphlpapi
               advapi32
               secur32
    )
endif()

# ######################################################################################################################
# Linux platform configuration
# ######################################################################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_link_libraries(Core PRIVATE ${CMAKE_THREAD_LIBS_INIT})
    target_link_libraries(Core PRIVATE ${CMAKE_DL_LIBS})

    file(GLOB_RECURSE NATIVE_LINUX_SRC "internal/linux/*.cpp" "internal/linux/*.h"
    )
    target_sources(Core PRIVATE ${NATIVE_LINUX_SRC})

    set_target_properties(Core PROPERTIES POSITION_INDEPENDENT_CODE ON)
    target_link_options(Core PUBLIC "-rdynamic")
endif()

# ######################################################################################################################
# macOS platform configuration
# ######################################################################################################################
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    target_link_libraries(Core PRIVATE ${COREFOUNDATION_FRAMEWORK} ${IOKIT_FRAMEWORK} ${CORESERVICES_FRAMEWORK})

    file(GLOB_RECURSE NATIVE_DARWIN_SRC "internal/darwin/*.cpp" "internal/darwin/*.h"
    )
    target_sources(Core PRIVATE ${NATIVE_DARWIN_SRC})
endif()

# ######################################################################################################################
# Unix-Like platform configuration
# ######################################################################################################################
if(UNIX)
    include(CheckFunctionExists)
    target_sources(Core PRIVATE ${UNIX_SRC})
    check_function_exists(secure_getenv HAVE_SECURE_GETENV)
endif()

include(${PROJECT_SOURCE_DIR}/cmake/Install.cmake)
