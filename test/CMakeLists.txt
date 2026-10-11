find_package(GTest REQUIRED)
include(GoogleTest)

file(GLOB_RECURSE TEST_SRCS
    "*.cc"
    "*.cpp"
)

# Source files from src/ required for testing
file(GLOB_RECURSE CORE_SRCS
    "${PROJECT_SOURCE_DIR}/src/model/*.cc"
    "${PROJECT_SOURCE_DIR}/src/model/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/service/*.cc"
    "${PROJECT_SOURCE_DIR}/src/service/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/repository/*.cc"
    "${PROJECT_SOURCE_DIR}/src/repository/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/util/*.cc"
    "${PROJECT_SOURCE_DIR}/src/util/*.cpp"
    "${PROJECT_SOURCE_DIR}/src/ai/*.cc"
    "${PROJECT_SOURCE_DIR}/src/ai/*.cpp"
)

add_executable(sudharshanmart_tests
    ${TEST_SRCS}
    ${CORE_SRCS}
)

target_include_directories(sudharshanmart_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(sudharshanmart_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    GTest::gmock
    Drogon::Drogon
    nlohmann_json::nlohmann_json
    unofficial-sodium::sodium
    spdlog::spdlog
    unofficial::sqlite3::sqlite3
    CURL::libcurl
)

gtest_discover_tests(sudharshanmart_tests)
