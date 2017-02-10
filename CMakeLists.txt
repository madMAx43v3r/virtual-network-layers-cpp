cmake_minimum_required(VERSION 2.4)
project(virtual-network-layers-cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -pthread")

include_directories(include/ generated/include/)

ADD_LIBRARY(vnl STATIC
	src/BinaryValue.cpp
	src/CRC64.cpp
	src/Engine.cpp
	src/Layer.cpp
	src/Message.cpp
	src/Object.cpp
	src/Router.cpp
	src/Type.cpp
	src/Pipe.cpp
)

target_include_directories(vnl PUBLIC include/)

