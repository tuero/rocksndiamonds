cmake_minimum_required(VERSION 3.0)
project(rocksndiamonds_headless)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


#set(CMAKE_C_FLAGS "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP")
#set(CMAKE_C_FLAGS_DEBUG "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP")
#set(CMAKE_C_FLAGS_MINSIZEREL "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP")
#set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP")
#set(CMAKE_C_FLAGS_RELEASE "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP")
#
#set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O3 -MMD -MP -DDEBUG -g")
#set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O3 -MMD -MP -DDEBUG -g")
#set(CMAKE_CXX_FLAGS_MINSIZEREL "-Wall -Wextra -g -O3 -MMD -MP")
#set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wextra -g -O3 -MMD -MP")
#set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -g -O3 -MMD -MP")

set(CMAKE_C_FLAGS "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS_DEBUG "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS_MINSIZEREL "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes")
set(CMAKE_C_FLAGS_RELEASE "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes")

set(CMAKE_CXX_FLAGS "-Wall -Wextra -g -O3 -MMD -g")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O3 -DDEBUG -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Wall -Wextra -g -O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wextra -g -O3")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -g -O3")


# Includes
include(sourcelist.cmake)
include_directories (${PROJECT_SOURCE_DIR}/src/ai/includes)

# Find SDL2 libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
find_package(SDL2)
find_package(SDL2_image)
find_package(SDL2_mixer)
find_package(SDL2_net)

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
        DEPENDS ${sources}
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
    add_executable(rocksndiamonds_gui ${sources})
        add_dependencies(rocksndiamonds_gui auto_conf auto_conftime)
    set_target_properties(rocksndiamonds_gui PROPERTIES COMPILE_FLAGS "-DTARGET_SDL2")
endif()

add_executable(rocksndiamonds_headless ${sources})
add_dependencies(rocksndiamonds_headless auto_conf auto_conftime)
set_target_properties(rocksndiamonds_headless PROPERTIES COMPILE_FLAGS "-DHEADLESS")


# Dependendies
if (SDL2_FOUND AND SDL2_image_FOUND AND SDL2_mixer_FOUND AND SDL2_net_FOUND)
    target_link_libraries(rocksndiamonds_gui m z)
    target_link_libraries(rocksndiamonds_gui ${SDL2_IMAGE_LIBRARIES} ${SDL2_MIXER_LIBRARIES} ${SDL2_NET_LIBRARIES} ${SDL2_LIBRARIES})
endif()

target_link_libraries(rocksndiamonds_headless m z)
