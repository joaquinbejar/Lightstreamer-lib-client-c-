enable_testing()
include(CTest)

add_executable(test1
        unit/test1.cpp)
target_link_libraries(test1 PRIVATE Catch2::Catch2WithMain)
target_include_directories(test1
        PRIVATE
        ${SIMPLE_COLOR_INCLUDE}
)

target_link_libraries(test1 PRIVATE
        Logging
        simple_color
)

#target_compile_options(test1 PRIVATE -fmodules -std=gnu++20)
#target_sources(test1
#        PRIVATE
#        FILE_SET cxx_modules TYPE CXX_MODULES FILES
#        ${LIGHTSTREAMER_ROOT_DIR}/src/modules/Logger.hpp
#        ${LIGHTSTREAMER_ROOT_DIR}/src/modules/ConsoleLogLevel.hpp
#)
add_test(NAME Test1 COMMAND test1)