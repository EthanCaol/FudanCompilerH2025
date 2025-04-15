aux_source_directory(. AST_SRCS)
add_library(ast STATIC ${AST_SRCS})
target_include_directories(ast PUBLIC .)