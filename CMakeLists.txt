
include_directories(include/)

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

