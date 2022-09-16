cmake_minimum_required(VERSION 3.15)

project(Hajime LANGUAGES CXX)

project(Hajime VERSION 0.2.0)

find_package(ZLIB)

if(NOT ZLIB_FOUND)
  add_subdirectory(zlib)
  include_directories(hajime zlib)
  set_target_properties(example PROPERTIES EXCLUDE_FROM_ALL 1)
  set_target_properties(minigzip PROPERTIES EXCLUDE_FROM_ALL 1)
  if(NOT WIN32)
    set_target_properties(example64 PROPERTIES EXCLUDE_FROM_ALL 1)
    set_target_properties(minigzip64 PROPERTIES EXCLUDE_FROM_ALL 1)
   endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_BUILD_TYPE Release)

set(CMAKE_THREAD_PREFER_PTHREAD True)
set(THREADS_PREFER_PTHREAD_FLAG True)
find_package(Threads REQUIRED)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT lto_error)

#set(OPENSSL_USE_STATIC_LIBS TRUE)

if(NOT NO_SSL)
  find_package(OpenSSL)
endif()

if(OpenSSL_FOUND)
  add_compile_definitions(CPPHTTPLIB_OPENSSL_SUPPORT)
endif()

find_package(OpenMP)

add_compile_definitions(CPPHTTPLIB_ZLIB_SUPPORT)

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-march=native" COMPILER_SUPPORTS_MARCH_NATIVE)
if(COMPILER_SUPPORTS_MARCH_NATIVE AND (NOT DEFINED ENV{CI})) #do not use a native uarch if we are in a CI environment
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ADD_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ADD_FLAGS}")

add_executable(hajime
main.cpp
server.cpp
server_features.cpp
getvarsfromfile.cpp output.cpp
installer.cpp
deduce.cpp
languages.cpp
wizard.cpp
constants.cpp
performance_counters/other_metrics.cpp
)

if(UNIX AND NOT APPLE)
  if(${CMAKE_SYSTEM_NAME} STREQUAL "FreeBSD")
    target_sources(hajime PRIVATE performance_counters/freebsd.cpp)
  else()
    target_sources(hajime PRIVATE performance_counters/linux.cpp)
  endif()
elseif(APPLE)
  target_sources(hajime PRIVATE performance_counters/macos.cpp)
elseif(WIN32)
  target_sources(hajime PRIVATE performance_counters/windows.cpp)
else()
  target_sources(hajime PRIVATE performance_counters/other.cpp)
endif()

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
else()
  add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/W4>")
  add_compile_options("$<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall>")
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/zlib/zconf.h.included)
  file(RENAME zlib/zconf.h.included zlib/zconf.h)
endif()

target_link_libraries(hajime Threads::Threads)

if(OpenMP_CXX_FOUND)
    target_link_libraries(hajime OpenMP::OpenMP_CXX)
endif()

if(OpenSSL_FOUND)
  target_link_libraries(hajime OpenSSL::SSL)
  target_link_libraries(hajime OpenSSL::Crypto)
endif()

if(UNIX AND NOT APPLE)
  if(ZLIB_FOUND)
    target_link_libraries(hajime ZLIB::ZLIB)
  else()
    set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} ${CMAKE_BINARY_DIR}/zlib/libz.a ${ADD_FLAGS}") #statically link zlib; this is the simplest way to do it
  endif()
else()
  if(ZLIB_FOUND)
    target_link_libraries(hajime ZLIB::ZLIB)
  else()
    target_link_libraries(hajime zlib)
  endif()
  set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} ${ADD_FLAGS}")
endif()


