cmake_minimum_required(VERSION 3.10)

project(jarunner)

set(CMAKE_CXX_STANDARD 11)

# libjarunner.a libjarunner.h
if(WIN32)
    set(LIB_NAME libjarunner.a)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows -e mainCRTStartup")
else()
    set(LIB_NAME libjarunner.so)
endif()

link_directories(${CMAKE_SOURCE_DIR}/lib)
include_directories(${CMAKE_SOURCE_DIR}/lib)

add_executable(jarunner main.cpp)
target_link_libraries(jarunner ${LIB_NAME})

