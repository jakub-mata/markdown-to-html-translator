cmake_minimum_required(VERSION 3.10)
project(markdown_converter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories for header files
include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/parsing
    ${PROJECT_SOURCE_DIR}/parsing_tree
    ${PROJECT_SOURCE_DIR}/building
)

# Add source files
set(SOURCES
    main.cpp
)

# Add header files
set(HEADERS
    parsing/markdown_parser.hpp
    parsing/state.hpp
    parsing_tree/tree_builder.hpp
    building/html_constructor.hpp
    building/css_constructor.hpp
    token.hpp
    node.hpp
)

# Create executable
add_executable(markdown_converter ${SOURCES} ${HEADERS})
