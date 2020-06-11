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
		Generator.cpp
		Generator.hpp
		Log.cpp
		Log.hpp
		SFML.hpp
		Text.cpp
		Text.hpp
		Window.cpp
		Window.hpp
)

foreach( ver IN ITEMS MAJOR MINOR PATCH TWEAK )
	if( PROJECT_VERSION_${ver} )
		target_compile_definitions( ${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_VERSION_${ver}=${PROJECT_VERSION_${ver}} )
	else()
		target_compile_definitions( ${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_VERSION_${ver}=0 )
	endif()
endforeach()

target_include_directories( ${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_LIST_DIR}" "${PROJECT_BINARY_DIR}/Embed" )
target_link_libraries( ${PROJECT_NAME} PRIVATE sfml-audio sfml-graphics sfml-system sfml-window )
if( TARGET sfml-main )
	target_link_libraries( ${PROJECT_NAME} PRIVATE sfml-main )
endif()

project_target( ${PROJECT_NAME} )
