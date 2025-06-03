cmake_minimum_required(VERSION 3.10)
project(RXLanguage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/install)

include_directories(src)

# Collect all source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

# Remove any test files if they exist
list(FILTER SOURCES EXCLUDE REGEX ".*test.*")

add_executable(rx ${SOURCES})

if(WIN32)
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx.exe")
    target_link_libraries(rx -static -static-libgcc -static-libstdc++)
elseif(APPLE)
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx_macos")
else()
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx_linux")
endif()

if(MSVC)
    target_compile_options(rx PRIVATE /W4 /O2)
else()
    target_compile_options(rx PRIVATE -Wall -Wextra -pedantic -O3 -march=native)
endif()

install(TARGETS rx DESTINATION bin)

# Custom target for building all platforms
add_custom_target(build_all
    COMMENT "Building for all platforms"
)

# Print build information
message(STATUS "Building RX Language Interpreter")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
