# Install script for directory: /home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/ubuntu/argos3-installation/argos3-dist")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/argos3/plugins/robots/e-puck/control_interface" TYPE FILE FILES
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_proximity_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_ground_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_light_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_encoder_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_accelerometer_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_ircom_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_micro_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_battery_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_omnidirectional_camera_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_virtual_camrab_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_base_leds_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_rgb_leds_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_range_and_bearing_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_wheels_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_ircom_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/control_interface/ci_epuck_virtual_camrab_actuator.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/argos3/plugins/robots/e-puck/simulator" TYPE FILE FILES
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/dynamics2d_epuck_model.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_entity.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/battery_equipped_entity.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_base_leds_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_rgb_leds_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_wheels_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_range_and_bearing_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_virtual_camrab_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_virtual_camrab_default_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_ircom_default_actuator.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_omnidirectional_camera_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_proximity_default_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_battery_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_light_default_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_range_and_bearing_default_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_ircom_default_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_ground_rotzonly_sensor.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/epuck_rab_equipped_entity.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/src/plugins/robots/e-puck/simulator/qtopengl_epuck.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/argos3" TYPE SHARED_LIBRARY FILES "/home/ubuntu/mandarina-supp/software/argos3-installation/argos3-epuck/build/plugins/robots/e-puck/libargos3plugin_simulator_epuck.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so"
         OLD_RPATH "/home/ubuntu/argos3-installation/argos3-dist/lib/argos3:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3plugin_simulator_epuck.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

