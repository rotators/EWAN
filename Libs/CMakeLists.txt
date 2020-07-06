cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( CompileTarget )

set( BUILD_SHARED_LIBS FALSE )

##
## AngelScript
##

add_subdirectory( AngelScript/sdk/angelscript/projects/cmake EXCLUDE_FROM_ALL )
target_compile_definitions( ${ANGELSCRIPT_LIBRARY_NAME} PUBLIC AS_USE_NAMESPACE )
other_target( ${ANGELSCRIPT_LIBRARY_NAME} )
if( CMAKE_COMPILER_IS_GNUCXX )
    # https://www.angelcode.com/angelscript/sdk/docs/manual/doc_compile_lib.html#doc_compile_gnuc
    compiler_flag( "compile" angelscript -fno-strict-aliasing COMPILER_FLAG_FNO_STRICT_ALIASING )
endif()

##
## AngelScript addons
##

add_library( ${ANGELSCRIPT_LIBRARY_NAME}-addons STATIC )
target_sources( ${ANGELSCRIPT_LIBRARY_NAME}-addons
    PRIVATE
        AngelScript/sdk/add_on/scriptarray/scriptarray.cpp
        AngelScript/sdk/add_on/scriptarray/scriptarray.h
        AngelScript/sdk/add_on/scriptbuilder/scriptbuilder.cpp
        AngelScript/sdk/add_on/scriptbuilder/scriptbuilder.h
        AngelScript/sdk/add_on/scriptstdstring/scriptstdstring.cpp
        AngelScript/sdk/add_on/scriptstdstring/scriptstdstring.h
        AngelScript/sdk/add_on/scriptstdstring/scriptstdstring_utils.cpp
)
target_include_directories( ${ANGELSCRIPT_LIBRARY_NAME}-addons PRIVATE ${CMAKE_CURRENT_LIST_DIR}/AngelScript/sdk/angelscript/include )
target_link_libraries( ${ANGELSCRIPT_LIBRARY_NAME}-addons PRIVATE ${ANGELSCRIPT_LIBRARY_NAME} )
other_target( ${ANGELSCRIPT_LIBRARY_NAME}-addons )

##
## AngelScript tests
## Run tests right after building targets, for simplicity
##

add_subdirectory(AngelScript/sdk/tests/test_feature/projects/cmake EXCLUDE_FROM_ALL)
get_target_property( ANGELSCRIPT__TEST_FEATURE__SOURCES test_feature SOURCES )
get_target_property( ANGELSCRIPT__TEST_FEATURE__SOURCE_DIR test_feature SOURCE_DIR )
list( TRANSFORM ANGELSCRIPT__TEST_FEATURE__SOURCES REPLACE "^(.+)$" "${ANGELSCRIPT__TEST_FEATURE__SOURCE_DIR}/\\1" )
add_executable( angelscript-test-feature EXCLUDE_FROM_ALL )
target_sources( angelscript-test-feature PRIVATE ${ANGELSCRIPT__TEST_FEATURE__SOURCES} )
target_compile_definitions( angelscript-test-feature PRIVATE DONT_WAIT=1 )
target_link_libraries( angelscript-test-feature PRIVATE angelscript )
target_include_directories( angelscript-test-feature PRIVATE ${CMAKE_CURRENT_LIST_DIR}/AngelScript/sdk/angelscript/include )
other_target(angelscript-test-feature)
if(CMAKE_COMPILER_IS_GNUCXX)
    if( WIN32 )
        compiler_flag( "link" angelscript-test-feature -static           COMPILER_FLAG_STATIC )
    endif()
    compiler_flag( "link" angelscript-test-feature -static-libgcc    COMPILER_FLAG_STATIC_LIBGCC )
    compiler_flag( "link" angelscript-test-feature -static-libstdc++ COMPILER_FLAG_STATIC_LIBSTDCPP )
endif()

add_custom_command( TARGET angelscript-test-feature
    POST_BUILD
    COMMAND  $<TARGET_FILE:angelscript-test-feature>
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/AngelScript/sdk/tests/test_feature/bin/
)

##
## json
##

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory( json EXCLUDE_FROM_ALL )

##
## SFML
##

set(SFML_BUILD_TEST_SUITE ON)

add_subdirectory( SFML EXCLUDE_FROM_ALL )
foreach( name IN ITEMS activity audio graphics main network system window)
    if( TARGET sfml-${name} )
        other_target( sfml-${name} )
    endif()
endforeach()

####               ####
##                   ##
## Interface targets ##
##                   ##
####               ####

##
## {PROJECT_NAME}.AngelScript
##

add_library( ${PROJECT_NAME}.AngelScript INTERFACE )
target_include_directories( ${PROJECT_NAME}.AngelScript INTERFACE ${CMAKE_CURRENT_LIST_DIR}/AngelScript/sdk/angelscript/include )
target_include_directories( ${PROJECT_NAME}.AngelScript INTERFACE ${CMAKE_CURRENT_LIST_DIR}/AngelScript/sdk/add_on )
target_link_libraries( ${PROJECT_NAME}.AngelScript INTERFACE ${ANGELSCRIPT_LIBRARY_NAME} ${ANGELSCRIPT_LIBRARY_NAME}-addons )

##
## {PROJECT_NAME}.JSON
##

add_library( ${PROJECT_NAME}.JSON INTERFACE )
target_link_libraries( ${PROJECT_NAME}.JSON INTERFACE nlohmann_json::nlohmann_json )

##
## {PROJECT_NAME}.SFML
##

add_library( ${PROJECT_NAME}.SFML INTERFACE )
target_link_libraries( ${PROJECT_NAME}.SFML INTERFACE sfml-audio sfml-graphics sfml-system sfml-window) # sfml-network

foreach( lib IN ITEMS sfml-activity sfml-main )
    if( TARGET ${lib} )
        target_link_libraries( ${PROJECT_NAME}.SFML INTERFACE ${lib} )
    endif()
endforeach()

####                 ####
##                     ##
## {PROJECT_NAME}.Libs ##
##                     ##
####                 ####

add_library( ${PROJECT_NAME}.Libs INTERFACE )
target_link_libraries( ${PROJECT_NAME}.Libs INTERFACE ${PROJECT_NAME}.AngelScript )
target_link_libraries( ${PROJECT_NAME}.Libs INTERFACE ${PROJECT_NAME}.JSON )
target_link_libraries( ${PROJECT_NAME}.Libs INTERFACE ${PROJECT_NAME}.SFML )
