cmake_minimum_required(VERSION 2.4)

set(CMAKE_CXX_FLAGS "-std=c++11 -g -O3 -pthread -Wfatal-errors")

include_directories(
	generated/include/
	../include/
	./
)

link_directories(../ generated/)

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

add_subdirectory(generated)

