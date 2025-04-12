find_package(Catch2 REQUIRED)

add_executable(tests tests.cpp)
target_link_libraries(tests PRIVATE aa Catch2::Catch2WithMain)
add_test(NAME tests COMMAND tests)
