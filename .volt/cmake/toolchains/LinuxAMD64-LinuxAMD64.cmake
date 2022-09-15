set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR amd64)

set(triple amd64-pc-linux-gnu)

set(CMAKE_C_COMPILER clang)
set(CMAKE_C_COMPILER_TARGET ${triple})
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_COMPILER_TARGET ${triple})

# Force relative RPATH
set(CMAKE_SKIP_BUILD_RPATH TRUE)
set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS}    -Wl,-rpath,'$ORIGIN'")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,-rpath,'$ORIGIN'")
# Static libraries are not used

set(CMAKE_NO_SONAME TRUE)
