cmake_minimum_required(VERSION 3.16)
project(redtops VERSION 0.1 LANGUAGES CXX)

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDEs (optional but nice)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directory (bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include header directory
include_directories(${PROJECT_SOURCE_DIR}/include)

# Recursively collect all source files
file(GLOB_RECURSE REDTOPS_SOURCES
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(redtops ${REDTOPS_SOURCES})

# If you ever want to link libraries later (asio, ncurses, etc), they will go here:
# target_link_libraries(redtops PRIVATE some_library)

# Copy assets to build folder automatically
add_custom_command(TARGET redtops POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets
)

# Copy configs as well
add_custom_command(TARGET redtops POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/configs
        ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/configs
)

message(STATUS "REDTOPS build configured successfully.")
