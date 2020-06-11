cmake_minimum_required( VERSION 3.17 FATAL_ERROR )

include( Embed )
EmbedDirectory( "${CMAKE_CURRENT_LIST_DIR}" "iii" )
EmbedWrite( "${CMAKE_CURRENT_BINARY_DIR}/Embed.hpp" )
