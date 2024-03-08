enable_testing()
include(CTest)

add_executable(test_logger unit/test_logger.cpp)
target_link_libraries(test_logger PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_logger PRIVATE ${SIMPLE_COLOR_INCLUDE} ${LIGHTSTREAMER_INCLUDE_DIR})
target_link_libraries(test_logger PRIVATE Lightstreamer simple_color)
add_test(NAME Logger COMMAND test_logger)



add_executable(test_proxy unit/test_proxy.cpp)
target_link_libraries(test_proxy PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_proxy PRIVATE ${SIMPLE_COLOR_INCLUDE} ${LIGHTSTREAMER_INCLUDE_DIR})
target_link_libraries(test_proxy PRIVATE Lightstreamer simple_color)
add_test(NAME Proxy COMMAND test_proxy)


add_executable(test_connectiondetails unit/test_connectiondetails.cpp)
target_link_libraries(test_connectiondetails PRIVATE Catch2::Catch2WithMain)
target_include_directories(test_connectiondetails PRIVATE ${SIMPLE_COLOR_INCLUDE} ${LIGHTSTREAMER_INCLUDE_DIR})
target_link_libraries(test_connectiondetails PRIVATE Lightstreamer simple_color)
add_test(NAME ConnectionDetails COMMAND test_connectiondetails)