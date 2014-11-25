
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
set(NAME_BIN "bin")
set(PATH_INC "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_INC}")
set(PATH_LIB "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_LIB}")
set(PATH_BIN "${CMAKE_CURRENT_SOURCE_DIR}/../${NAME_BIN}")
set(HEADER "${PATH_INC}/cluster.h")
set(SRC_CLUSTER "cluster.c")
set(SRC_CORRELATE "correlate.c")
set(SRC_PATTERN "pattern.c")
set(SRC_UTIL "util.c")
set(SRC_DATA "data.c")
set(TGE_CLUSTER "CLUSTER")
set(OUT_CLUSTER "cluster")


# Parse the command options.
if (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_DEBUG)
    set(PATH_OUT "${PATH_BIN}/debug")
elseif (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_RELEASE)
    set(PATH_OUT "${PATH_BIN}/release")
else()
    message("${RED}-- Error: CMAKE_BUILD_TYPE is not properly specified.${RESET}")
    return()
endif()


# List the source building rules.
include_directories(${PATH_INC})

add_executable( ${TGE_CLUSTER} 
    ${SRC_CLUSTER} ${SRC_CORRELATE} ${SRC_PATTERN} ${SRC_UTIL} ${SRC_DATA}
)

set_target_properties( ${TGE_CLUSTER} PROPERTIES
    PUBLIC_HEADER ${HEADER}
    RUNTIME_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_CLUSTER}
)


# List the installation rules
install(TARGETS ${TGE_CLUSTER}
    RUNTIME DESTINATION ${NAME_BIN}
    PUBLIC_HEADER DESTINATION ${NAME_INC}
)

