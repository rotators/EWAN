cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( CheckCXXSymbolExists )
include( CheckIncludeFileCXX )

include( CompileTarget )

check_cxx_symbol_exists( __has_include string ${PROJECT_NAME}_SYMBOL_HAS_INCLUDE )
if( NOT ${PROJECT_NAME}_SYMBOL_HAS_INCLUDE )
    message( FATAL_ERROR "Missing symbol __has_include" )
endif()

check_cxx_symbol_exists( __func__ string ${PROJECT_NAME}_SYMBOL_FUNC )
if( NOT ${PROJECT_NAME}_SYMBOL_FUNC )
    message( FATAL_ERROR "Missing __func__" )
endif()

##

check_include_file_cxx( filesystem                   ${PROJECT_NAME}_INCLUDE_FILESYSTEM )
check_include_file_cxx( format                       ${PROJECT_NAME}_INCLUDE_FORMAT )
check_include_file_cxx( source_location              ${PROJECT_NAME}_INCLUDE_SOURCE_LOCATION )
check_include_file_cxx( experimental/source_location ${PROJECT_NAME}_INCLUDE_EXPERIMENTAL_SOURCE_LOCATION )

##

add_executable( ${PROJECT_NAME} WIN32 )
target_sources( ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_FILE}
        Main.cpp

        App.cpp
        App.hpp
        Content.hpp
        Content.cpp
        GameInfo.cpp
        GameInfo.hpp
        Generator.cpp
        Generator.hpp
        Log.cpp
        Log.hpp
        Script.cpp
        Script.hpp
        Script.API.cpp
        Script.Builder.cpp
        Script.Callback.cpp
        Script.Event.cpp
        Text.cpp
        Text.hpp
        Utils.cpp
        Utils.hpp
        Window.cpp
        Window.hpp

        Libs/AngelScript.hpp
        Libs/JSON.cpp
        Libs/JSON.hpp
        Libs/SFML.hpp
)
target_include_directories( ${PROJECT_NAME}
    PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}"
        "${PROJECT_BINARY_DIR}/Embed"
)

if(CMAKE_COMPILER_IS_GNUCXX)
    compiler_source_flag(Script.API.cpp -Wno-cast-function-type COMPILER_FLAG_WNO_CAST_FUNCTION_TYPE)
endif()

foreach( ver IN ITEMS MAJOR MINOR PATCH TWEAK )
    if( PROJECT_VERSION_${ver} )
        target_compile_definitions( ${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_VERSION_${ver}=${PROJECT_VERSION_${ver}} )
    else()
        target_compile_definitions( ${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_VERSION_${ver}=0 )
    endif()
endforeach()


project_target_libraries( ${PROJECT_NAME} )
project_target( ${PROJECT_NAME} )
