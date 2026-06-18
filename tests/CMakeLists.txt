enable_testing()
add_executable(grafx-tests grafx-tests.cpp)
target_link_libraries(grafx-tests PUBLIC grafx doctest)
add_test(NAME grafx-tests COMMAND grafx-tests --force-colors -d)