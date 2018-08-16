cmake_minimum_required(VERSION 2.8)

project(usburn)

################################################################################
# Enable all Warnings
################################################################################

if("${CMAKE_CXX_FLAGS}" EQUAL ""
   OR NOT {CMAKE_CXX_FLAGS} MATCHES "-Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif()
if("${CMAKE_C_FLAGS}" EQUAL ""
   OR NOT ${CMAKE_C_FLAGS} MATCHES "-Wall")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
endif()
if("${CMAKE_CXX_FLAGS}" EQUAL ""
   OR NOT ${CMAKE_CXX_FLAGS} MATCHES "-Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
endif()
if("${CMAKE_C_FLAGS}" EQUAL ""
   OR NOT ${CMAKE_C_FLAGS} MATCHES "-Wextra")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wextra")
endif()

################################################################################
# Change output dir to project root like in original usburn
################################################################################
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} "${CMAKE_CURRENT_SOURCE_DIR}/")
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set(CURRENT_PROJECT usburn)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
  
file (GLOB  SOURCE_FILES
            "*.h"
            "*.c"
            "*.cpp")

add_executable( ${CURRENT_PROJECT} ${SOURCE_FILES} )

target_link_libraries ( ${CURRENT_PROJECT} LINK_PRIVATE
                            usb )
