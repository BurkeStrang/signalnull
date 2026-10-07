cmake_minimum_required(VERSION 3.16)
project(signal_null VERSION 0.2.0 LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
# Written to the build directory for clangd.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(SIGNAL_HEADLESS "Build portable core without SDL3 (no desktop play)" OFF)
add_executable(signal-null
    src/main.c
    src/game/game.c
    src/game/player.c
    src/game/weapon.c
    src/game/enemy.c
    src/game/scores.c
    src/world/world.c
    src/world/collision.c
    src/renderer/renderer.c
    src/renderer/texture.c
    src/renderer/font.c
    src/test/selftest.c)
target_include_directories(signal-null PRIVATE src)
target_include_directories(signal-null SYSTEM PRIVATE third_party)
# Fallback for when the game is not started from the project directory (e.g. ctest).
target_compile_definitions(signal-null PRIVATE SIGNAL_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets")
if(SIGNAL_HEADLESS)
    target_compile_definitions(signal-null PRIVATE SIGNAL_HEADLESS)
else()
    find_package(SDL3 3.2 REQUIRED CONFIG)
    target_sources(signal-null PRIVATE src/platform/platform.c)
    target_link_libraries(signal-null PRIVATE SDL3::SDL3)
    if(WIN32 AND TARGET SDL3::SDL3-shared)
        add_custom_command(TARGET signal-null POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3-shared> $<TARGET_FILE_DIR:signal-null>)
    endif()
endif()
# Optional: the renderer fills rows in parallel when the compiler has OpenMP.
find_package(OpenMP)
if(OpenMP_C_FOUND)
    target_link_libraries(signal-null PRIVATE OpenMP::OpenMP_C)
endif()
if(NOT MSVC)
    target_compile_options(signal-null PRIVATE -Wall -Wextra -Wpedantic)
    target_link_libraries(signal-null PRIVATE m)
else()
    target_compile_options(signal-null PRIVATE /W4)
endif()
enable_testing()
add_test(NAME core COMMAND signal-null --test)
