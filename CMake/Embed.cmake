function( EmbedDirectory directory ident )
    file( GLOB_RECURSE found_files LIST_DIRECTORIES false RELATIVE "${directory}" CONFIGURE_DEPENDS "*" )

    list( REMOVE_ITEM found_files "CMakeLists.txt" )
    list( REMOVE_ITEM found_files "README.md" )

    foreach( filename IN LISTS found_files )
        get_filename_component( dir "${filename}" DIRECTORY )
        get_filename_component( name "${filename}" NAME )

        if( dir MATCHES "(.+)/(.+)" )
            set( dir "${CMAKE_MATCH_1}" )
            set( name "${CMAKE_MATCH_2}/${name}" )
        endif()
        string( REGEX REPLACE "/" "__" var "${name}")
        string( MAKE_C_IDENTIFIER "${var}" var )

        file( READ "${directory}/${filename}" bytes HEX )
        file( SIZE "${directory}/${filename}" bytes_size )

        string( REGEX MATCHALL ".." array "${bytes}")
        string( REGEX REPLACE ";" ", 0x" array "${array}")

        if( NOT "${dir}" STREQUAL "" )
            set_property( GLOBAL APPEND PROPERTY EMBED_LIST "${dir}" )
            set_property( GLOBAL APPEND_STRING PROPERTY EMBED_DATA_${dir} "            static const std::uint8_t ${var}[] = { 0x${array} };\n" )
            set_property( GLOBAL APPEND_STRING PROPERTY EMBED_DATA_${dir} "            static const std::size_t  ${var}_l = ${bytes_size};\n" )
        else()
            set_property( GLOBAL APPEND_STRING PROPERTY EMBED_DATA "        static const std::uint8_t ${var}[] = { 0x${array} };\n" )
            set_property( GLOBAL APPEND_STRING PROPERTY EMBED_DATA "        static const std::size_t  ${var}_l = ${bytes_size};\n" )
        endif()
    endforeach()
endfunction()

function( EmbedWrite filename )
    get_property( EMBED_LIST GLOBAL PROPERTY EMBED_LIST )
    get_property( EMBED_DATA GLOBAL PROPERTY EMBED_DATA )
    string( LENGTH "${EMBED_LIST}" lenList )
    string( LENGTH "${EMBED_DATA}" lenData )

    if( lenList EQUAL 0 AND lenData EQUAL 0 )
        file( REMOVE "${filename}" )
        return()
    endif()

    unset( EMBED_DATA )

    get_property( EMBED_LIST GLOBAL PROPERTY EMBED_LIST )
    foreach( name IN LISTS EMBED_LIST )
        string( APPEND EMBED_DATA "        namespace ${name}\n" )
        string( APPEND EMBED_DATA "        {\n"  )
        get_property( data GLOBAL PROPERTY EMBED_DATA_${name} )
        string( APPEND EMBED_DATA "${data}" )
        string( APPEND EMBED_DATA "        }\n\n" )
    endforeach()

    get_property( root_data GLOBAL PROPERTY EMBED_DATA )
    string( APPEND EMBED_DATA "${root_data}" )

    string( REGEX REPLACE "[\n]+$" "" EMBED_DATA "${EMBED_DATA}" )

    configure_file( "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Embed.hpp.in" "${filename}" @ONLY NEWLINE_STYLE LF )
endfunction()
