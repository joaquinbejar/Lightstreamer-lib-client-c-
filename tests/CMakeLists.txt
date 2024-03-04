enable_testing()
include(CTest)

add_executable(test_logger unit/test_logger.cpp)
target_link_libraries(test_logger PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_logger PRIVATE ${SIMPLE_COLOR_INCLUDE})
target_link_libraries(test_logger PRIVATE Logging simple_color)

add_test(NAME Logger COMMAND test_logger)