cmake_minimum_required(VERSION 3.17)
project(MAXWELL)
set(CMAKE_CXX_STANDARD 20)
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)

include(FetchContent)
FetchContent_Declare(
  detours
  GIT_REPOSITORY https://github.com/microsoft/Detours.git
  GIT_TAG        4b8c659f549b0ab21cf649377c7a84eb708f5e68
)
FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG        1768bf971409effd74346c61446ac4f31c2bedcd
)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        975beafafdcf8cdfa1b83261ce8c8cbeaa28091b
)
FetchContent_Declare(
  toml11
  GIT_REPOSITORY https://github.com/ToruNiina/toml11.git
  GIT_TAG        85faca9cbe8d76324ff38c1801be44c63e12d5be
)

FetchContent_MakeAvailable(detours fmt imgui toml11)

# imgui
add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imconfig.h"
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui.h"
    "${imgui_SOURCE_DIR}/imgui_internal.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.h"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.h"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
)

target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# detours
add_library(detours STATIC
    "${detours_SOURCE_DIR}/src/creatwth.cpp"
    "${detours_SOURCE_DIR}/src/detours.cpp"
    "${detours_SOURCE_DIR}/src/detours.h"
    "${detours_SOURCE_DIR}/src/detver.h"
    "${detours_SOURCE_DIR}/src/disasm.cpp"
    "${detours_SOURCE_DIR}/src/disolarm.cpp"
    "${detours_SOURCE_DIR}/src/disolarm64.cpp"
    "${detours_SOURCE_DIR}/src/disolia64.cpp"
    "${detours_SOURCE_DIR}/src/disolx64.cpp"
    "${detours_SOURCE_DIR}/src/disolx86.cpp"
    "${detours_SOURCE_DIR}/src/image.cpp"
    "${detours_SOURCE_DIR}/src/modules.cpp"
    "${detours_SOURCE_DIR}/src/uimports.cpp"
)

set_property(SOURCE "${detours_SOURCE_DIR}/src/uimports.cpp" APPEND PROPERTY HEADER_FILE_ONLY true)

if(MSVC)
    target_compile_options(detours PRIVATE /w /Zi /MT /Gy /Gm- /Zl /Od)
else()
    target_compile_options(detours PRIVATE -w)
endif()

target_include_directories(detours PUBLIC "${detours_SOURCE_DIR}/src")

# fmt
set(FMT_MASTER_PROJECT OFF)

# toml
option(toml11_BUILD_TEST OFF)
# add_subdirectory(toml11)

add_library(dll SHARED
    dll/dllmain.cpp
    dll/hook.cpp dll/hook.h
    dll/ui.cpp dll/ui.h
    dll/max.cpp dll/max.h
    dll/settings.cpp dll/settings.h
    dll/memory.cpp dll/memory.h
    dll/search.cpp dll/search.h
    dll/virtual_table.cpp dll/virtual_table.h
    dll/image.cpp dll/image.h
    dll/ghidra_byte_string.h
    dll/tokenize.h
    dll/font.h
    dll/stb_image.h
    dll/stb_image_write.h
    shared/version.cpp shared/version.h
    shared/logger.h
    res/MAXWELL.rc)

target_include_directories(dll PUBLIC shared/)
target_link_libraries(dll PRIVATE detours imgui fmt toml11::toml11)
set_target_properties(dll PROPERTIES OUTPUT_NAME MAXWELL)

add_executable(exe
    injector/main.cpp
    injector/injector.cpp injector/injector.h
    injector/cmd_line.cpp injector/cmd_line.h
    shared/version.cpp shared/version.h
    shared/logger.h
    res/MAXWELL.rc)

target_include_directories(exe PUBLIC shared/)
target_link_libraries(exe PRIVATE detours fmt)
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
target_compile_definitions(exe PRIVATE GIT_VERSION=${MAXWELL_GIT_VERSION})
target_compile_definitions(dll PRIVATE GIT_VERSION=${MAXWELL_GIT_VERSION})
