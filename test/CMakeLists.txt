cmake_minimum_required(VERSION 2.4)
cmake_policy(SET CMP0015 NEW)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O3 -pthread -Wfatal-errors")

include_directories(
	generated/include/
	../include/
	./
)

link_directories(../ generated/)

ADD_LIBRARY(vnl STATIC
	../src/CRC64.cpp
	../src/Engine.cpp
	../src/FiberEngine.cpp
	../src/Layer.cpp
	../src/Message.cpp
	../src/Object.cpp
	../src/Random.cpp
	../src/Router.cpp
)

file(MAKE_DIRECTORY generated)

add_custom_target(codegen ALL COMMAND vnicppcodegen ../interface interface/)

add_subdirectory(generated)

add_executable(test_module test_module.cpp)
target_link_libraries(test_module vnl interface)

add_executable(test_type_stream test_type_stream.cpp)
target_link_libraries(test_type_stream vnl interface)

add_executable(test_packet test_packet.cpp)
target_link_libraries(test_packet vnl interface)

add_executable(test_tcp_server test_tcp_server.cpp)
target_link_libraries(test_tcp_server vnl interface)

add_executable(test_tcp_client test_tcp_client.cpp)
target_link_libraries(test_tcp_client vnl interface)



