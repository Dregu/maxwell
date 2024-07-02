cmake_minimum_required(VERSION 3.17)
project(MAXWELL)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

# imgui
add_library(imgui STATIC
    imgui/imconfig.h
    imgui/imgui.cpp
    imgui/imgui.h
    imgui/imgui_internal.h
    imgui/backends/imgui_impl_dx12.cpp
    imgui/backends/imgui_impl_dx12.h
    imgui/backends/imgui_impl_win32.cpp
    imgui/backends/imgui_impl_win32.h
    imgui/imgui_widgets.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/misc/cpp/imgui_stdlib.h
    imgui/misc/cpp/imgui_stdlib.cpp)

target_include_directories(imgui PUBLIC imgui)

# detours
add_library(detours STATIC
    detours/src/creatwth.cpp
    detours/src/detours.cpp
    detours/src/detours.h
    detours/src/detver.h
    detours/src/disasm.cpp
    detours/src/disolarm.cpp
    detours/src/disolarm64.cpp
    detours/src/disolia64.cpp
    detours/src/disolx64.cpp
    detours/src/disolx86.cpp
    detours/src/image.cpp
    detours/src/modules.cpp
    detours/src/uimports.cpp)

set_property(
    SOURCE detours/src/uimports.cpp
    APPEND PROPERTY HEADER_FILE_ONLY true)

if(MSVC)
    target_compile_options(detours PRIVATE /w /Zi /MT /Gy /Gm- /Zl /Od)
else()
    target_compile_options(detours PRIVATE -w)
endif()

target_include_directories(detours PUBLIC detours/src)

# fmt
set(FMT_MASTER_PROJECT OFF)
add_subdirectory(fmt)

add_library(dll SHARED
    dllmain.cpp
    hook.cpp hook.h
    ui.cpp ui.h
    max.cpp max.h
    memory.cpp memory.h
    search.cpp search.h
    virtual_table.cpp virtual_table.h
    version.cpp version.h
    image.cpp image.h
    ghidra_byte_string.h
    tokenize.h
    logger.h
    font.h
    stb_image.h
    stb_image_write.h
    res/MAXWELL.rc)

target_link_libraries(dll PRIVATE
    detours
    imgui
    fmt
    toml11::toml11)

set_target_properties(dll PROPERTIES OUTPUT_NAME MAXWELL)

# toml
option(toml11_BUILD_TEST OFF)
add_subdirectory(toml11)

add_executable(exe
    main.cpp
    injector.cpp injector.h
    cmd_line.cpp cmd_line.h
    version.cpp version.h
    logger.h
    res/MAXWELL.rc)
target_link_libraries(exe PRIVATE
    detours
    fmt)
add_dependencies(exe dll)
set_target_properties(exe PROPERTIES OUTPUT_NAME MAXWELL)

add_compile_definitions(NOMINMAX)
add_compile_definitions(WIN32_LEAN_AND_MEAN)

execute_process(
    COMMAND git describe --always --dirty=-modified
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE MAXWELL_GIT_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(exe PRIVATE
    GIT_VERSION=${MAXWELL_GIT_VERSION}
)
target_compile_definitions(dll PRIVATE
    GIT_VERSION=${MAXWELL_GIT_VERSION}
)
