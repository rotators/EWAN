function( compiler_flag type target flag var )
    include( CheckCXXCompilerFlag )
    check_cxx_compiler_flag( ${flag} ${var} )

    if( ${var} )
        if( "${type}" STREQUAL "compile" )
            target_compile_options( ${target} PRIVATE ${flag} )
        elseif( "${type}" STREQUAL "link" )
            target_link_options( ${target} PRIVATE ${flag} )
        else()
            message( FATAL_ERROR "Unknown flag type '${type}'" )
        endif()
    endif()
endfunction()

function( compiler_source_flag file flag var )
    include( CheckCXXCompilerFlag )
    check_cxx_compiler_flag( ${flag} ${var} )

    if( ${var} )
        # set_source_files_properties(${file} PROPERTIES COMPILE_OPTIONS "${flag}")
        set_property( SOURCE ${file} APPEND PROPERTY COMPILE_OPTIONS "${flag}" )
    endif()
endfunction()

function( other_target target )
    if( CMAKE_COMPILER_IS_GNUCXX )
        #[[
        include( CheckCXXCompilerFlag )
        check_cxx_compiler_flag( -fdata-sections     COMPILER_FLAG_FDATA_SECTIONS )
        if( COMPILER_FLAG_FDATA_SECTIONS )
            check_cxx_compiler_flag( -ffunction-sections COMPILER_FLAG_FFUNCTION_SECTIONS )
            if( COMPILER_FLAG_FFUNCTION_SECTIONS )
                check_cxx_compiler_flag( -Wl,--gc-sections   COMPILER_FLAG_GC_SECTIONS )
                if( COMPILER_FLAG_GC_SECTIONS )
                    target_compile_options( ${target} PRIVATE -fdata-sections -ffunction-sections )
                    target_link_options( ${target}  PRIVATE -Wl,--gc-sections )
                endif()
            endif()
        endif()
        ]]
    elseif( MSVC )
        set_property( TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /MT )
        set_property( TARGET ${target} APPEND PROPERTY COMPILE_OPTIONS /Z7 )
        set_property( TARGET ${target} APPEND PROPERTY LINK_OPTIONS    /OPT:REF )
        set_property( TARGET ${target} APPEND PROPERTY LINK_OPTIONS    /DEBUG )
        set_property( TARGET ${target} APPEND PROPERTY LINK_OPTIONS    /LARGEADDRESSAWARE )
    endif()
endfunction()

function( project_target target )
    other_target( ${target} )

    if( CMAKE_COMPILER_IS_GNUCXX )
        compiler_flag( "compile" ${target} -Wall      COMPILER_FLAG_WALL )
        compiler_flag( "compile" ${target} -Werror    COMPILER_FLAG_WERROR )
        compiler_flag( "compile" ${target} -Wextra    COMPILER_FLAG_WEXTRA )
        compiler_flag( "compile" ${target} -Wpedantic COMPILER_FLAG_WPEDANTIC )

        compiler_flag( "compile" ${target} -Wconversion          COMPILER_FLAG_WCONVERSION )
        compiler_flag( "compile" ${target} -Wduplicated-branches COMPILER_FLAG_WDUPLICATED_BRANCHES )
        compiler_flag( "compile" ${target} -Wduplicated-cond     COMPILER_FLAG_WDUPLICATED_COND )
        compiler_flag( "compile" ${target} -Wlogical-op          COMPILER_FLAG_WLOGICAL_OP )
        compiler_flag( "compile" ${target} -Wnull-dereference    COMPILER_FLAG_WNULL_DEREFERENCE )
        compiler_flag( "compile" ${target} -Wuseless-cast        COMPILER_FLAG_WUSELESS_CAST )
#       compiler_flag( "compile" ${target} -Wpadded              COMPILER_FLAG_WPADDED )
        compiler_flag( "compile" ${target} -Wshadow              COMPILER_FLAG_WSHADOW )

        compiler_flag( "compile" ${target} -ggdb3                COMPILER_FLAG_GGDB3 )
        if(NOT COMPILER_FLAG_GGDB3 )
            compiler_flag( "compile" ${target} -ggdb             COMPILER_FLAG_GGDB )
            if(NOT COMPILER_FLAG_GGDB )
                compiler_flag( "compile" ${target} -g            COMPILER_FLAG_G )
            endif()
        endif()

        if( WIN32 )
            compiler_flag( "link" ${target} -static           COMPILER_FLAG_STATIC )
        endif()
        compiler_flag( "link" ${target} -static-libgcc    COMPILER_FLAG_STATIC_LIBGCC )
        compiler_flag( "link" ${target} -static-libstdc++ COMPILER_FLAG_STATIC_LIBSTDCPP )
    elseif( MSVC )
    endif()
endfunction()
