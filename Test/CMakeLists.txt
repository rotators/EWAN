cmake_minimum_required( VERSION 3.17 FATAL_ERROR )
###( CMAKE_VERBOSE_MAKEFILE TRUE )

##
## Get tests sources and generate test driver
##
file( GLOB_RECURSE test_files LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "*.cpp" )
create_test_sourcelist( test_sources ${PROJECT_NAME}.Test.cpp ${test_files} )

##
## Add extra compiler flags for selected files
##
if(CMAKE_COMPILER_IS_GNUCXX)
    compiler_source_flag( ${PROJECT_NAME}.Test.cpp     -Wno-useless-cast       COMPILER_FLAG_WNO_USELESS_CAST )
endif()

##
## Use test sources filenames to create more friendly tests names
##
foreach( cpp IN LISTS test_files )
    string( REGEX REPLACE "\\.[Cc][Pp][Pp]$" "" test "${cpp}" )
    list( APPEND test_list "${test}" )
    string( MAKE_C_IDENTIFIER  "${test}" test )
    set_source_files_properties( ${cpp} PROPERTIES COMPILE_DEFINITIONS TEST_NAME=${test} )
endforeach()

##
## Test app setup
##
add_executable( ${PROJECT_NAME}.CoreTest EXCLUDE_FROM_ALL )
target_sources( ${PROJECT_NAME}.CoreTest
    PRIVATE
        ${test_sources}
        ${engine_sources}
)
target_include_directories( ${PROJECT_NAME}.CoreTest
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${engine_dir}"
        "${PROJECT_BINARY_DIR}/Embed"
)

target_link_libraries( ${PROJECT_NAME}.CoreTest PRIVATE ${PROJECT_NAME}.Core )
project_target( ${PROJECT_NAME}.CoreTest )
set_target_properties( ${PROJECT_NAME}.CoreTest PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD TRUE )

##
## On Windows, copy OpenAL library to build directory
##
if( TARGET OpenAL AND WIN32 )
    include(CheckTypeSize)
    check_type_size(void* ${PROJECT_NAME}_SIZEOF_VOID_P) 

    if( ${PROJECT_NAME}_SIZEOF_VOID_P EQUAL 4 )
        set( openal_dir x86 )
    elseif( ${PROJECT_NAME}_SIZEOF_VOID_P EQUAL 8 )
        set( openal_dir x64 )
    else()
        message(FATAL_ERROR "${PROJECT_NAME}_SIZEOF_VOID_P = '${${PROJECT_NAME}_SIZEOF_VOID_P}'")
    endif()

    message(STATUS "Check size of void* - ${${PROJECT_NAME}_SIZEOF_VOID_P}")

    add_custom_command( TARGET ${PROJECT_NAME}.CoreTest
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/Libs/SFML/extlibs/bin/${openal_dir}/openal32.dll ${CMAKE_CURRENT_BINARY_DIR}/OpenAL32.dll
    )
endif()

##
## Run tests right after building test app, for simplicity
##
add_custom_command( TARGET ${PROJECT_NAME}.CoreTest
    POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

enable_testing()
foreach( test IN LISTS test_list )
    add_test( NAME ${test} COMMAND ${PROJECT_NAME}.CoreTest ${test} )
endforeach()

##
## Prettify IDE
##
set_target_properties( ${PROJECT_NAME}.CoreTest PROPERTIES FOLDER "${PROJECT_NAME}" )

source_group( " "     REGULAR_EXPRESSION "\.[CcHh][Pp][Pp]$" )
source_group( "CMake" REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
source_group( "Libs"  REGULAR_EXPRESSION "(Libs/.+\.[CcHh][Pp][Pp]$|/nlohmann_json\.natvis$)" )
