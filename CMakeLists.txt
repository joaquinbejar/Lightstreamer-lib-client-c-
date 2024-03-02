cmake_minimum_required(VERSION 3.28)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(Lightstreamer-lib-client-cpp VERSION 0.1.0 LANGUAGES C CXX)



add_library(Logging SHARED
        src/modules/Logging/Logger.cppm
        src/modules/Logging/LoggerProvider.cppm
        src/modules/Logging/Log.cppm
)
target_sources(Logging
        PUBLIC
        FILE_SET cxx_modules TYPE CXX_MODULES FILES
        src/modules/Logging/Logger.cppm
        src/modules/Logging/LoggerProvider.cppm
)
target_compile_features(Logging PUBLIC cxx_std_20)
target_compile_options(Logging PRIVATE -fmodules -std=gnu++20)




#target_compile_features(Logging PUBLIC cxx_std_20)
#target_compile_options(Logging PRIVATE -fmodules -std=gnu++20)
#target_include_directories(Logging PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include/Logging)
