# Set up ARGoS compilation information
include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS})
link_directories(${ARGOS_LIBRARY_DIRS})

# Headers
set(AUTOMODE_HEADERS
	core/AutoMoDeController.h
	core/AutoMoDeFiniteStateMachine.h
	core/AutoMoDeFsmBuilder.h
	core/AutoMoDeFsmHistory.h
	# Behaviours
	modules/AutoMoDeBehaviour.h
	modules/AutoMoDeBehaviourAntiPhototaxis.h
	modules/AutoMoDeBehaviourPhototaxis.h
	modules/AutoMoDeBehaviourAttraction.h
	modules/AutoMoDeBehaviourRepulsion.h
	modules/AutoMoDeBehaviourStop.h
	modules/AutoMoDeBehaviourExploration.h
    modules/AutoMoDeBehaviourGoToColor.h
    modules/AutoMoDeBehaviourGoAwayColor.h
	# Conditions
	modules/AutoMoDeCondition.h
	modules/AutoMoDeConditionBlackFloor.h
	modules/AutoMoDeConditionWhiteFloor.h
	modules/AutoMoDeConditionGrayFloor.h
	modules/AutoMoDeConditionNeighborsCount.h
	modules/AutoMoDeConditionInvertedNeighborsCount.h
    modules/AutoMoDeConditionFixedProbability.h
    modules/AutoMoDeConditionProbColor.h)

# Sources
set(AUTOMODE_SOURCES
	core/AutoMoDeController.cpp
	core/AutoMoDeFiniteStateMachine.cpp
	core/AutoMoDeFsmBuilder.cpp
	core/AutoMoDeFsmHistory.cpp
	# Behaviours
	modules/AutoMoDeBehaviour.cpp
	modules/AutoMoDeBehaviourAntiPhototaxis.cpp
	modules/AutoMoDeBehaviourPhototaxis.cpp
	modules/AutoMoDeBehaviourAttraction.cpp
	modules/AutoMoDeBehaviourRepulsion.cpp
	modules/AutoMoDeBehaviourStop.cpp
	modules/AutoMoDeBehaviourExploration.cpp
    modules/AutoMoDeBehaviourGoToColor.cpp
    modules/AutoMoDeBehaviourGoAwayColor.cpp
	# Conditions
	modules/AutoMoDeCondition.cpp
	modules/AutoMoDeConditionBlackFloor.cpp
	modules/AutoMoDeConditionWhiteFloor.cpp
	modules/AutoMoDeConditionGrayFloor.cpp
	modules/AutoMoDeConditionNeighborsCount.cpp
	modules/AutoMoDeConditionInvertedNeighborsCount.cpp
    modules/AutoMoDeConditionFixedProbability.cpp
    modules/AutoMoDeConditionProbColor.cpp)

#add_executable(automode /home/arena/dgarzon/iridia-tracking-system/src/plugins/robots/e-puck/real_robot/real_epuck_its.h /home/arena/dgarzon/iridia-tracking-system/src/plugins/robots/e-puck/real_robot/real_epuck_its_main.cpp ${AUTOMODE_HEADERS} ${AUTOMODE_SOURCES})
#target_link_libraries(automode argos3plugin_${ARGOS_BUILD_FOR}_epuck argos3plugin_${ARGOS_BUILD_FOR}_epuck argos3_demiurge_epuck_dao argos3plugin_${ARGOS_BUILD_FOR}_genericvirtualsensorsandactuators)

add_executable(automode /home/trackingsystem/dgarzon/sw-mandarina/argos3-installation/argos3-tycho/src/plugins/robots/e-puck/real_robot/real_epuck_its.h /home/trackingsystem/dgarzon/sw-mandarina/argos3-installation/argos3-tycho/src/plugins/robots/e-puck/real_robot/real_epuck_its_main.cpp ${AUTOMODE_HEADERS} ${AUTOMODE_SOURCES})
target_link_libraries(automode argos3plugin_${ARGOS_BUILD_FOR}_epuck argos3_demiurge_epuck_dao argos3plugin_${ARGOS_BUILD_FOR}_genericvirtualsensorsandactuators)