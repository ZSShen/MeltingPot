
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
set(OPT_LIB_STATIC "Static")
set(OPT_LIB_DYNAMIC "Dynamic")


# Define the constants for path generation.
set(NAME_INC "include")
set(NAME_LIB "lib")
set(NAME_LIB_STATIC "lib/static")
set(PATH_INC "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_INC}")
set(PATH_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_LIB}")
set(HEADER "${PATH_INC}/format.h")
set(SRC_SPEW "spew.c")
set(SRC_DATA "data.c")
set(SRC_PE "format_pe.c")
set(TGE_PE "FORMAT_PE")
set(OUT_PE "fmt_pe")


# Parse the command options.
if (LIB_TYPE STREQUAL OPT_LIB_STATIC)
    set(LIB_TYPE "STATIC")
elseif (LIB_TYPE STREQUAL OPT_LIB_DYNAMIC)
    set(LIB_TYPE "MODULE")
else()
    message("${RED}-- Error: LIB_TYPE is not properly specified.${RESET}")
    return()
endif()

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

add_library(${TGE_PE} ${LIB_TYPE} ${SRC_PE} ${SRC_SPEW} ${SRC_DATA})
target_link_libraries(${TGE_PE} ${GLIB2_LIBRARIES})

set_target_properties( ${TGE_PE} PROPERTIES
    PUBLIC_HEADER ${HEADER}
    ARCHIVE_OUTPUT_DIRECTORY ${PATH_OUT}
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_PE}
)


# List the installation rules
install(TARGETS ${TGE_PE}
    LIBRARY DESTINATION ${NAME_LIB}
    ARCHIVE DESTINATION ${NAME_LIB_STATIC}
    PUBLIC_HEADER DESTINATION ${NAME_INC}
)
