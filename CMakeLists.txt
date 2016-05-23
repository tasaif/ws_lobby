cmake_minimum_required (VERSION 2.6)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
set(CMAKE_FIND_LIBRARY_PREFIXES "lib")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
find_library(JSONCPP_LIBRARY NAMES jsoncpp HINTS ./vendor/jsoncpp/src/lib_json)

project (wslobby)
include_directories(wslobby
  ./vendor/websocketpp/opt/websocketpp/include
  ./vendor/jsoncpp/include
)

add_executable(wslobby
  src/main.cpp
  src/lobbyserver.cpp
  src/action.cpp
  src/connection_data.cpp
  src/config.cpp
)
target_link_libraries(wslobby
  boost_system
  pthread
  ${JSONCPP_LIBRARY}
  curlpp
  curl
)
