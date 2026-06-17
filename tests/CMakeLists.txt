enable_testing()
add_executable(tests tests.cpp)
target_link_libraries(tests PUBLIC grafx doctest)
add_test(NAME tests COMMAND tests --force-colors -d)