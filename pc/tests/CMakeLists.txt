include(FetchContent)

# Catch2'yi internetten indir (internet olmadan build edilemez, bir kerelik)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# Test executable
add_executable(preciscan_tests
    test_seq_tracker.cpp
)

target_link_libraries(preciscan_tests
    PRIVATE
    preciscan_core
    Catch2::Catch2WithMain
)

target_compile_options(preciscan_tests PRIVATE /W4)

# CTest entegrasyonu
include(CTest)
include(Catch)
catch_discover_tests(preciscan_tests)
