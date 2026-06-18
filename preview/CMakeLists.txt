# LVGL UI Preview — 最小化 SDL 预览工程
# 用法:
#   1. 运行 setup.ps1 (Windows) 或 setup.sh (Linux/Mac)
#   2. cmake -B build -G Ninja && cmake --build build
#   3. ./build/main
#
# 用户 UI 代码路径: 相对于 preview/ 的 ../src/ui/

cmake_minimum_required(VERSION 3.12)
project(lvgl_preview C)

set(CMAKE_C_STANDARD 99)

# ---- LVGL ----
add_subdirectory(lvgl)
target_include_directories(lvgl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# ---- SDL2 ----
find_package(SDL2 REQUIRED)

# ---- 用户 UI 代码 (相对于 preview/ 的上级目录) ----
set(USER_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/../..)
file(GLOB USER_PAGES ${USER_ROOT}/src/ui/pages/*.c)
file(GLOB USER_FONTS ${USER_ROOT}/src/ui/fonts/*.c)

# ---- 预览入口 ----
add_executable(main
    ${USER_PAGES}
    ${USER_FONTS}
    main.c
)

target_link_libraries(main lvgl lvgl::examples lvgl::demos ${SDL2_LIBRARIES} m)
target_include_directories(main PRIVATE
    ${USER_ROOT}/src
    ${SDL2_INCLUDE_DIRS}
)

# Windows 控制台输出
if(WIN32)
    target_link_options(main PRIVATE "-mconsole")
endif()
