cmake_minimum_required(VERSION 3.16)
project(redtops VERSION 0.1 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.4.0 # Use a specific tag for stability
)
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2 # Use a specific tag for stability
)
FetchContent_GetProperties(nlohmann_json)
if(NOT nlohmann_json_POPULATED)
    FetchContent_Populate(nlohmann_json)
    add_library(nlohmann_json INTERFACE)
    target_include_directories(nlohmann_json INTERFACE ${nlohmann_json_SOURCE_DIR}/include)
endif()

enable_testing() # Enable testing for CTest

# Try to find PCAP using pkg-config first for better robustness
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(PCAP_PKG libpcap QUIET)
endif()

if (PCAP_PKG_FOUND)
    message(STATUS "Found libpcap (via PkgConfig): ${PCAP_PKG_LIBRARIES}")
    set(PCAP_INCLUDE_DIRS ${PCAP_PKG_INCLUDE_DIRS})
    set(PCAP_LIBRARIES ${PCAP_PKG_LIBRARIES})
else()
    # Fallback to CMake's FindPCAP module
    find_package(PCAP REQUIRED)
    message(STATUS "Found libpcap (via FindPCAP): ${PCAP_LIBRARIES}")
endif()

# Use C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folders for IDEs (optional but nice)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directory (bin/)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Include header directory
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PCAP_INCLUDE_DIRS})

# Recursively collect all source files
file(GLOB_RECURSE REDTOPS_SOURCES
    LIST_DIRECTORIES false
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Filter out anything in the Depreciated folder
list(FILTER REDTOPS_SOURCES EXCLUDE REGEX "Depreciated")

# Create executable
add_executable(redtops ${REDTOPS_SOURCES})
target_link_libraries(redtops PRIVATE ${PCAP_LIBRARIES})
target_link_libraries(redtops PRIVATE nlohmann_json)

# --- PLATFORM NETWORKING FIX ---
if (WIN32)
  message(STATUS "Configuring networking for Windows...")
  target_compile_definitions(redtops PRIVATE _WIN32_WINNT=0x0600)
  target_link_libraries(redtops PRIVATE iphlpapi ws2_32)
elseif(UNIX AND NOT APPLE)
  message(STATUS "Configuring networking for Linux...")
  target_compile_definitions(redtops PRIVATE _GNU_SOURCE)
elseif(APPLE)
  message(STATUS "Configuring networking for macOS...")
endif()
# --------------------------------

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

add_subdirectory(tests)
