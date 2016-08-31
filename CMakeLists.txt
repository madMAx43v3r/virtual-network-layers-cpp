cmake_minimum_required(VERSION 2.4)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O3 -pthread -Wfatal-errors")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/env/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/env/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/env/bin)

include_directories(
  include/
  generated/include/
)

ADD_LIBRARY(vnl STATIC
	src/CRC64.cpp
	src/Engine.cpp
	src/FiberEngine.cpp
	src/Layer.cpp
	src/Message.cpp
	src/Object.cpp
	src/Random.cpp
	src/Router.cpp
)

add_subdirectory(generated)