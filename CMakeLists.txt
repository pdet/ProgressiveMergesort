cmake_minimum_required(VERSION 3.9)
project(ProgressiveMergesort)

set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
include_directories(src/include/)

set(updates_sources src/experiment.cpp src/cracking/cracking.cpp src/cracking/avl_tree.cpp
        src/progressive/progressive_quicksort.cpp src/progressive/progressive_mergesort.cpp
        src/progressive/progressive_index.cpp)

add_library(updates ${updates_sources})

# Compile main
set(main_sources src/main.cpp)
add_executable(main ${main_sources})
target_link_libraries(main updates)

# Compile tests
set(test_sources test/test.cpp)
add_executable(tests ${test_sources})
target_link_libraries(tests updates)