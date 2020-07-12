cmake_minimum_required( VERSION 3.17 FATAL_ERROR )
message( STATUS "CMake v${CMAKE_VERSION}" )

##
## Setup
##

set( CMAKE_CONFIGURATION_TYPES    Release )
set( CMAKE_BUILD_TYPE             Release )
set( CMAKE_CXX_STANDARD           20 )
set( CMAKE_CXX_STANDARD_REQUIRED  YES )
set( CMAKE_SKIP_INSTALL_RULES     TRUE )

list( APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake" )

include( DisableInSourceBuild )
DisableInSourceBuild()

project( EWAN VERSION 0.1 LANGUAGES CXX )

set( ${PROJECT_NAME}_ROOTDIR "${CMAKE_CURRENT_LIST_DIR}" )

##
## Configuration target
##

file( GLOB_RECURSE ${PROJECT_NAME}_CMAKE_FILES  LIST_DIRECTORIES false CONFIGURE_DEPENDS "CMake/*" )
file( GLOB_RECURSE ${PROJECT_NAME}_GITHUB_FILES LIST_DIRECTORIES false CONFIGURE_DEPENDS ".github/*" )
add_custom_target( ${PROJECT_NAME}.Configuration
    SOURCES
        ${${PROJECT_NAME}_CMAKE_FILES}
        ${${PROJECT_NAME}_GITHUB_FILES}
        .editorconfig
        .gitattributes
        .gitignore
        .gitmodules
        Source/.clang-format
)

set_target_properties( ${PROJECT_NAME}.Configuration PROPERTIES FOLDER "${PROJECT_NAME}" )

source_group( "CMake"             REGULAR_EXPRESSION "^${CMAKE_CURRENT_LIST_DIR}/CMake/" )
source_group( "CMake\\Toolchain"  REGULAR_EXPRESSION "^${CMAKE_CURRENT_LIST_DIR}/CMake/Toolchain/" )
source_group( "Git"               REGULAR_EXPRESSION "^${CMAKE_CURRENT_LIST_DIR}/.git" )
source_group( "GitHub"            REGULAR_EXPRESSION "^${CMAKE_CURRENT_LIST_DIR}/.github/" )
source_group( "GitHub\\Workflows" REGULAR_EXPRESSION "^${CMAKE_CURRENT_LIST_DIR}/.github/workflows/" )

##
## Code
##

add_subdirectory( Libs )
add_subdirectory( Embed )
add_subdirectory( Source )
add_subdirectory( Test )

##
## Prettify IDE
##

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake" )
