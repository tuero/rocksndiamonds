cmake_minimum_required(VERSION 3.0)
project(rocksndiamonds_headless)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})


set(CMAKE_C_FLAGS_DEBUG "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP -DHEADLESS")
set(CMAKE_C_FLAGS_MINSIZEREL "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP -DHEADLESS")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP -DHEADLESS")
set(CMAKE_C_FLAGS_RELEASE "-g -O3 -Wall -Wstrict-prototypes -Wmissing-prototypes -MMD -MP -DHEADLESS")

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -g -O3 -MMD -MP -DHEADLESS -DDEBUG -g")
set(CMAKE_CXX_FLAGS_MINSIZEREL "-Wall -Wextra -g -O3 -MMD -MP -DHEADLESS")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wextra -g -O3 -MMD -MP -DHEADLESS")
set(CMAKE_CXX_FLAGS_RELEASE "-Wall -Wextra -g -O3 -MMD -MP -DHEADLESS")


include(sourcelist.cmake)
include_directories (src/ai/includes)

set(CNFS_CMD ${PROJECT_SOURCE_DIR}/build-scripts/create_element_defs.pl)
#set(auto-conf for i in ${CNFS}; do echo \"${CNFS_CMD} $$i > $$i\"; ${CNFS_CMD} $$i > $$i; done)

#add_custom_command(
#        OUTPUT auto-conf
#        COMMAND for i in ${CNFS}; do echo "${CNFS_CMD} $$i > $$i"; ${CNFS_CMD} $$i > $$i; done
#        DEPENDS ${SOURCE_FILES} /tmp/bin/create_foo_hh main.cpp
#)

#add_custom_command(
#        OUTPUT src/conf_gfx.h
#        MAIN_DEPENDENCY src/conf_gfx.c
#        DEPENDS ${CNFS_CMD}
#        COMMAND make for i in ${CNFS}; do echo \"${CNFS_CMD} $$i > $$i\"; ${CNFS_CMD} $$i > $$i; done
#)
#add_custom_command(
#        OUTPUT src/conf_snd.h
#        MAIN_DEPENDENCY src/conf_snd.c
#        DEPENDS ${CNFS_CMD}
#        COMMAND make for i in ${CNFS}; do echo \"${CNFS_CMD} $$i > $$i\"; ${CNFS_CMD} $$i > $$i; done
#)
#add_custom_command(
#        OUTPUT src/conf_mus.h
#        MAIN_DEPENDENCY src/conf_mus.c
#        DEPENDS ${CNFS_CMD}
#        COMMAND make for i in ${CNFS}; do echo \"${CNFS_CMD} $$i > $$i\"; ${CNFS_CMD} $$i > $$i; done
#)

#add_dependencies(myTarget myCustomTarget)


add_executable(rocksndiamonds_headless ${PROJECT_SOURCE_DIR}/src/conftime.h ${PROJECT_SOURCE_DIR}/build-scripts/create_element_defs.pl ${AUTOCONF} ${sources})
#
#add_custom_target(
#        myCustomTarget
#        COMMAND for i in ${CNFS}; do echo \"${CNFS_CMD} $$i > $$i\"; ${CNFS_CMD} $$i > $$i; done
#)
#add_dependencies(rocksndiamonds_headless myCustomTarget)

target_link_libraries(rocksndiamonds_headless m)
target_link_libraries(rocksndiamonds_headless z)