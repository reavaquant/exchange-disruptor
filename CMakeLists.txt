cmake_minimum_required(VERSION 3.20)

project(
  exchange_disruptor
  VERSION 0.1.0
  DESCRIPTION "Exchange Disruptor"
  LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(EXCHANGE_DISRUPTOR_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Warnings)

add_library(exchange_disruptor_options INTERFACE)
target_compile_features(exchange_disruptor_options INTERFACE cxx_std_20)
target_include_directories(exchange_disruptor_options INTERFACE
  "${CMAKE_CURRENT_SOURCE_DIR}/include/exchange-disruptor"
)

set(EXCHANGE_DISRUPTOR_SRC
  src/matching_engine/order/order.cpp
  src/matching_engine/matching_engine.cpp
  src/matching_engine/orderbook/orderbook.cpp
  src/matching_engine/orderbook/book.cpp
  src/matching_engine/command/cancel_command.cpp
  src/matching_engine/command/command.cpp
  src/matching_engine/command/limit_command.cpp
  src/matching_engine/command/market_command.cpp
  src/matching_engine/event/reject_event.cpp
  src/matching_engine/event/fill_event.cpp
  src/matching_engine/event/event.cpp
  src/matching_engine/event/ack_event.cpp
  src/matching_engine/matching_engine.cpp
)

add_library(exchange_disruptor_lib ${EXCHANGE_DISRUPTOR_SRC})
target_link_libraries(exchange_disruptor_lib PUBLIC exchange_disruptor_options)

add_executable(exchange_disruptor src/main.cpp)
target_link_libraries(exchange_disruptor PRIVATE exchange_disruptor_lib)
target_set_warnings(exchange_disruptor)

include(CTest)
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
