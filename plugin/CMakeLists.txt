
cmake_minimum_required(VERSION 2.8)
set(CMAKE_INSTALL_PREFIX "")
project(PLUGIN)


set(DIR_SLC "${CMAKE_CURRENT_SOURCE_DIR}/slice")
set(DIR_SIM "${CMAKE_CURRENT_SOURCE_DIR}/similarity")
set(DIR_FMT "${CMAKE_CURRENT_SOURCE_DIR}/format")


add_subdirectory(${DIR_SLC})
add_subdirectory(${DIR_SIM})
add_subdirectory(${DIR_FMT})
