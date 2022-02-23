cmake_minimum_required(VERSION 3.15)

project(Hajime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_executable(hajime hajime.cpp server.cpp server_features.cpp server_performance.cpp getvarsfromfile.cpp output.cpp installer.cpp deduce.cpp languages.cpp wizard.cpp constants.cpp)

set_property(TARGET hajime PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")

target_link_libraries(hajime Threads::Threads)
