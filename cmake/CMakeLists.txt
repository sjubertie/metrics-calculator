cmake_minimum_required(VERSION 2.8)

project(metrics-calculator)

find_package(LLVM 3.6 REQUIRED CONFIG)


include_directories(${LLVM_INCLUDE_DIRS} ../include)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti " )

add_definitions(-D__STDC_LIMIT_MACROS -D__STDC_CONSTANT_MACROS)


link_directories(${LLVM_LIBRARY_DIRS})

add_executable(metrics-calculator ../src/metrics-calculator.cpp)

TARGET_LINK_LIBRARIES(metrics-calculator -lLLVM-3.6 -lclangTooling -lclangFrontend -lclangParse -lclangSema -lclangAnalysis -lclangAST -lclangBasic -lclangDriver -lclangSerialization -lclangEdit -lclangLex)
