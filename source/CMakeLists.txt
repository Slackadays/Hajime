cmake_minimum_required(VERSION 3.15)

project(Hajime LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

find_package(ZLIB)
find_package(OpenSSL)
if(ZLIB_FOUND AND OpenSSL_FOUND)
  add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT CPPHTTPLIB_ZLIB_SUPPORT)
endif()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

#include(CheckCXXCompilerFlag)
#CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
#if(COMPILER_SUPPORTS_MARCH_NATIVE)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
#endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")

add_executable(hajime
hajime.cpp
server.cpp
server_features.cpp
server_performance.cpp
server_performance_counters.cpp
getvarsfromfile.cpp output.cpp
installer.cpp
deduce.cpp
languages.cpp
wizard.cpp
constants.cpp
httplib/httplib.cpp
)

if(lto_supported)
  set_property(TARGET hajime PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
  if(CMAKE_COMPILER_IS_GNUCXX)
    list(APPEND CMAKE_CXX_COMPILE_OPTIONS_IPO "-flto=auto") # set the thread amount to what is available on the CPU
  endif()
else()
    message(STATUS "IPO/LTO not supported: ${lto_error}")
endif()

set_property(TARGET hajime PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s") # strip the binary for reduced size
  endif()
endif()

target_link_libraries(hajime Threads::Threads)

if(ZLIB_FOUND AND OpenSSL_FOUND)
  target_link_libraries(hajime ZLIB::ZLIB)
  target_link_libraries(hajime OpenSSL::SSL)
  target_link_libraries(hajime OpenSSL::Crypto)
endif()

set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} ${ADD_FLAGS}")
