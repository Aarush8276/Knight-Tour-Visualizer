cmake_minimum_required(VERSION 3.15)
project(KnightTourVisualizer VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Fetch SFML
include(FetchContent)
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.1
)

# SFML configuration options before FetchContent_MakeAvailable
set(SFML_BUILD_GRAPHICS ON CACHE BOOL "" FORCE)
set(SFML_BUILD_WINDOW ON CACHE BOOL "" FORCE)
set(SFML_BUILD_SYSTEM ON CACHE BOOL "" FORCE)
set(SFML_BUILD_AUDIO ON CACHE BOOL "" FORCE)
set(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
set(SFML_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SFML)

# Add our source and header files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.h")

add_executable(KnightTourVisualizer ${SOURCES} ${HEADERS})

target_include_directories(KnightTourVisualizer PRIVATE include)

# Disable incremental linking for Windows (fixes OneDrive permission issues)
if(MSVC)
    set_target_properties(KnightTourVisualizer PROPERTIES LINK_FLAGS "/INCREMENTAL:NO")
endif()

# Link SFML libraries
target_link_libraries(KnightTourVisualizer PRIVATE sfml-graphics sfml-audio sfml-window sfml-system)

# Copy assets folder structure to output directory for convenience
add_custom_command(TARGET KnightTourVisualizer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_CURRENT_SOURCE_DIR}/assets"
    "${CMAKE_BINARY_DIR}/bin/assets"
    COMMENT "Copying assets to bin directory..."
)
