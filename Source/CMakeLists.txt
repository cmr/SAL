cmake_minimum_required(VERSION 2.6)
project(SAL C)

set(sal_sources Cryptography.c Socket.c Thread.c Time.c)
set(sal_headers Cryptography.h Socket.h Thread.h Time.h Common.h)
add_library(SoftwareAbstractionLayer SHARED ${sal_sources})


if(MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/arch:AVX /Gr /O2 /MP /Za /Fx /FAs /Fa")
  set(CMAKE_C_FLAGS_DEBUG "/arch:AVX /Od /Za /MP /FAs /Fa /Zi")
endif()

if(NOT WIN32)
  install(FILES ${sal_headers} DESTINATION include/SAL)
endif()
