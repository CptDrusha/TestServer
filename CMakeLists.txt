cmake_minimum_required(VERSION 3.16)
project(EDGE_server)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")

set(CMAKE_CXX_STANDARD 14)

include_directories(include/enet-1.3.17/include)
include_directories(/usr/local)
include_directories(Server)
include_directories(Networking)

add_executable(EDGE_server
        Serial/Timer.h
        Serial/Serialization.h
        Serial/Serialization.cpp
        Networking/ENet_Common.h
        Models/MessageTypes.h
        Json/json11.cpp
        Json/json11.hpp
        Db/DbConnector.cpp
        Db/DbConnector.h
        Server/ENetServer.cpp
        Server/ENetServer.h
        main.cpp)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(EDGE_server PRIVATE Threads::Threads mysqlcppconn
        /usr/local/include/enet-1.3.17/win32.o
        /usr/local/include/enet-1.3.17/.libs/libenet.so)