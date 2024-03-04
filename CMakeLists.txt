cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(Lightstreamer-lib-client-cpp VERSION 0.1.0 LANGUAGES C CXX)

add_subdirectory(extern/simple_color)


add_library(Logging SHARED
        src/modules/ClientListener.cppm
        src/modules/ClientMessageListener.cppm
        src/modules/ItemUpdate.cppm
        src/modules/SubscriptionListener.cppm
        src/modules/ConnectionDetails.cppm
        src/modules/ConnectionOptions.cppm
        src/modules/ConsoleLoggerProvider.cppm
        src/modules/LightstreamerClient.cppm
        src/modules/Proxy.cppm
        src/modules/Subscription.cppm

        include/ConsoleLogLevel.hpp
        include/Logger.hpp
)
target_sources(Logging
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        src/modules/ClientListener.cppm
        src/modules/ClientMessageListener.cppm
        src/modules/ItemUpdate.cppm
        src/modules/SubscriptionListener.cppm
        src/modules/ConnectionDetails.cppm
        src/modules/ConnectionOptions.cppm
        src/modules/ConsoleLoggerProvider.cppm
        src/modules/LightstreamerClient.cppm
        src/modules/Proxy.cppm
        src/modules/Subscription.cppm
)
target_compile_features(Logging PUBLIC cxx_std_20)
target_compile_options(Logging PRIVATE -fmodules -std=gnu++20)

target_link_libraries(Logging PRIVATE simple_color)

target_include_directories(Logging
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        extern/simple_color/include
        PRIVATE
        src
        )
set(LIGHTSTREAMER_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

option(LIGHTSTREAMER_BUILD_TESTS "Build the Lightstreamer tests" ON)

if(LIGHTSTREAMER_BUILD_TESTS)
        include(FetchContent)
        FetchContent_Declare(Catch2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG v3.4.0
        )
        FETCHCONTENT_GETPROPERTIES(Catch2)
        FETCHCONTENT_MAKEAVAILABLE(Catch2)
        add_subdirectory(tests)
endif()
