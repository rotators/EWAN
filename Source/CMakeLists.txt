cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( CheckCXXSymbolExists )
include( CheckIncludeFileCXX )

include( CompileTarget )

check_cxx_symbol_exists( __has_include string ${PROJECT_NAME}_SYMBOL_HAS_INCLUDE )
if( NOT ${PROJECT_NAME}_SYMBOL_HAS_INCLUDE )
    message( FATAL_ERROR "Missing __has_include" )
endif()

check_cxx_symbol_exists( __func__ string ${PROJECT_NAME}_SYMBOL_FUNC )
if( NOT ${PROJECT_NAME}_SYMBOL_FUNC )
    message( FATAL_ERROR "Missing __func__" )
endif()

##

foreach( include IN ITEMS filesystem format source_location )
    check_include_file_cxx( ${include}                  ${PROJECT_NAME}_INCLUDE_${include} )
    if( NOT ${PROJECT_NAME}_INCLUDE_${include} )
        check_include_file_cxx( experimental/${include} ${PROJECT_NAME}_INCLUDE_EXPERIMENTAL_${include} )
    endif()
endforeach()

##

set( CMAKE_EXPORT_COMPILE_COMMANDS TRUE )

add_executable( ${PROJECT_NAME} WIN32 )
add_library( ${PROJECT_NAME}.Core STATIC )

target_sources( ${PROJECT_NAME}
    PRIVATE
        ${CMAKE_CURRENT_LIST_FILE}
        Main.cpp
)

target_link_libraries( ${PROJECT_NAME} PRIVATE ${PROJECT_NAME}.Core )
project_target( ${PROJECT_NAME} )

##

target_sources( ${PROJECT_NAME}.Core
    PRIVATE
        ${CMAKE_CURRENT_LIST_FILE}

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
        Script.UserData.cpp
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
target_include_directories( ${PROJECT_NAME}.Core
    PUBLIC
        "${CMAKE_CURRENT_LIST_DIR}"
        "${PROJECT_BINARY_DIR}/Embed"
)

foreach( ver IN ITEMS MAJOR MINOR PATCH TWEAK )
    if( PROJECT_VERSION_${ver} )
        set_property( SOURCE App.cpp APPEND PROPERTY COMPILE_DEFINITIONS PROJECT_VERSION_${ver}=${PROJECT_VERSION_${ver}} )
    else()
        set_property( SOURCE App.cpp APPEND PROPERTY COMPILE_DEFINITIONS PROJECT_VERSION_${ver}=0 )
    endif()
endforeach()

target_link_libraries( ${PROJECT_NAME}.Core PUBLIC ${PROJECT_NAME}.Libs )
project_target( ${PROJECT_NAME}.Core )

# Must be done after project_target()
if(CMAKE_COMPILER_IS_GNUCXX)
    compiler_source_flag( Script.API.cpp -Wno-cast-function-type COMPILER_FLAG_WNO_CAST_FUNCTION_TYPE )
    compiler_source_flag( Script.API.cpp -Wno-useless-cast       COMPILER_FLAG_WNO_USELESS_CAST )
endif()

## Prettify IDE

set_target_properties( ${PROJECT_NAME}      PROPERTIES FOLDER "${PROJECT_NAME}" )
set_target_properties( ${PROJECT_NAME}.Core PROPERTIES FOLDER "${PROJECT_NAME}" )

source_group( " "     REGULAR_EXPRESSION "\.[CcHh][Pp][Pp]$" )
source_group( "CMake" REGULAR_EXPRESSION "[Cc][Mm][Aa][Kk][Ee]" )
source_group( "Libs"  REGULAR_EXPRESSION "(Libs/.+\.[CcHh][Pp][Pp]$|/nlohmann_json\.natvis$)" )
