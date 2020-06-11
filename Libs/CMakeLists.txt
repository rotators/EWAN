cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( CompileTarget )

set( BUILD_SHARED_LIBS FALSE )

##
## SFML
##

add_subdirectory( SFML EXCLUDE_FROM_ALL )
foreach( name IN ITEMS activity audio graphics main network system window)
	if( TARGET sfml-${name} )
		other_target( sfml-${name} )
	endif()
endforeach()
