function( EmbedDirectory directory ident )
    file( GLOB_RECURSE found_files LIST_DIRECTORIES false RELATIVE "${directory}" CONFIGURE_DEPENDS "*.*" )

    list( REMOVE_ITEM found_files "CMakeLists.txt" )
    list( REMOVE_ITEM found_files "README.md" )

    foreach( filename IN LISTS found_files )
        string( REGEX REPLACE "/" "__" var "${filename}")
        string( MAKE_C_IDENTIFIER "${var}" var )

        get_filename_component( dir "${filename}" DIRECTORY )
        get_filename_component( name "${filename}" NAME )
        string( TOLOWER "${name}" name )

        file( READ "${directory}/${filename}" bytes HEX )
        file( SIZE "${directory}/${filename}" bytes_size )

        string( REGEX MATCHALL ".." array "${bytes}")
        string( REGEX REPLACE ";" ", 0x" array "${array}")

        set_property( GLOBAL APPEND_STRING PROPERTY EMBED_DATA "        static const std::uint8_t ${var}[] = { 0x${array} };\n" )
        set_property( GLOBAL APPEND_STRING PROPERTY EMBED_INIT "        Log::Raw(\" *embed/${ident}/${name}\");\n" )
        set_property( GLOBAL APPEND_STRING PROPERTY EMBED_INIT "        content.${dir}.New(\"*embed/${ident}/${name}\")->loadFromMemory(&Embed::${var}, ${bytes_size});\n" )
    endforeach()

endfunction()

function( EmbedWrite filename )
    get_property( EMBED_DATA GLOBAL PROPERTY EMBED_DATA )
    string( LENGTH "${EMBED_DATA}" len )

    if( len EQUAL 0 )
        file( REMOVE "${filename}" )
        return()
    endif()

    get_property( EMBED_INIT GLOBAL PROPERTY EMBED_INIT )

    string( REGEX REPLACE "\n$" "" EMBED_DATA "${EMBED_DATA}" )
    string( REGEX REPLACE "\n$" "" EMBED_INIT "${EMBED_INIT}" )

    configure_file( "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/Embed.hpp.in" "${filename}" @ONLY NEWLINE_STYLE LF )
endfunction()
