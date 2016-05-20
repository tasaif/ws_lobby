cmake_minimum_required (VERSION 2.6)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")

project (wslobby)
include_directories(wslobby
  ./vendor/websocketpp/opt/websocketpp/include
)

add_executable(wslobby
  src/main.cpp
  src/lobbyserver.cpp
)
target_link_libraries(wslobby
  boost_system
  pthread
)
