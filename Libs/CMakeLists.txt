cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( CompileTarget )

set( BUILD_SHARED_LIBS FALSE )

function( project_target_libraries target )
    target_link_libraries( ${target} PRIVATE nlohmann_json::nlohmann_json )
    target_link_libraries( ${target} PRIVATE sfml-audio sfml-graphics sfml-system sfml-window) # sfml-network
    foreach( lib IN ITEMS sfml-activity sfml-main )
        if( TARGET ${lib} )
            target_link_libraries( ${target} PRIVATE ${lib} )
        endif()
    endforeach()
endfunction()

##
## json
##

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory( json EXCLUDE_FROM_ALL )

##
## SFML
##

add_subdirectory( SFML EXCLUDE_FROM_ALL )
foreach( name IN ITEMS activity audio graphics main network system window)
    if( TARGET sfml-${name} )
        other_target( sfml-${name} )
    endif()
endforeach()
