cmake_minimum_required(VERSION 3.10)
project(GoQuant-Assignment)
cmake_policy(SET CMP0167 NEW)
set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Boost REQUIRED COMPONENTS system filesystem)

include_directories(${CMAKE_SOURCE_DIR}/include ${CURL_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

set(COMMON_LIBS Boost::system Boost::filesystem OpenSSL::SSL OpenSSL::Crypto CURL::libcurl)

add_executable(trader 
    trader.cpp 
    src/DeribitAPI.cpp 
    src/HttpClient.cpp 
    src/WebSocketClient.cpp
    src/WebSocketServer.cpp
)
target_link_libraries(trader PRIVATE ${COMMON_LIBS})

add_executable(realtime_client 
    realtime_client.cpp 
    src/WebSocketClient.cpp
)
target_link_libraries(realtime_client PRIVATE ${COMMON_LIBS})

add_executable(realtime_server
    realtime_server.cpp
    src/WebSocketServer.cpp
)
target_link_libraries(realtime_server PRIVATE ${COMMON_LIBS})
