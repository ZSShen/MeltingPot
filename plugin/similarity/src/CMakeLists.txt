
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
set(HEADER "${PATH_INC}/similarity.h")
set(SRC_SSDEEP "similarity_ssdeep.c")
set(SRC_NGRAM "similarity_ngram.c")
set(TGE_SSDEEP "SIM_SSDEEP")
set(TGE_NGRAM "SIM_NGRAM")
set(OUT_SSDEEP "sim_ssdeep")
set(OUT_NGRAM "sim_ngram")
set(IMPORT_FUZZY "-lfuzzy")


# Parse the command options.
if (LIB_TYPE STREQUAL OPT_LIB_STATIC)
    set(LIB_TYPE "STATIC")
elseif (LIB_TYPE STREQUAL OPT_LIB_DYNAMIC)
    set(LIB_TYPE "MODULE")
else()
    message("${RED}-- Error: LIB_TYPE is not properly specified.${RESET}")
    return()
endif()
# The specialized options for ngram similarity comparison.
if (NGRAM_DIMENSION)
    add_definitions(-DNGRAM_DIMENSION=${NGRAM_DIMENSION})
endif()
if (BLOOM_FILTER_WIDTH)
    add_definitions(-DBLOOM_FILTER_WIDTH=${BLOOM_FILTER_WIDTH})
endif()


if (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_DEBUG)
    set(PATH_OUT "${PATH_LIB}/debug")
elseif (CMAKE_BUILD_TYPE STREQUAL OPT_BUILD_RELEASE)
    set(PATH_OUT "${PATH_LIB}/release")
else()
    message("${RED}-- Error: CMAKE_BUILD_TYPE is not properly specified.${RESET}")
    return()
endif()


# List the building rules.
include_directories(${PATH_INC})

add_library(${TGE_SSDEEP} ${LIB_TYPE} ${SRC_SSDEEP})
target_link_libraries(${TGE_SSDEEP} ${IMPORT_FUZZY})
add_library(${TGE_NGRAM} ${LIB_TYPE} ${SRC_NGRAM})

set_target_properties( ${TGE_SSDEEP} PROPERTIES
    PUBLIC_HEADER ${HEADER}
    ARCHIVE_OUTPUT_DIRECTORY ${PATH_OUT}
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_SSDEEP}
)
set_target_properties( ${TGE_NGRAM} PROPERTIES
    PUBLIC_HEADER ${HEADER}
    ARCHIVE_OUTPUT_DIRECTORY ${PATH_OUT}
    LIBRARY_OUTPUT_DIRECTORY ${PATH_OUT}
    OUTPUT_NAME ${OUT_NGRAM}
)

# List the installation rules
install(TARGETS ${TGE_SSDEEP} ${TGE_NGRAM}
    LIBRARY DESTINATION ${NAME_LIB}
    ARCHIVE DESTINATION ${NAME_LIB_STATIC}
    PUBLIC_HEADER DESTINATION ${NAME_INC}
)
