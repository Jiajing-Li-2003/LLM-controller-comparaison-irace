# Install script for directory: /home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/argos3/demiurge/epuck-dao" TYPE FILE FILES
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/EpuckDAO.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/RabMessageBuffer.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel1Dot1.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel1Dot2.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel2Dot0.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel2Dot1.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel2Dot2.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel2Dot3.h"
    "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/src/ReferenceModel3Dot0.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/argos3" TYPE SHARED_LIBRARY FILES "/home/ubuntu/mandarina-supp/software/argos3-installation/demiurge-epuck-dao/build/src/libargos3_demiurge_epuck_dao.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so"
         OLD_RPATH "/home/ubuntu/argos3-installation/argos3-dist/lib/argos3:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/argos3/libargos3_demiurge_epuck_dao.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
endif()

