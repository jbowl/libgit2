cmake_minimum_required(VERSION 3.14)
project(lg2Test)

set(CMAKE_CXX_STANDARD 17)

link_directories(/home/j/libgit2/libgit2/build)
add_executable(lg2Test main.cpp)

include_directories(/home/j/libgit2/libgit2/include)


target_link_libraries(lg2Test LINK_PUBLIC git2)