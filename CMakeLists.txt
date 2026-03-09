cmake_minimum_required(VERSION 3.25)

project(PWDoom
    VERSION 0.1.0
    DESCRIPTION "PWDoom - A Doom-inspired game built with raylib"
    LANGUAGES C
)

# Global settings
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-g3 -O0 -fsanitize=address,undefined)
    add_link_options(-fsanitize=address,undefined)
endif()

# Dependencies via FetchContent
include(FetchContent)

# raylib 5.5
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)
FetchContent_MakeAvailable(raylib)

# PWDoom executable
file(GLOB_RECURSE PWDOOM_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

add_executable(${PROJECT_NAME} ${PWDOOM_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

# Strict warnings only for our code, not dependencies
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror=implicit-function-declaration
    -Werror=return-type
    -Wshadow
    -Wdouble-promotion
    -Wformat=2
    -Wundef
    -Wconversion
    -Wsign-conversion
    -Wnull-dereference
    -Wstrict-prototypes
    -Wold-style-definition
    -Wmissing-prototypes
    -Wswitch-enum
    -Wunused
    -Wstack-protector
    -fstack-protector-strong
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

# Copy assets to build directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
        COMMENT "Copying assets to build directory..."
    )
endif()

# Install target
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# Static analysis targets
find_program(CLANG_TIDY_EXE NAMES clang-tidy
    HINTS /opt/homebrew/opt/llvm/bin)
find_program(CPPCHECK_EXE NAMES cppcheck)

if(CLANG_TIDY_EXE)
    add_custom_target(tidy
        COMMAND ${CLANG_TIDY_EXE}
            -p ${CMAKE_BINARY_DIR}
            --header-filter='${CMAKE_CURRENT_SOURCE_DIR}/src/.*'
            --system-headers=false
            ${PWDOOM_SOURCES}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running clang-tidy..."
    )
endif()

if(CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE}
            --enable=all
            --std=c99
            --suppress=missingInclude
            --suppress=missingIncludeSystem
            --suppress=unusedFunction
            --suppress=unmatchedSuppression
            --suppress=checkersReport
            --project=${CMAKE_BINARY_DIR}/compile_commands.json
            --file-filter=${CMAKE_CURRENT_SOURCE_DIR}/src/
            --error-exitcode=1
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running cppcheck..."
    )
endif()

message(STATUS "")
message(STATUS "╔══════════════════════════════════════════╗")
message(STATUS "║         PWDoom Build Configuration       ║")
message(STATUS "╠══════════════════════════════════════════╣")
message(STATUS "║  Version    : ${PROJECT_VERSION}")
message(STATUS "║  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "║  C Standard : C${CMAKE_C_STANDARD}")
message(STATUS "║  Compiler   : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "║  Generator  : ${CMAKE_GENERATOR}")
message(STATUS "╚══════════════════════════════════════════╝")
message(STATUS "")
