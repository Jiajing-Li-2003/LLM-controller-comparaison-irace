# Set up ARGoS compilation information
include_directories(${CMAKE_SOURCE_DIR} ${ARGOS_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS}) #pr API
link_directories(${ARGOS_LIBRARY_DIRS})
link_directories(
	${CMAKE_BINARY_DIR}/src
    )

add_library(tutti_st_loopfunc SHARED TuttiStLoopFunc.h TuttiStLoopFunc.cpp)
target_link_libraries(tutti_st_loopfunc argos3plugin_${ARGOS_BUILD_FOR}_epuck)

add_library(tutti_ag_loopfunc SHARED TuttiAgLoopFunc.h TuttiAgLoopFunc.cpp)
target_link_libraries(tutti_ag_loopfunc argos3plugin_${ARGOS_BUILD_FOR}_epuck)

add_library(tutti_fr_loopfunc SHARED TuttiFrLoopFunc.h TuttiFrLoopFunc.cpp)
target_link_libraries(tutti_fr_loopfunc argos3plugin_${ARGOS_BUILD_FOR}_epuck)

add_library(seq_loopfunc SHARED SeqLoopFunc.h SeqLoopFunc.cpp)
target_link_libraries(seq_loopfunc argos3plugin_${ARGOS_BUILD_FOR}_epuck)
target_link_libraries(seq_loopfunc PUBLIC ${CURL_LIBRARIES})#pr API

