add_executable(redtops_tests test_main.cpp)
target_link_libraries(redtops_tests PRIVATE Catch2::Catch2WithMain)
add_test(NAME redtops_tests COMMAND redtops_tests)