cmake_minimum_required(VERSION 3.0)
project(rocksndiamonds_headless)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# C compiler flags
set(CMAKE_C_FLAGS "-Wall -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS} -g")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 ")

# C++ compiler flags
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -DDEBUG ")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 ")

# Special flags for clang
if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-extern-c-compat")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wno-extern-c-compat")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wno-extern-c-compat -O3")
endif()
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-extern-c-compat")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -Wno-extern-c-compat")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -Wno-extern-c-compat -O3")
endif()

# Options for comparing different LevinTS policies
option(TRIVIAL_POLICY "Trivial LevinTS policy" OFF)

if(TRIVIAL_POLICY)
    add_definitions(-DTRIVIAL_POLICY)
endif()

# Log/replay/stat directories 
message(STATUS "Creating log/replay/stat directories.")
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/ai/logs)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/ai/replays)
file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/src/ai/stats)

# Includes
include(sourcelist.cmake)
include_directories (${PROJECT_SOURCE_DIR}/src/ai/include)
include_directories (${PROJECT_SOURCE_DIR}/src/ai/extern/plog/include)
include_directories (${PROJECT_SOURCE_DIR}/src/ai/extern/Catch2/single_include)

# Find SDL2 libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
find_package(SDL2)
find_package(SDL2_image)
find_package(SDL2_mixer)
find_package(SDL2_net)

# Find Pytorch libary
find_package(Torch REQUIRED)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${TORCH_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${TORCH_CXX_FLAGS}")


# Handle SDL2 libraries
if(SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_net_FOUND)
    message(STATUS "SDL2 found, configuring both GUI and HEADLESS build.")
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS} ${SDL2_MIXER_INCLUDE_DIRS} ${SDL2_NET_INCLUDE_DIRS})
else()
    message(STATUS "Skipping GUI configuration.")
endif()
if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found.")
endif()
if(NOT SDL2_image_FOUND)
    message(STATUS "SDL2_image not found.")
endif()
if(NOT SDL2_mixer_FOUND)
    message(STATUS "SDL2_mixer not found.")
endif()
if(NOT SDL2_net_FOUND)
    message(STATUS "SDL2_net not found.")
endif()


set(CNFS_CMD ${PROJECT_SOURCE_DIR}/build-scripts/create_element_defs.pl)

# conftime header with datetime
set(conftime ${PROJECT_SOURCE_DIR}/src/conftime.h)
add_custom_command(
        OUTPUT ${conftime}
        DEPENDS ${sources_rnd}
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
        COMMAND ${PROJECT_SOURCE_DIR}/build-scripts/string_date.sh
)
add_custom_target(
        auto_conftime
        ALL
        DEPENDS ${conftime}
)

# configuration header/source files
add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/src/conf_gfx.h
        DEPENDS ${CNFS_CMD} ${PROJECT_SOURCE_DIR}/src/conf_gfx.c
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
        COMMAND ${PROJECT_SOURCE_DIR}/build-scripts/create_confs.sh
)
add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/src/conf_snd.h
        DEPENDS ${CNFS_CMD} ${PROJECT_SOURCE_DIR}/src/conf_snd.c
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
        COMMAND ${PROJECT_SOURCE_DIR}/build-scripts/create_confs.sh
)
add_custom_command(
        OUTPUT ${PROJECT_SOURCE_DIR}/src/conf_mus.h
        DEPENDS ${CNFS_CMD} ${PROJECT_SOURCE_DIR}/src/conf_mus.c
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src/
        COMMAND ${PROJECT_SOURCE_DIR}/build-scripts/create_confs.sh
)
add_custom_target(
        auto_conf
        ALL
        DEPENDS ${AUTOCONF}
)

# Executable
if (SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_net_FOUND)
    add_executable(rocksndiamonds ${sources_rnd} ${sources_ai})
    add_dependencies(rocksndiamonds auto_conf auto_conftime)
    set_target_properties(rocksndiamonds PROPERTIES COMPILE_FLAGS "-DTARGET_SDL2")
endif()

# These are always added as they don't depend on SDL2
add_executable(rocksndiamonds_headless ${sources_rnd} ${sources_ai})
add_dependencies(rocksndiamonds_headless auto_conf auto_conftime)
set_target_properties(rocksndiamonds_headless PROPERTIES COMPILE_FLAGS "-DHEADLESS")

add_executable(run_tests ${sources_rnd} ${sources_ai} ${sources_tests})
add_dependencies(run_tests auto_conf auto_conftime)
set_target_properties(run_tests PROPERTIES COMPILE_FLAGS "-DHEADLESS -DRUN_TESTS")
set_target_properties(run_tests PROPERTIES EXCLUDE_FROM_ALL TRUE)


# Dependendies
if (SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_net_FOUND)
    target_link_libraries(rocksndiamonds m z ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_NET_LIBRARIES} ${SDL2_LIBRARIES} ${TORCH_LIBRARIES})
endif()

target_link_libraries(rocksndiamonds_headless m z ${TORCH_LIBRARIES})
target_link_libraries(run_tests m z ${TORCH_LIBRARIES})
