cmake_minimum_required(VERSION 2.6)
project(SAL C)


set(sal_sources Cryptography.c Socket.c Thread.c Time.c)
file(GLOB_RECURSE sal_headers include/*.h)


include_directories(include)


add_library(SoftwareAbstractionLayer SHARED ${sal_sources})


if(MSVC)
  set(CMAKE_C_FLAGS_RELEASE "/arch:AVX /Gr /O2 /MP /Za /Fx /FAs /Fa")
  set(CMAKE_C_FLAGS_DEBUG "/arch:AVX /Od /Za /MP /FAs /Fa /Zi")
endif()


if(NOT WIN32)
  install(FILES ${sal_headers} DESTINATION include/SAL)
endif()
