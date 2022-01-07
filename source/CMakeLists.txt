cmake_minimum_required(VERSION 3.12)

project(Hajime)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(hajime hajime.cpp server.cpp getvarsfromfile.cpp output.cpp installer.cpp deduce.cpp languages.cpp wizard.cpp)

target_link_libraries(hajime Threads::Threads)
