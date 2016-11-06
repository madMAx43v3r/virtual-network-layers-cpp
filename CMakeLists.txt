
set(CMAKE_CXX_FLAGS "-std=c++11 -g -O3 -pthread -Wfatal-errors -fPIC")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/installs/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/installs/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/installs/bin)

include_directories(
  include/
  generated/include/
)

ADD_LIBRARY(vnl STATIC
	src/BinaryValue.cpp
	src/CRC64.cpp
	src/Engine.cpp
	src/FiberEngine.cpp
	src/Layer.cpp
	src/Message.cpp
	src/Object.cpp
	src/Router.cpp
)

add_subdirectory(generated)
