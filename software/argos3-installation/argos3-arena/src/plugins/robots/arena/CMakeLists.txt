set(ARGOS3_HEADERS_PLUGINS_ROBOTS_ARENA_SIMULATOR
  simulator/dynamics2d_block_model.h
  simulator/arena_entity.h
  simulator/wall_entity.h
  simulator/block_entity.h)

if(ARGOS_COMPILE_QTOPENGL)
  set(ARGOS3_HEADERS_PLUGINS_ROBOTS_ARENA_SIMULATOR
    ${ARGOS3_HEADERS_PLUGINS_ROBOTS_ARENA_SIMULATOR}
    simulator/qtopengl_block.h)
endif(ARGOS_COMPILE_QTOPENGL)

# Install location for the arena simulator headers
install(
  FILES ${ARGOS3_HEADERS_PLUGINS_ROBOTS_ARENA_SIMULATOR}
  DESTINATION include/argos3/plugins/robots/arena/simulator)

set(ARGOS3_SOURCES_PLUGINS_ROBOTS_ARENA_SIMULATOR
  ${ARGOS3_HEADERS_PLUGINS_ROBOTS_ARENA_SIMULATOR}
  simulator/dynamics2d_block_model.cpp
  simulator/arena_entity.cpp
  simulator/wall_entity.cpp
  simulator/block_entity.cpp)

if(ARGOS_COMPILE_QTOPENGL)
  set(ARGOS3_SOURCES_PLUGINS_ROBOTS_ARENA_SIMULATOR
    ${ARGOS3_SOURCES_PLUGINS_ROBOTS_ARENA_SIMULATOR}
    simulator/qtopengl_block.cpp)
endif(ARGOS_COMPILE_QTOPENGL)

add_library(argos3plugin_${ARGOS_BUILD_FOR}_arena SHARED
  ${ARGOS3_SOURCES_PLUGINS_ROBOTS_ARENA_SIMULATOR})

target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_arena
  argos3core_${ARGOS_BUILD_FOR}
  argos3plugin_${ARGOS_BUILD_FOR}_dynamics2d
  argos3plugin_${ARGOS_BUILD_FOR}_entities)

if(ARGOS_COMPILE_QTOPENGL)
  target_link_libraries(argos3plugin_${ARGOS_BUILD_FOR}_arena
    argos3plugin_${ARGOS_BUILD_FOR}_qtopengl)
endif(ARGOS_COMPILE_QTOPENGL)

# Install location for the arena plugin
install(TARGETS argos3plugin_${ARGOS_BUILD_FOR}_arena
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib/argos3
  ARCHIVE DESTINATION lib/argos3)
