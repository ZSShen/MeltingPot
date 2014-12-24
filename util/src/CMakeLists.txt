
cmake_minimum_required(VERSION 2.8)


# Define the color code for error message.
if (NOT WIN32)
    string(ASCII 27 Esc)
    set(RESET "${Esc}[m")
    set(RED "${Esc}[31m")
endif()


# Define the constants to parse command options.
set(OPT_BUILD_DEBUG "Debug")
set(OPT_BUILD_RELEASE "Release")


# Define the constants for path generation.
set(NAME_INC "include")
set(NAME_LIB "lib")
set(NAME_LIB_STATIC "lib/static")
set(PATH_INC "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_INC}")
set(PATH_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_LIB}")
set(HEADER_SPEW "${PATH_INC}/spew.h")
set(HEADER_DATA "${PATH_INC}/data.h")
set(SRC_SPEW "spew.c")
set(SRC_DATA "data.c")
set(TGE_SPEW "UTIL_SPEW")
set(TGE_DATA "UTIL_DATA")
set(OUT_SPEW "spew")
set(OUT_DATA "data")
set(LIB_TYPE "SHARED")


# Parse the command options.
if (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_DEBUG)
    set(PATH_OUT "${PATH_LIB}/debug")
elseif (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_RELEASE)
    set(PATH_OUT "${PATH_LIB}/release")
else()
    message("${RED}-- Error: CMAKE_BUILD_TYPE is not properly specified.${RESET}")
    return()
endif()


# Check for glib-2.0
find_package(PkgConfig)
pkg_check_modules(GLIB2 glib-2.0)
if (GLIB2_NOTFOUND)
    message("${RED}-- Error: Cannot find the glib-2.0 library.${RESET}")
endif()


# List the source building rules.
include_directories(${PATH_INC} ${GLIB2_INCLUDE_DIRS})

add_library(${TGE_SPEW} ${LIB_TYPE} ${SRC_SPEW})
add_library(${TGE_DATA} ${LIB_TYPE} ${SRC_DATA})
target_link_libraries(${TGE_DATA} ${GLIB2_LIBRARIES})

set_target_properties( ${TGE_SPEW} PROPERTIES
    PUBLIC_HEADER ${HEADER_SPEW}
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_SPEW}
)
set_target_properties( ${TGE_DATA} PROPERTIES
    PUBLIC_HEADER ${HEADER_DATA}
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_DATA}
)


# List the installation rules
install(TARGETS ${TGE_SPEW} ${TGE_DATA}
    LIBRARY DESTINATION ${NAME_LIB}
    ARCHIVE DESTINATION ${NAME_LIB_STATIC}
    PUBLIC_HEADER DESTINATION ${NAME_INC}
)
