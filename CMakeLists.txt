cmake_minimum_required( VERSION 3.17 FATAL_ERROR )
message( STATUS "CMake v${CMAKE_VERSION}" )

## Setup

set( CMAKE_CONFIGURATION_TYPES    "Release" )
set( CMAKE_BUILD_TYPE             "Release" )
set( CMAKE_CXX_STANDARD           20 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )
set( CMAKE_SKIP_INSTALL_RULES     TRUE )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake" )

include( DisableInSourceBuild )
DisableInSourceBuild()

project( EWAN VERSION 0.1 LANGUAGES CXX )

set( ${PROJECT_NAME}_ROOTDIR "${CMAKE_CURRENT_LIST_DIR}" )

## Code

add_subdirectory( Libs )
add_subdirectory( Embed )
add_subdirectory( Source )
add_subdirectory( Test EXCLUDE_FROM_ALL )

## Prettify IDE

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake" )

source_group( " "               REGULAR_EXPRESSION "\.([CcHh]|[CcHh][Pp][Pp])$" )
source_group( "CMake"           REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
source_group( "${PROJECT_NAME}" REGULAR_EXPRESSION ${PROJECT_NAME} )
