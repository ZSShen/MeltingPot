
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
set(PATH_INC_SLICE "${PROJECT_SOURCE_DIR}/../plugin/slice/${NAME_INC}")
set(PATH_INC_SIMILARITY "${PROJECT_SOURCE_DIR}/../plugin/similarity/${NAME_INC}")
set(HEADER "${PATH_INC}/cluster.h")
set(SRC_CLUSTER "cluster.c")
set(SRC_CORRELATE "correlate.c")
set(SRC_PATTERN "pattern.c")
set(SRC_SPEW "spew.c")
set(SRC_DATA "data.c")
set(TGE_CLUSTER "CLUSTER")
set(OUT_CLUSTER "cluster")
set(IMPORT_CONFIG "-lconfig")
set(IMPORT_DL "-ldl")
set(IMPORT_PTHREAD "-lpthread")
set(IMPORT_MATH "-lm")


# Parse the command options.
if (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_DEBUG)
    set(PATH_OUT "${PATH_BIN}/debug")
elseif (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_RELEASE)
    set(PATH_OUT "${PATH_BIN}/release")
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
include_directories(${PATH_INC} ${GLIB2_INCLUDE_DIRS} ${PATH_INC_SLICE} ${PATH_INC_SIMILARITY})

add_executable( ${TGE_CLUSTER}
    ${SRC_CLUSTER} ${SRC_CORRELATE} ${SRC_PATTERN} ${SRC_SPEW} ${SRC_DATA}
)
target_link_libraries(${TGE_CLUSTER} ${GLIB2_LIBRARIES} ${IMPORT_CONFIG} ${IMPORT_DL}
                      ${IMPORT_PTHREAD} ${IMPORT_MATH})

set_target_properties( ${TGE_CLUSTER} PROPERTIES
    PUBLIC_HEADER ${HEADER}
    RUNTIME_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_CLUSTER}
)

message("${PROJECT_SOURCE_DIR}")

# List the installation rules
install(TARGETS ${TGE_CLUSTER}
    RUNTIME DESTINATION ${NAME_BIN}
    PUBLIC_HEADER DESTINATION ${NAME_INC}
)

