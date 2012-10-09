cmake_minimum_required(VERSION 2.6)
project(SAL C)

set(sal_sources Cryptography.c Socket.c Thread.c Time.c)
file(GLOB_RECURSE sal_headers include/*.h)

include_directories(include)

add_library(SAL SHARED ${sal_sources})

if(MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/arch:AVX /Gr /O2 /MP /Za")
  set(CMAKE_C_FLAGS_DEBUG "/arch:AVX /Od /Za /MP /Zi")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD true)
find_package(Threads REQUIRED)

target_link_libraries(SAL ${CMAKE_THREAD_LIBS_INIT})

if(NOT WIN32)
  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIRS})
  target_link_libraries(SAL ${OPENSSL_LIBRARIES})
  install(FILES ${sal_headers} DESTINATION include/SAL)
endif()
