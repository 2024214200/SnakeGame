cmake_minimum_required(VERSION 3.28.3)
project(greedy_snake_c C)

set(CMAKE_C_STANDARD 11)


# 查找 GTK3
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# 查找 SQLite3
find_package(SQLite3 REQUIRED)

# 包含 GTK3 头文件目录
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

include_directories(${SQLITE3_INCLUDE_DIRS})


add_executable(greedy_snake_c snake.c)

# 链接 GTK3 库
target_link_libraries(greedy_snake_c ${GTK3_LIBRARIES})
