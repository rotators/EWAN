cmake_minimum_required( VERSION 3.17 FATAL_ERROR )
###( CMAKE_VERBOSE_MAKEFILE TRUE )

##
## Get tests sources and generate test driver
##
file( GLOB_RECURSE test_files LIST_DIRECTORIES false RELATIVE "${CMAKE_CURRENT_LIST_DIR}" CONFIGURE_DEPENDS "*.cpp" )
create_test_sourcelist( test_sources ${PROJECT_NAME}.Test.cpp ${test_files} )

##
## Include engine's sources in test app, except entry point code
##
get_target_property( engine_sources ${PROJECT_NAME} SOURCES )
get_target_property( engine_dir ${PROJECT_NAME} SOURCE_DIR )
list( FILTER engine_sources EXCLUDE REGEX "^Main\.cpp$" )
list( FILTER engine_sources EXCLUDE REGEX "/CMakeLists\.txt$" )
list( TRANSFORM engine_sources REPLACE "^(.+)$" "${engine_dir}/\\1" )

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
add_executable( ${PROJECT_NAME}.Test )
target_sources( ${PROJECT_NAME}.Test
    PRIVATE
        ${test_sources}
        ${engine_sources}
)

target_include_directories( ${PROJECT_NAME}.Test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${engine_dir}
)
target_link_libraries( ${PROJECT_NAME}.Test PRIVATE sfml-audio sfml-graphics sfml-system sfml-window )
if( TARGET sfml-main )
	target_link_libraries( ${PROJECT_NAME}.Test PRIVATE sfml-main )
endif()
add_dependencies( ${PROJECT_NAME}.Test ${PROJECT_NAME} )

project_target( ${PROJECT_NAME}.Test )

##
## On Windows, copy OpenAL library to build directory
##
if( TARGET OpenAL AND WIN32 )
    if( CMAKE_SIZEOF_VOID_P EQUAL 4 )
        set( openal_dir x86 )
    elseif( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        set( openal_dir x64 )
    endif()

    add_custom_command( TARGET ${PROJECT_NAME}.Test
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PROJECT_SOURCE_DIR}/Libs/SFML/extlibs/bin/${openal_dir}/openal32.dll ${CMAKE_CURRENT_BINARY_DIR}/OpenAL32.dll
    )
endif()

##
## Run tests right after building test app, for simplicity
##
add_custom_command( TARGET ${PROJECT_NAME}.Test
    POST_BUILD
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure -C Release
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

enable_testing()
foreach( test IN LISTS test_list )
    add_test( NAME ${test} COMMAND ${PROJECT_NAME}.Test ${test} )
endforeach()
