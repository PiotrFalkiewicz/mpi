cmake_minimum_required(VERSION 3.7)
project(programs)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        mpi_hl.c)

add_executable(programs ${SOURCE_FILES})