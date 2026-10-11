cmake_minimum_required(VERSION 3.25)
project(sudharshanmart VERSION 1.1.0 LANGUAGES CXX)

# Require C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for clang-tidy and LSP tools
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler Warnings & Standards
if(MSVC)
    add_compile_options(/W4 /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wconversion -Wnon-virtual-dtor)
endif()

# Optional Sanitizers in Debug / CI builds
option(ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(ENABLE_ASAN AND NOT MSVC)
    add_compile_options(-fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address -fsanitize=undefined)
endif()

# Find external dependencies via vcpkg / CMake config packages
find_package(Drogon CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(unofficial-sodium CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(CURL CONFIG REQUIRED)
find_package(unofficial-sqlite3 CONFIG REQUIRED)
find_package(PostgreSQL REQUIRED)

# Project Include Directories
include_directories(
    ${PROJECT_SOURCE_DIR}/src
)

# Source files for the main backend executable
file(GLOB_RECURSE SRC_FILES
    "src/*.cc"
    "src/*.cpp"
)

# Exclude any test sources if present in src
list(FILTER SRC_FILES EXCLUDE REGEX "test/.*")

# Main Executable: sudharshanmart
add_executable(sudharshanmart ${SRC_FILES})

target_link_libraries(sudharshanmart PRIVATE
    Drogon::Drogon
    nlohmann_json::nlohmann_json
    unofficial-sodium::sodium
    spdlog::spdlog
    unofficial::sqlite3::sqlite3
    CURL::libcurl
    PostgreSQL::PostgreSQL
)

# Compile definitions
target_compile_definitions(sudharshanmart PRIVATE
    SUDHARSHANMART_VERSION="${PROJECT_VERSION}"
    STATIC_PATH="${PROJECT_SOURCE_DIR}/public"
)

# Static Analysis target: clang-tidy & cppcheck
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(DO_CLANG_TIDY "${CLANG_TIDY_EXE};-warnings-as-errors=*")
endif()

find_program(CPPCHECK_EXE NAMES "cppcheck")
if(CPPCHECK_EXE)
    add_custom_target(cppcheck
        COMMAND ${CPPCHECK_EXE} --enable=all --inconclusive --std=c++20
                --suppress=missingIncludeSystem ${PROJECT_SOURCE_DIR}/src
    )
endif()

# Testing Configuration
enable_testing()
add_subdirectory(test)
