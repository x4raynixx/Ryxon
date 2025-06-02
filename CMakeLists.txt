cmake_minimum_required(VERSION 3.10)
project(RXLanguage)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_installARY_DIR}/install)

include_directories(src)

set(SOURCES
    src/main.cpp
    src/Token.cpp
    src/Tokenizer.cpp
    src/AST.cpp
    src/Parser.cpp
    src/Interpreter.cpp
)

add_executable(rx ${SOURCES})

if(WIN32)
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx.exe")
elseif(APPLE)
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx_macos")
else()
    set_target_properties(rx PROPERTIES OUTPUT_NAME "rx_linux")
endif()

if(MSVC)
    target_compile_options(rx PRIVATE /W4)
else()
    target_compile_options(rx PRIVATE -Wall -Wextra -pedantic)
endif()

install(TARGETS rx DESTINATION bin)

add_custom_target(build_all
    COMMENT "Building for all platforms"
)